/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file is the main entry of the TPCH Q4. 
 *Input: 
*        1, number of threads,  
*		 2, model (column group, column-only), 
*		 3, configuration
 * See file LICENSE.md for details.
 *******************************************************************************/
#include <cstdio>
#include <cstdlib>
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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <sched.h>              /* CPU_ZERO, CPU_SET */
//#include "affinity.h"           /* pthread_attr_setaffinity_np */
#include "cpu_mapping.h"

#include   "types_simd.h"

#include "common_tool.h"
#include "column_compare.h"



//#include 	"byteslice_column_block.h"
//#include 	"bitvector_block.h" 

//#include "rand.h"
//#include "perf_counters.h"

//BIT_WIDTH_BYTESLICE:::: user need to specifize the 
//bit width of the design...

	
#ifdef __INTEL_COMPILER
typedef long si64;
#else
typedef long long si64;
#endif


#if 0	
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
#endif












typedef struct {
	pthread_t id;

	int thread;
	int threads;

	uint64_t *times[3];
	pthread_barrier_t *barrier;
	
    uint32_t *T1_addr[32];      //beginning address for different columns in the table T1.
    uint32_t T1_bit_width[32]; //bit width for different columns in the table T1.

    uint32_t *T2_addr[32]; //beginning address for different columns in the table T2.
    uint32_t T2_bit_width[32]; //bit width for different columns in the table T2.

    WordUnit* T1_bitmap;     //bit_map of table T1 for the current thread.
    WordUnit* T2_bitmap;     //bit_map of table T2 for the current thread.
	
    uint64_t T1_len;     //size of table T1 for the current thread.
    uint64_t T2_len;     //size of table T2 for the current thread.
 	
} info_t;


