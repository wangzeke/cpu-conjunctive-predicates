/*******************************************************************************
 * Copyright (c) 2017
 * National University of Singapore
 * 
 * Author: Wang, Zeke (wangzeke638@gmail.com)
 * for Q5....
 * See file LICENSE.md for details.
 *******************************************************************************/

#include	<cstdio>
#include    <cstdlib>

#include <time.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <immintrin.h>

#include "cpu_mapping.h"

#include   "byteslice_column_block.h"
#include   "bitvector_block.h"
#include   "types.h"

//#include "rand.h"
//#include "rdtsc.h"
#include "perf_counters.h"

//BIT_WIDTH_BYTESLICE:::: user need to specifize the 
//bit width of the design...

#define BIT_WIDTH_BYTESLICE 17
#define INTEL_PCM_ENABLE
#define TWO_COLUMN_ENABLE //test on two column....


namespace byteslice {
	

struct Monitor_Event inst_Monitor_Event = {
	{
		{0,0},
		{0,0},
		{0,0},
		{0,0},
	},
	0,
	{
		"core_0",
		"core_1",
		"core_2",
		"core_3",
	},
	{
		{0,0},
		{0,0},
		{0,0},
		{0,0},		
	},
	2,
	{
		"MIC_0",
		"MIC_1",
		"MIC_2",
		"MIC_3",
	},
    0  //15	//
};
	
#ifdef __INTEL_COMPILER
typedef long si64;
#else
typedef long long si64;
#endif


typedef struct rand_state_32 {
	uint32_t num[625];
	size_t index;
} rand32_t;

rand32_t *rand32_init(uint32_t seed)
{
	rand32_t *state = (rand32_t *) malloc(sizeof(rand32_t));
	uint32_t *n = state->num;
	size_t i;
	n[0] = seed;
	for (i = 0 ; i != 623 ; ++i)
		n[i + 1] = 0x6c078965 * (n[i] ^ (n[i] >> 30));
	state->index = 624;
	return state;
}

uint32_t rand32_next(rand32_t *state)
{
	uint32_t y, *n = state->num;
	if (state->index == 624) {
		size_t i = 0;
		do {
			y = n[i] & 0x80000000;
			y += n[i + 1] & 0x7fffffff;
			n[i] = n[i + 397] ^ (y >> 1);
			n[i] ^= 0x9908b0df & -(y & 1);
		} while (++i != 227);
		n[624] = n[0];
		do {
			y = n[i] & 0x80000000;
			y += n[i + 1] & 0x7fffffff;
			n[i] = n[i - 227] ^ (y >> 1);
			n[i] ^= 0x9908b0df & -(y & 1);
		} while (++i != 624);
		state->index = 0;
	}
	y = n[state->index++];
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680;
	y ^= (y << 15) & 0xefc60000;
	y ^= (y >> 18);
	return y;
}
	
uint64_t thread_time(void)
{
	struct timespec t;
	assert(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t) == 0);
	return t.tv_sec * 1000 * 1000 * 1000 + t.tv_nsec;
}

void bind_thread(int thread, int threads)
{
	assert(thread >= 0 && thread < threads);
	size_t size = CPU_ALLOC_SIZE(threads);
	cpu_set_t *cpu_set = CPU_ALLOC(threads);
	assert(cpu_set != NULL);
	CPU_ZERO_S(size, cpu_set);
	CPU_SET_S(thread, size, cpu_set);
	assert(pthread_setaffinity_np(pthread_self(), size, cpu_set) == 0);
	CPU_FREE(cpu_set);
}

void *alloc(uint64_t bytes)
{
	void *ptr = NULL;
	assert(posix_memalign(&ptr, 128, bytes) == 0);
	memset(ptr, 0xDB, bytes);
	return ptr;
}

uint32_t min(uint32_t x, uint32_t y) { return x < y ? x : y; }
uint32_t max(uint32_t x, uint32_t y) { return x > y ? x : y; }

