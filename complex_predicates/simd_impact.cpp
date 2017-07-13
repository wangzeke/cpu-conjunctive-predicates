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
#include "perf_counters.h"

#define INTEL_PCM_ENABLE

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
	}	
};

	
#ifdef __INTEL_COMPILER
typedef long si64;
#else
typedef long long si64;
#endif


void byte_scan_with_m64(const ByteUnit *data, ByteUnit *bitmap, size_t tuples, ByteUnit value)
{   
    __m64 input_64_8, mask_64_8;
	ByteUnit flip_value = FLIP(static_cast<ByteUnit>(value));
	__m64 constant_64_8 = _mm_set1_pi8(flip_value);// _mm_set_pi8 (flip_value, flip_value, flip_value, flip_value, 
	                                               // flip_value, flip_value, flip_value, flip_value);  //_mm_set_pi8 (64, 64, 64, 64, 64, 64, 64, 64);
	
	for (int i = 0; i < tuples; i += 8)
	{
        //(__m64) (*(int64_t*)(data+i)); //_mm_set_pi64x(*(int64_t*)(data+i));//		
		//input_64_8 = *(__m64 *)(data + i);
		//input_64_8 = _mm_set_pi8 (data[i+7], data[i+6], data[i+5], data[i+4], data[i+3], data[i+2], data[i+1], data[i]);
	  	int data_0   = *(int *)(data + i);
		int data_1   = *(int *)(data + i + 4);
		input_64_8   = _mm_set_pi32(data_1, data_0);
		mask_64_8    = _mm_cmpgt_pi8(input_64_8, constant_64_8);
		int mask     =_mm_movemask_pi8(mask_64_8);
		bitmap[i>>3] = (ByteUnit)mask;
	}
    return; 
}

void byte_scan_with_avx(const ByteUnit *data, ByteUnit *bitmap, size_t tuples, ByteUnit value)
{   
    __m128i input_128_8, mask_128_8;
	ByteUnit flip_value    = FLIP(static_cast<ByteUnit>(value));
	__m128i constant_128_8 = _mm_set1_epi8(flip_value); 
	
	for (int i = 0; i < tuples; i += 16)
	{
		input_128_8   = _mm_loadu_si128((__m128i *)(data + i)); //_mm_set_pi32(data_1, data_0);
		mask_128_8    = _mm_cmpgt_epi8(input_128_8, constant_128_8);
		int mask         =_mm_movemask_epi8(mask_128_8);
		*(short *)(bitmap+(i>>3)) = (short)mask;
	}
    return; 
}

void byte_scan_with_avx2(const ByteUnit *data, ByteUnit *bitmap, size_t tuples, ByteUnit value)
{   
    __m256i input_256_8, mask_256_8;
	ByteUnit flip_value    = FLIP(static_cast<ByteUnit>(value));
	__m256i constant_256_8 = _mm256_set1_epi8(flip_value); 
	
	for (int i = 0; i < tuples; i += 32)
	{
		input_256_8             = _mm256_loadu_si256((__m256i *)(data + i)); //_mm_set_pi32(data_1, data_0);
		mask_256_8              = _mm256_cmpgt_epi8(input_256_8, constant_256_8);
		int mask                = _mm256_movemask_epi8(mask_256_8);
		*(int *)(bitmap+(i>>3)) = mask;
	}
    return; 
}