void *run(void *arg)
{
	info_t *d = (info_t*) arg;
	assert(pthread_equal(pthread_self(), d->id));
	bind_thread(d->thread, d->threads);
	
	pthread_barrier_t *barrier = d->barrier;
	
	uint64_t i, T1_len = d->T1_len; //length of T1
    uint32_t *l_commitdate  = d->T1_addr[0];
    uint32_t *l_receiptdate = d->T1_addr[1];

	uint32_t T1_bit_width[2];
	T1_bit_width[0] = d->T1_bit_width[0];
	T1_bit_width[1] = d->T1_bit_width[1];
		
    ByteUnit* data_commitdate[4];
    ByteUnit* data_receiptdate[4];
	
	WordUnit *T1_bitmap  = d->T1_bitmap;
	//uint64_t kMemSizePerByteSlice = T1_len; // * sizeof(uint32_t);

  ///////////////////////Generate the byte-level data format..///////////////////////////////////////	
	for (i = 0; i < 4; i++)
	{
      size_t ret  = posix_memalign((void**)&data_commitdate[i], 128, T1_len);   
	  if (ret) {
            printf ( "&data_commitdate[%d]_posix_memalign: %s\n", i, strerror(ret) );
            return NULL;
         }
		 
            ret   = posix_memalign((void**)&data_receiptdate[i], 128, T1_len);   
	  if (ret) {
            printf ( "&data_receiptdate[%d]_posix_memalign: %s\n", i, strerror(ret) );
            return NULL;
         }		 
	}
	
  ///////////////////////Generate the byte-level data format..///////////////////////////////////////	
    int kNumBytesPerCode = (T1_bit_width[0]+7)/8;
	int kNumPaddingBits  = kNumBytesPerCode * 8 - T1_bit_width[0];
	for (i = 0; i < T1_len; i++)
	{
      SetTuple(data_commitdate,  i, l_commitdate[i],  kNumBytesPerCode, kNumPaddingBits);
      SetTuple(data_receiptdate, i, l_receiptdate[i], kNumBytesPerCode, kNumPaddingBits);
	}
	
	//T1_len = 64;
   ///////////////////////first barrier.....///////////////////////
	  pthread_barrier_wait(barrier++);
	  
		uint64_t t1 = thread_time();
	for (int loop = 0; loop < 100; loop++)	
        two_column_cmp(data_commitdate, data_receiptdate, T1_bitmap, T1_len, kNumBytesPerCode);

    ///////////////////////second barrier...../////////////////////		
	  pthread_barrier_wait(barrier++);
	    
		 t1 = thread_time() - t1;
     
	 //test the bitmap is right or not....
     if (d->thread == 2)
	 {   
		 for (size_t ii = 0; ii < T1_len; ii++) //
		{
            bool real  = (l_commitdate[ii] < l_receiptdate[ii]); 
			bool eval  = GetBit(T1_bitmap, ii); //bvblock->GetBit(ii); 
            if (real !=  eval )
			{
              printf("thread_%d_id_%d:  eval: %d, real: %d \n", d->thread, ii, eval, real);
			  return NULL;
			}
  	    }
	 }	
       printf("d-thread = %d, loop = 100, ns = %x\n", d->thread, t1);
		
		d->times[0][d->thread] = t1;

      ///////////////////////second barrier...../////////////////////			
		pthread_barrier_wait(barrier++);
		
		
		
		
		if (d->thread == 0) {
			uint64_t t1 = 0.0;
			for (size_t t = 0 ; t != d->threads ; ++t) {
				t1 += d->times[0][t];
			}
			printf("%2d-bit code, codes_per_ns: %6.3f\n", T1_bit_width[0],
			       (T1_len * d->threads * 1.0) / ((double)t1 / (double)d->threads));
		}
		
#if 0    	
	size_t i;
	size_t bitmasks = tuples / 64;
	size_t words = bitmasks * max_bits;
	rand32_t *gen = rand32_init(d->seed);
	uint32_t *original     = (uint32_t *) alloc(tuples * sizeof(uint32_t));
#ifdef	TWO_COLUMN_ENABLE
	uint32_t *original_1   = (uint32_t *) alloc(tuples * sizeof(uint32_t));
#endif	
	//uint64_t *compressed   = (uint64_t *) alloc(words * sizeof(uint64_t));
	//uint32_t *decompressed = (uint32_t *) alloc(tuples * sizeof(uint32_t));
	uint64_t *bitmap       = (uint64_t *) alloc(bitmasks * sizeof(uint64_t));
	
		
	kMemSizePerByteSlice = 1;	
    //uint32_t value_1 = rand32_next(gen) >> (32 - bits);		
    uint32_t num_    = tuples; //1024;//64; //128;//

	//for (bits = min_bits ; bits <= max_bits ; ++bits) 
	{
		uint32_t value_1 = rand32_next(gen) & ((1<<bits)-1); //rand32_next(gen) >> (32 - bits);
		uint32_t value_2 = rand32_next(gen) & ((1<<bits)-1); //rand32_next(gen) >> (32 - bits);
		uint32_t min_value = min(value_1, value_2);
		uint32_t max_value = max(value_1, value_2);

		BitVectorBlock* bvblock = new BitVectorBlock(num_);
	
    //ByteSliceColumnBlock<16>* block2 = new ByteSliceColumnBlock<16>(num_);
      ByteSliceColumnBlock<BIT_WIDTH_BYTESLICE>* block2   = new ByteSliceColumnBlock<BIT_WIDTH_BYTESLICE>(num_);
#ifdef	TWO_COLUMN_ENABLE	  
      ByteSliceColumnBlock<BIT_WIDTH_BYTESLICE>* block2_1 = new ByteSliceColumnBlock<BIT_WIDTH_BYTESLICE>(num_);
#endif
	  pthread_barrier_wait(barrier++);
		
		// compress
		uint64_t t1 = thread_time();
		   //compress(original, compressed, tuples, bits);
		  // compress(original, data_, tuples, bits);
		       
			   
			 //  uint32_t tmp_cons = rand32_next(gen) & ((1<<bits)-1); 
			   
             for(i=0; i < num_; i++){
	          	uint32_t tmp = rand32_next(gen) & ((1<<bits)-1); //tmp_cons; //rand32_next(gen) >> (32 - bits);
	          	original[i]  = tmp; //tmp_cons; //
                 block2->SetTuple(i, tmp);
				 
#ifdef	TWO_COLUMN_ENABLE				 
				tmp = rand32_next(gen) & ((1<<bits)-1); //rand32_next(gen) >> (32 - bits);
				original_1[i] = tmp;
                 block2_1->SetTuple(i, tmp);
#endif				 
             }
			 
			//testing the data.... 
           if (d->thread == 0)
		   {
             for(i=0; i < num_; i++){
	          	//original[i]  = tmp; //tmp_cons; //
                uint32_t tmp = block2->GetTuple(i);
				if (original[i]  != tmp)
                  printf("tmp = 0x%d, original[i] = 0x%x\n", tmp, original[i]);					
             }			   
		   }

			 
		t1 = thread_time() - t1;
		pthread_barrier_wait(barrier++);

		
		// decompress
		uint64_t t2 = thread_time();
		  // decompress(data_, decompressed, tuples, bits);
		t2 = thread_time() - t2;
		
		
#ifdef INTEL_PCM_ENABLE		
    if (d->thread == 0)
	{   
        printf("bit_width = %d\n", BIT_WIDTH_BYTESLICE);
        PCM_initPerformanceMonitor(&inst_Monitor_Event, NULL);
        PCM_start();
	}
#endif	
		pthread_barrier_wait(barrier++);
		
		
		
		int sb = 1000;
		uint64_t t3 = thread_time();
           block2->Scan(Comparator::kGreaterEqual, min_value, bvblock, Bitwise::kSet);
#ifdef	TWO_COLUMN_ENABLE			   
		   block2_1->Scan(Comparator::kLessEqual, max_value, bvblock, Bitwise::kAnd);
#endif		   
		//compare(compressed, bitmap, tuples, bits, min_value, max_value);
		t3 = thread_time() - t3;
		pthread_barrier_wait(barrier++);
#ifdef INTEL_PCM_ENABLE			
	if (d->thread == 0)
	{
        PCM_stop();
        printf("=====print the profiling result==========\n");//PCM_log("======= Partitioning phase profiling results ======\n");
        PCM_printResults();		
		PCM_cleanup();
	}
#endif

	
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
#ifdef	TWO_COLUMN_ENABLE				
			bool real  = ((original[ii] >= min_value)&&(original_1[ii] <= max_value)); 
#else
            bool real  = ((original[ii] >= min_value)); 
#endif	
			bool eval  = bvblock->GetBit(ii);
            if (real !=  eval )
			{
#ifdef	TWO_COLUMN_ENABLE	
		printf("%d:  eval: %d, real: %d (%d, %d)\n", ii, eval, real, (original[ii] >= min_value), (original_1[ii] <= max_value));
#else 
		printf("%d:  eval: %d, real: %d \n", ii, eval, real);
#endif	
			 //   return NULL;
			}
  	    }
	}	


	}
	free(gen);
	free(original);