inline void SetTuple(size_t pos, WordUnit value, ByteUnit** data_, size_t kNumBytesPerCode)
{
    switch(kNumBytesPerCode){
        case 4:
            data_[0][pos] = (value >> 24)&255;
            data_[1][pos] = (value >> 16)&255;
            data_[2][pos] = (value >> 8) &255;
            data_[3][pos] = (value)&255;
            break;
        case 3:
            data_[0][pos] = (value >> 16)&255;
            data_[1][pos] = (value >> 8) &255;
            data_[2][pos] = (value)&255;
            break;
        case 2:
            data_[0][pos] = (value >> 8) &255;
            data_[1][pos] = (value)&255;
            break;
        case 1:
            data_[0][pos] = (value)&255;
            break;
    }
}

void compress(const uint32_t *decompressed, ByteUnit** data_, size_t tuples, int8_t bits)
{
	size_t kNumBytesPerCode = ( (bits+7) >> 3);//ceil (bits, 8);
	uint32_t pos;
    for (pos = 0; pos < tuples; pos++ )
	{
      uint32_t value = decompressed[pos];		
      SetTuple( pos, value, data_, kNumBytesPerCode);
		
	}
}

inline WordUnit GetTuple(size_t pos, ByteUnit** data_, size_t kNumBytesPerCode)
{
	WordUnit value;
    switch(kNumBytesPerCode){
        case 4:
		    value = (data_[0][pos]<<24) | (data_[1][pos]<<16) | (data_[2][pos]<<8) | (data_[3][pos]<<0);
            break;
        case 3:
		    value = (data_[0][pos]<<16) | (data_[1][pos]<<8) | (data_[2][pos]<<0);
            break;
        case 2:
		    value = (data_[0][pos]<<8) | (data_[1][pos]<<0);
            break;
        case 1:
		    value = (data_[0][pos]<<0);
            break;
    }
	return value;
}

void decompress(ByteUnit** data_, uint32_t *decompressed, size_t tuples, int8_t bits)
{
	size_t kNumBytesPerCode = ( (bits+7) >> 3);//ceil (bits, 8);
	uint32_t pos;
    for (pos = 0; pos < tuples; pos++ )
	{
      uint32_t value; 	
      value = GetTuple( pos, data_, kNumBytesPerCode);
	  decompressed[pos]  = value;		
	}
}



typedef struct {
	pthread_t id;
	int seed;
	int thread;
	int threads;
	int min_bits;
	int max_bits;

    float T1_selevitity;	
    float T2_selevitity;	
    float T3_selevitity;	
    int execution_model;
	
	size_t tuples;
	size_t *set_bits;
	uint64_t *times[3];
	pthread_barrier_t *barrier;
} info_t;