typedef struct {
	pthread_t id;

	int thread;
	int threads;
	int model;

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
	int model               = d->model;
	
	
	uint32_t T1_bit_width[2];
	T1_bit_width[0] = d->T1_bit_width[0];
	T1_bit_width[1] = d->T1_bit_width[1];
		
    ByteUnit* data_input;
    ByteUnit* data_for_scan;
	
    ByteUnit* bitvector;
	
	WordUnit *T1_bitmap  = d->T1_bitmap;
	//uint64_t kMemSizePerByteSlice = T1_len; // * sizeof(uint32_t);


  ///////////////////////Generate the byte-level data format..///////////////////////////////////////	

      size_t ret  = posix_memalign((void**)&data_input, 128, T1_len);   
	  if (ret) {
            printf ( "&data_input_posix_memalign: %s\n", strerror(ret) );
            return NULL;
         }

       ret  = posix_memalign((void**)&data_for_scan, 128, T1_len);   
	  if (ret) {
            printf ( "&data_for_scan_posix_memalign: %s\n", strerror(ret) );
            return NULL;
         }
		 
      ret = posix_memalign((void**)&bitvector, 128, T1_len);   
	  if (ret) {
            printf ( "&bitvector_posix_memalign: %s\n", strerror(ret) );
            return NULL;
         }		 

	
  ///////////////////////Generate the byte-level data format..///////////////////////////////////////	
    int kNumBytesPerCode = (T1_bit_width[0]+7)/8;
	int kNumPaddingBits  = kNumBytesPerCode * 8 - T1_bit_width[0];
	for (i = 0; i < T1_len; i++)
	{
      data_input[i] = (2*i+3*i*i)%256;
	  data_for_scan[i] = FLIP( static_cast<ByteUnit>(data_input[i]) );
	}

   ///////////////////////zero barrier.....///////////////////////
	pthread_barrier_wait(barrier++);
	  
	ByteUnit constant = 64;
	//T1_len = 64;
	
	#ifdef INTEL_PCM_ENABLE		
    if (d->thread == 0)
	{   
        PCM_initPerformanceMonitor(&inst_Monitor_Event, NULL);
        PCM_start();
	}
    #endif	

   ///////////////////////first barrier.....///////////////////////
	  pthread_barrier_wait(barrier++);
      uint64_t t1 = thread_time();
		
	//for (int loop = 0; loop < 1; loop++)
	if (model == 0)
    {
      byte_scan_with_m64(data_for_scan, bitvector, T1_len, constant);
	}		
    else if (model == 1)
	{
      byte_scan_with_avx(data_for_scan, bitvector, T1_len, constant);		
	}
	else if (model == 2)
	{
      byte_scan_with_avx2(data_for_scan, bitvector, T1_len, constant);				
	}
	else 
	{
		printf("wrong execution model. Model ranges from 0--2.\n");
	}
       //printf("id_%d::::in the test barrier\n", d->id);		
       // two_column_cmp(data_input, data_receiptdate, T1_bitmap, T1_len, kNumBytesPerCode);

    ///////////////////////second barrier...../////////////////////		
	  pthread_barrier_wait(barrier++);
		 t1 = thread_time() - t1;
		
#ifdef INTEL_PCM_ENABLE			
	if (d->thread == 0)
	{
        PCM_stop();
        printf("=====print the profiling result==========\n");//PCM_log("======= Partitioning phase profiling results ======\n");
        PCM_printResults();		
		PCM_cleanup();
	}
#endif

    ///////////////////////second barrier...../////////////////////		
	  pthread_barrier_wait(barrier++);			 		 
#if 1    
	 //test the bitmap is right or not....
     //if (d->thread == 2)
	 {   
		 for (size_t ii = 0; ii < T1_len; ii++) //
		{
            bool real  = (data_input[ii] > constant); 
			bool eval  = GetBit_from_byte(bitvector, ii); //bvblock->GetBit(ii); 
            if (real !=  eval )
			{
              printf("thread_%d_index_%d:  eval: %d, real: %d \n", d->thread, ii, eval, real);
			  break; //return NULL;
			}
  	    }
	 }	
       printf("d-thread = %d, ns = %x\n", d->thread, t1);
		
		d->times[0][d->thread] = t1;

      ///////////////////////second barrier...../////////////////////			
		pthread_barrier_wait(barrier++);
	
		
		
		if (d->thread == 0) {
			uint64_t t1 = 0.0;
			for (size_t t = 0 ; t != d->threads ; ++t) {
				t1 += d->times[0][t];
			}
			printf("byte-level predicate, threads: %d, time: %6.3fns, codes_per_ns: %6.3f\n", d->threads,
			      ((double)t1 / (double)d->threads), (T1_len * d->threads * 1.0) / ((double)t1 / (double)d->threads));
		}
#endif

	pthread_exit(NULL);
}


  
void main(int argc, char **argv)
{
  uint64_t t, thread_num = argc > 1 ? atoi(argv[1]) : hardware_threads(); //deflaut to use all threads.
  int         model      = argc > 2 ? atoi(argv[2]) : 0; //deflaut to use 64-bit MMX.
   
  uint64_t tuples          = 1000000000; //
  
  uint32_t *l_commitdate   =  (uint32_t *) alloc(tuples * sizeof(uint32_t));
  uint32_t *l_receiptdate  =  (uint32_t *) alloc(tuples * sizeof(uint32_t));
  
  uint64_t commitdate_num  = tuples;
  uint64_t receiptdate_num = tuples;
 
  if (commitdate_num != receiptdate_num) 
  {
	  std::cout<<"different number between commitdate and receiptdate: "<<commitdate_num<<":"<<receiptdate_num;
	  return;
  }
  
  

  printf("model: %d\n", model);
  printf("the total number of tuples (byte): %d\n", tuples);
  
  //std::cout << "first\n";  
  std::vector<uint64_t> task_len;
  task_len.resize(thread_num);
  uint64_t size_for_each_thread = compute_task_len_for_each_thread(task_len, commitdate_num, thread_num);

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
        //printf("Assigning thread-%d to CPU-%d\n", t, cpu_idx);
        CPU_ZERO(&set);
        CPU_SET(cpu_idx, &set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &set);

		info[t].model = model;          
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

		//printf("task_len[%d] = %d\n", t, task_len[t]);
		
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