#ifdef	TWO_COLUMN_ENABLE	
	free(original_1);
#endif





#endif	






	//free(compressed);
	//free(decompressed);
	//free(bitmap);
	pthread_exit(NULL);
}


  
void main(int argc, char **argv)
{
  uint64_t t, thread_num = argc > 1 ? atoi(argv[1]) : hardware_threads(); //deflaut to use all threads.
  
  
  uint64_t tuples          = 1000000000; //
  
  uint32_t *l_commitdate   =  (uint32_t *) alloc(tuples * sizeof(uint32_t));
  uint32_t *l_receiptdate  =  (uint32_t *) alloc(tuples * sizeof(uint32_t));
  
  uint64_t commitdate_num  = read_one_column("/home/zeke/tpch_experiment/tpch_1g/lineitemWT/l_commitdate.txt_c.txt", l_commitdate);
  uint64_t receiptdate_num = read_one_column("/home/zeke/tpch_experiment/tpch_1g/lineitemWT/l_receiptdate.txt_c.txt", l_receiptdate);
 
  if (commitdate_num != receiptdate_num) 
  {
	  std::cout<<"different number between commitdate and receiptdate: "<<commitdate_num<<":"<<receiptdate_num;
	  return;
  }

  //std::cout << "first\n";  
  std::vector<uint64_t> task_len;
  task_len.resize(thread_num);
  uint64_t size_for_each_thread = compute_task_len_for_each_thread(task_len, commitdate_num, thread_num);
/*
  uint64_t size = commitdate_num;   
  if (threads == 1) //only using one thread
	  task_len[0] = size;
  else
  {
    uint64_t reminder             = 	size/threads;
    uint64_t size_for_each_thread = 	(reminder+63)&(0xffffffc0);
	for (int i = 0; i < threads-1; i++)
	{
		task_len[i] = size_for_each_thread;
		//std::cout << i <<":"<<task_len[i];
	}
	
    task_len[threads-1] = size - size_for_each_thread*(threads-1);
    //std::cout << threads-1 <<":"<<task_len[threads-1];
  }  
*/

  WordUnit *l_bitmap  =  (WordUnit *) alloc((task_len[0]*thread_num/8) + 128);

  
    //initialize 20 barriers to sync between threads.
	int b, barrier_num = 20;
	pthread_barrier_t barrier[barrier_num];
	for (b = 0 ; b != barrier_num; ++b)
		pthread_barrier_init(&barrier[b], NULL, thread_num);

	
    info_t info[thread_num]; //
	uint64_t times[3][thread_num];
	size_t set_bits[thread_num];
	
	//for affinity setting.
    pthread_t tid[thread_num];
    pthread_attr_t attr;
    cpu_set_t set; //cpu_set_t *set = (cpu_set_t *) malloc (sizeof (cpu_set_t)); //
    pthread_attr_init(&attr);
		
	//printf("HHHHHHHHHHHHHHHHHtest 1\n"); //OK
	 
	for (t = 0 ; t != thread_num ; ++t) 
	{

	    //for affinity setting.		
        int cpu_idx = get_cpu_id(t);
        //DEBUGMSG(1, "Assigning thread-%d to CPU-%d\n", i, cpu_idx);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);


		info[t].thread = t;
		info[t].threads = thread_num;
		info[t].barrier = barrier;

		info[t].times[0] = times[0];
		info[t].times[1] = times[1];
		info[t].times[2] = times[2];
		
		info[t].T1_addr[0]      = l_commitdate  + t * size_for_each_thread;
		info[t].T1_bit_width[0] = 12;

		info[t].T1_addr[1]      = l_receiptdate + t * size_for_each_thread;
		info[t].T1_bit_width[1] = 12;
		
		info[t].T1_bitmap       = l_bitmap + t * (size_for_each_thread/64); //result bit for  
		info[t].T1_len          = task_len[t]; //same for all the threads (64*), except the last one 

		printf("task_len[%d] = %d\n", t, task_len[t]);
		
		pthread_create(&info[t].id, &attr, run, (void*) &info[t]); //&info[t].id, NULL
	}
	
   //finish the execution of all threads......	
	for (t = 0 ; t != thread_num ; ++t)
		pthread_join(info[t].id, NULL);
	
	
	
	for (b = 0 ; b != barrier_num; ++b)
		pthread_barrier_destroy(&barrier[b]);
	

/*
  std::cout << 0 <<":"<<task_len[0]<<std::endl;
  std::cout << threads-1 <<":"<<task_len[threads-1]<<std::endl;  
  
  printf("l_commitdate[0] = %d\n", l_commitdate[0]);
  printf("l_commitdate[1] = %d\n", l_commitdate[1]);
  printf("l_commitdate[2] = %d\n", l_commitdate[2]);
   
  printf("l_receiptdate[0] = %d\n", l_receiptdate[0]); 
  printf("l_receiptdate[1] = %d\n", l_receiptdate[1]); 
  printf("l_receiptdate[2] = %d\n", l_receiptdate[2]); 
   
  printf("commitdate_num = %d\n", commitdate_num);
  printf("receiptdate_num = %d\n", receiptdate_num); 
*/


   free (l_commitdate);
   free (l_receiptdate); 

   return;//EXIT_SUCCESS  
/*	
	int min_bits = argc > 1 ? atoi(argv[1]) : 1;
	int max_bits = argc > 2 ? atoi(argv[2]) : 32;

	size_t tuples = argc > 4 ? atoll(argv[4]) : 1000 * 1000 * 1000;

	srand(time(NULL));
	fprintf(stderr, "Threads: %d\n", threads);
#ifdef SIMD_128_MODEL_ENABLE
	fprintf(stderr, "SIMD_128_MODEL_ENABLE!\n");
#endif
	
	

*/	

}