void *run(void *arg)
{
	info_t *d = (info_t*) arg;
	assert(pthread_equal(pthread_self(), d->id));
	bind_thread(d->thread, d->threads);
	int bits = BIT_WIDTH_BYTESLICE, t; 

//	  pthread_barrier_wait(barrier++); 
	  FILE *fp_r_name, *fp_o_orderdate;//
	  char str[128];
	  uint32_t tmp;
      size_t i;

	  

	pthread_barrier_t *barrier = d->barrier;
	size_t tuples = d->tuples;
	//assert(tuples % 128 == 0);
	size_t bitmasks = (tuples+63) / 64;
	int execution_model = d->execution_model;



	uint32_t *original         = (uint32_t *) alloc(tuples * sizeof(uint32_t));
	uint32_t *original_1       = (uint32_t *) alloc(tuples * sizeof(uint32_t));
	uint64_t *bitmap           = (uint64_t *) alloc(bitmasks * sizeof(uint64_t));
	
    size_t kMemSizePerByteSlice = 1;	
    //uint32_t value_1 = rand32_next(gen) >> (32 - bits);		
    uint32_t num_    = tuples; //1024;//64; //128;//

	//for (bits = min_bits ; bits <= max_bits ; ++bits) 
	{
        uint32_t literal_1   = 3; //Asia
	    uint32_t literal_2   = 1488; //1994-1-1  
        uint32_t literal_3   = 1860; //1995-1-1


		BitVectorBlock* bvblock = new BitVectorBlock(num_);
	
    //ByteSliceColumnBlock<16>* block2 = new ByteSliceColumnBlock<16>(num_);
      ByteSliceColumnBlock<3>* block2   = new ByteSliceColumnBlock<3>(num_);
      ByteSliceColumnBlock<12>* block2_1 = new ByteSliceColumnBlock<12>(num_);

  //1....................: output_r1_name
      if((fp_r_name=fopen("../../../lineitemWT/output_r1_name.txt","r"))==NULL) {
        printf("cannot open output_r1_name.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_r_name, "%d\n", &tmp) > 0) { 
         original[i]       = tmp; //tmp_cons; //
         block2->SetTuple(i, tmp);		  
	     i++;
    }
    fclose(fp_r_name);

	
  //2....................: output_o_orderdate.txt
      if((fp_o_orderdate=fopen("../../../lineitemWT/output_o_orderdate.txt","r"))==NULL) {
        printf("cannot open output_o_orderdate.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_o_orderdate, "%d\n", &tmp) > 0) { 
         original_1[i]       = tmp; //tmp_cons; //
         block2_1->SetTuple(i, tmp);		  
	     i++;
    }
    fclose(fp_o_orderdate);

	

    pthread_barrier_wait(barrier++);

		
		
#ifdef INTEL_PCM_ENABLE		
    if (d->thread == 0)
	{   
     // printf("bit_width = %d\n", BIT_WIDTH_BYTESLICE);
        PCM_initPerformanceMonitor(&inst_Monitor_Event, NULL);
        PCM_start();
	}
#endif	
		pthread_barrier_wait(barrier++);
		uint64_t t3 = thread_time();
		

		 if (execution_model == 0)
		 {
           block2->Scan(Comparator::kEqual,          literal_1, bvblock, Bitwise::kSet);
		   block2_1->Scan(Comparator::kGreaterEqual, literal_2, bvblock, Bitwise::kAnd);
		   block2_1->Scan(Comparator::kLess,         literal_3, bvblock, Bitwise::kAnd);
		 }
         else if (execution_model == 1)
		 {
           block2->Scan(Comparator::kEqual,          literal_1, bvblock, Bitwise::kSet);
		   block2_1->Scan(Comparator::kLess,         literal_3, bvblock, Bitwise::kAnd);
		   block2_1->Scan(Comparator::kGreaterEqual, literal_2, bvblock, Bitwise::kAnd);
		 }
         else if (execution_model == 2)
		 {
		   block2_1->Scan(Comparator::kLess,         literal_3, bvblock, Bitwise::kSet);
           block2->Scan(Comparator::kEqual,          literal_1, bvblock, Bitwise::kAnd);
		   block2_1->Scan(Comparator::kGreaterEqual, literal_2, bvblock, Bitwise::kAnd);
		 }
         else if (execution_model == 3)
		 {
		   block2_1->Scan(Comparator::kLess,         literal_3, bvblock, Bitwise::kSet);
		   block2_1->Scan(Comparator::kGreaterEqual, literal_2, bvblock, Bitwise::kAnd);
           block2->Scan(Comparator::kEqual,          literal_1, bvblock, Bitwise::kAnd);
		 }		 
         else if (execution_model == 4)
		 {
		   block2_1->Scan(Comparator::kGreaterEqual, literal_2, bvblock, Bitwise::kSet);
		   block2_1->Scan(Comparator::kLess,         literal_3, bvblock, Bitwise::kAnd);
           block2->Scan(Comparator::kEqual,          literal_1, bvblock, Bitwise::kAnd);
		 }
         else if (execution_model == 5)
		 {
		   block2_1->Scan(Comparator::kGreaterEqual, literal_2, bvblock, Bitwise::kSet);
           block2->Scan(Comparator::kEqual,          literal_1, bvblock, Bitwise::kAnd);
		   block2_1->Scan(Comparator::kLess,         literal_3, bvblock, Bitwise::kAnd);
		 }	
		 
		pthread_barrier_wait(barrier++);
		t3 = thread_time() - t3;
		
#ifdef INTEL_PCM_ENABLE			
	if (d->thread == 0)
	{
        PCM_stop();
        printf("=====print the profiling result==========\n");//PCM_log("======= Partitioning phase profiling results ======\n");
        PCM_printResults();		
		PCM_cleanup();
	}
#endif
		d->times[2][d->thread] = t3;

		pthread_barrier_wait(barrier++);
	
		//WordUnit flag_0 = bvblock->GetWordUnit(0);
		//printf("flag_0 = 0x%llx\n", flag_0);
	    //verification of the comparision results.
		//for (size_t ii = 0; ii < tuples; ii++)
        //if (d->thread == 0)

    if (d->thread == 0)
	{   
		 for (size_t ii = 0; ii < num_; ii++) //
		{
			//size_t ii = 11;
			bool real  = ((original[ii] == literal_1)&&(original_1[ii] >= literal_2)&&(original_1[ii] < literal_3)); 
	
			bool eval  = bvblock->GetBit(ii);
            if (real !=  eval )
			{
#ifdef	TWO_COLUMN_ENABLE	
		      printf("%d:  eval: %d, real: %d (%d, %d)\n", ii, eval, real, (original[ii] > literal_1), (original_1[ii] < literal_2));
#else 
		      printf("%d:  eval: %d, real: %d \n", ii, eval, real);
#endif	     
             break;
			 //   return NULL;
			}
  	    }
	}	
	


		pthread_barrier_wait(barrier++);
		if (d->thread == 0) {
			uint64_t t_sum = 0.0;
			for (size_t t = 0 ; t != d->threads ; ++t) {
				t_sum += d->times[2][t];
			}
			printf("THREE: %2d codes, time: %6.3f, codes_per_ns: %6.3f\n", BIT_WIDTH_BYTESLICE,((double)t_sum / (double)d->threads), 
			       (num_ * d->threads * 1.0) / ((double)t_sum / (double)d->threads) );
		}
		
	}

	free(original);
	free(original_1);
	free(bitmap);
	pthread_exit(NULL);
}

}

int hardware_threads(void)
{
	char name[64];
	struct stat st;
	int threads = -1;
	do {
		sprintf(name, "/sys/devices/system/cpu/cpu%d", ++threads);
	} while (stat(name, &st) == 0);
	return threads;
}

void main(int argc, char **argv)
{
	int t, threads       = argc > 1 ? atoi(argv[1]) : 1; //hardware_threads();
	int execution_model  = argc > 2 ? atof(argv[2]) : 0;         
    float T1_selevitity  = 0.5; 

    float T2_selevitity  = 0.5; 
    float T3_selevitity  = 0.5; 
	
	size_t tuples        =  60490115; //from TPC-H, with SF = 10
	
	int b, barriers      = 3* 7;
	pthread_barrier_t barrier[barriers];
	for (b = 0 ; b != barriers ; ++b)
		pthread_barrier_init(&barrier[b], NULL, threads);
	srand(time(NULL));
 
	printf("Threads: %d, tuples = %d, execution_model = %d\n", threads, tuples, execution_model);

	byteslice::info_t info[threads]; //
	uint64_t times[3][threads];
	size_t set_bits[threads];
	
		//for affinity setting.
    pthread_t tid[threads];
    pthread_attr_t attr;
    cpu_set_t set; //cpu_set_t *set = (cpu_set_t *) malloc (sizeof (cpu_set_t)); //
    pthread_attr_init(&attr);

	
	for (t = 0 ; t != threads ; ++t) {
	    //for affinity setting.		
        int cpu_idx = get_cpu_id(t);
        //DEBUGMSG(1, "Assigning thread-%d to CPU-%d\n", i, cpu_idx);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);


		
		info[t].tuples = tuples / threads;//(tuples / threads) & 0xffffff80;
		info[t].seed = rand();
		
        info[t].T1_selevitity  = T1_selevitity;
        info[t].T2_selevitity  = T2_selevitity;
        info[t].T3_selevitity  = T3_selevitity;
		info[t].execution_model = execution_model;
		
		info[t].thread   = t;
		info[t].threads  = threads;
		info[t].barrier  = barrier;
		info[t].set_bits = set_bits;
		info[t].times[0] = times[0];
		info[t].times[1] = times[1];
		info[t].times[2] = times[2];
		
		pthread_create(&info[t].id, &attr, byteslice::run, (void*) &info[t]); //
	}
	for (t = 0 ; t != threads ; ++t)
		pthread_join(info[t].id, NULL);
	for (b = 0 ; b != barriers ; ++b)
		pthread_barrier_destroy(&barrier[b]);
	return;//EXIT_SUCCESS
}



