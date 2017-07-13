/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file is used to show the impact of the prefetcher and TLB. 
 * And it is also used to demonestrate the impact of branches.
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


//#include "rdtsc.h"

#include <sched.h>              /* CPU_ZERO, CPU_SET */

#include "types_simd.h"
#include "avx-utility.h"

#include "cpu_mapping.h"
#include "common_tool.h"
#include "column_compare.h"
#include "memory_tool.h"
#include "rand_tool.h"
#include "perf_counters.h"
#include "column_compare_with_literal.h" 


#define INTEL_PCM_ENABLE



	
#ifdef __INTEL_COMPILER
typedef long si64;
#else
typedef long long si64;
#endif


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
    8	 
};


typedef struct {
	pthread_t id;

	int thread;
	int threads;
	
	bool huge_table_enable;
	uint32_t p_s_model;
 	float    selectivity;
    uint32_t stride;    
	
    int seed;
	uint64_t *times[3];
	pthread_barrier_t *barrier;
	
    uint32_t T1_bit_width; //bit width for different columns in the table T1.
    uint32_t T2_bit_width; //bit width for different columns in the table T2.
	
    uint64_t T1_len;     //size of table T1 for the current thread.
    uint64_t T2_len;     //size of table T2 for the current thread.
 	
} info_t;


void *run(void *arg)
{
	info_t *d = (info_t*) arg;
	
	assert(pthread_equal(pthread_self(), d->id));
	bind_thread(d->thread, d->threads);	

	rand32_t *gen              = rand32_init(d->seed);
	pthread_barrier_t *barrier = d->barrier;
    int seed                   = d->seed;
	uint64_t i, T1_len         = d->T1_len;            //number of input codes...
	bool huge_table_enable     = d->huge_table_enable;
	uint32_t p_s_model         = d->p_s_model;
	
    uint32_t T1_bit_width	   = 17;
    float   selectivity	       = d->selectivity;	
	uint32_t stride            = d->stride;
	
    int kNumBytesPerCode       = (T1_bit_width+7)/8;
	int kNumPaddingBits        = kNumBytesPerCode * 8 - T1_bit_width;


  ///////////////////////Generate the input output data..///////////////////////////////////////	
    uint32_t *original;  // input original data.  
	UintUnit *data_;  //compressed to byte boundary.
	WordUnit *bitvector; //ourput bit map for the original data. 
    uint64_t T1_len_aligned = ( ((T1_len + 63)>>6)<<6 ); //upper boundary to 64-byte alignment.
	
	
  ///////////////////////allocate space for input data and output bit vector///////////////////////////////////////	
   original               = (uint32_t *) malloc_memory(T1_len_aligned*sizeof(uint32_t), true);//use 4K page.	
   if (original == NULL) 
   {
         printf ( "input original_malloc fails\n");
         return NULL;
   }
   
   //consider to use 2M huge table.

   for (i = 0; i < 1; i++) // malloc memory space for the codes. 
   {
      //size_t ret  = posix_memalign((void**)&data_commitdate[i], 128, T1_len_aligned);
      data_      = (UintUnit *)malloc_memory(T1_len_aligned*sizeof(AvxUnit), true);	  
	  if (data_ == NULL) {
            printf ( "&data_[%d]_malloc fails\n", i);
            return NULL;
         }
   }
   
   //for load the TLBs into CPU, cache will be flushed by the huge data.
   for(i=0; i < T1_len; i++){
      uint32_t tmp = rand32_next(gen) & ( (1<<T1_bit_width) - 1 ); //tmp_cons; //rand32_next(gen) >> (32 - bits);
      original[i]  = tmp; //
    //  data_[i] =  1;//(data_,  i, tmp,  kNumBytesPerCode, kNumPaddingBits);
   }

	//T1_len = 64; //for debug to print out sevaral cases....
	float selectivity_begin;
	float selectivity_end;
  if (selectivity == 0.0)
  {
	selectivity_begin = 0.01;
	selectivity_end   = 0.01 * 10;	  
	  
  }
  else 
  {
	selectivity_begin = selectivity;
	selectivity_end   = selectivity*10.0;
  } 
  
	  
   for(i=0; i < T1_len*8; i+= 8){
      data_[i] =  1;//(data_,  i, tmp,  kNumBytesPerCode, kNumPaddingBits);
   }
   float selectivity_step = selectivity_begin;
   
for (float selectivity_index = selectivity_begin; selectivity_index <= selectivity_end; selectivity_index += selectivity_step)
{
   barrier     = d->barrier;
   
   uint32_t literal  =  (uint32_t)(selectivity_index * (float)( (1<<T1_bit_width) - 1 )); //do greater than literal...

    if (d->thread == 0)
	{    
      printf("\nselectivity = %f, literal = 0x%x\n", selectivity_index, literal);
    }   
   ///////////////////////first barrier: make sure all threads have finished the initialization.///////////////////////
   //////////Otherwise, the writing operations from the above might increase the memory read/write operations./////////
   pthread_barrier_wait(barrier++);
		
   #ifdef INTEL_PCM_ENABLE		
    if (d->thread == 0)
	{   
        PCM_initPerformanceMonitor(&inst_Monitor_Event, NULL);
        PCM_start();
	}
   #endif	
   
	 int counter = 0;	
    ///////////////////////second barrier.to sync all the threads then begin to execute the code./////////////////////		
	  pthread_barrier_wait(barrier++);
		uint64_t t1 = thread_time(); //
		

      //do the job.....	
	  ByteUnit sum = 0;
	  //if (stride < 8) T1_len *= (8/stride);
      for (int i = 0; i < T1_len; i++)
	  {   //uint32_t tmp = rand32_next(gen) & ( (1<<T1_bit_width) - 1 );
		  if (original[i] < literal) //if(tmp < literal) //
		  {
			  sum |= ((ByteUnit*)data_)[(stride*i)/8];
			  counter++;
		  }
	  }
/*
	  UintUnit sum = 0;
      for (int i = 0; i < T1_len; i++)
	  {   //uint32_t tmp = rand32_next(gen) & ( (1<<T1_bit_width) - 1 );
		  if (original[i] < literal) //if(tmp < literal) //
		  {
			  sum |= ((UintUnit*)data_)[stride*i];
			  counter++;
		  }
	  } 
*/	  
/*
	  AvxUnit simd_sum = avx_zero();
      AvxUnit simd_literal =  avx_set1<UintUnit>(literal);
	  AvxUnit simd_mask;
	  
      for (int i = 0; i < T1_len; i += 32)
	  {   
          AvxUnit simd_data_0 = avx_load( &original[i + 0] );
          AvxUnit simd_data_1 = avx_load( &original[i + 8] );
          AvxUnit simd_data_2 = avx_load( &original[i + 16] );
          AvxUnit simd_data_3 = avx_load( &original[i + 24] );
		  
		  AvxUnit simd_lt_mask_0 = avx_cmplt<UintUnit>(simd_data_0, simd_literal);
		  AvxUnit simd_lt_mask_1 = avx_cmplt<UintUnit>(simd_data_1, simd_literal);
		  AvxUnit simd_lt_mask_2 = avx_cmplt<UintUnit>(simd_data_2, simd_literal);
		  AvxUnit simd_lt_mask_3 = avx_cmplt<UintUnit>(simd_data_3, simd_literal);
		          simd_mask      = avx_or(avx_or(simd_lt_mask_0, simd_lt_mask_1), avx_or(simd_lt_mask_2, simd_lt_mask_3));
				  
		  if (!avx_iszero(simd_mask)) //if(tmp < literal) //
		  {   
		      AvxUnit sel_data_0 = avx_load(&data_[0][i]); //sum &= data_[0][i];
			            simd_sum = avx_or(simd_sum, sel_data_0);
			  counter+=32;
		  }
	  }
	  
	  _mm256_stream_si256 ( (__m256i *)(&data_[0][0]), simd_sum); 
    ///////////////////////third barrier to make sure all the threads have finished the execution/////////////////////		
*/
	pthread_barrier_wait(barrier++);
		 t1 = thread_time() - t1;

#ifdef INTEL_PCM_ENABLE			
	if (d->thread == 0)
	{
        PCM_stop();
        printf("=====print the profiling result==========sum = %d, real selectivity = %f\n", sum, (float)counter/(float)T1_len);
        PCM_printResults();		
		PCM_cleanup();
		//printf("real selectivity = %f\n", (float)counter/(float)T1_len);
	}
#endif		
	  pthread_barrier_wait(barrier++);
}	 




	//free(compressed);
	//free(decompressed);
	//free(bitmap);
	pthread_exit(NULL);
}


  
void main(int argc, char **argv)
{
  uint64_t t, thread_num   = argc > 1 ? atoi(argv[1]) : hardware_threads(); //deflaut to use all threads.
  uint32_t  prefetch_model = argc > 2 ? atoi(argv[2]) : 0;                  //default: enable prefetcher
  uint32_t  stride         = argc > 3 ? atoi(argv[3]) : 1;                //default: 1
  float    selectivity     = argc > 4 ? atof(argv[4]) : 0.01;              //default selectivity.
   
  //modify the L2 cache's prefetching model only when the input model is not default value (0). 
  if (prefetch_model != 0)  
    inst_Monitor_Event.prefetch_model = prefetch_model;
  
  uint64_t tuples          = 100*1000*1000; //

  printf("\ntuples = %d, stride = %d, thread number = %d, prefetch_model = %d\n", tuples, stride, thread_num, prefetch_model); 


  
  	srand(time(NULL));

  //std::cout << "first\n";  
  std::vector<uint64_t> task_len;
  task_len.resize(thread_num);
  uint64_t size_for_each_thread = compute_task_len_for_each_thread(task_len, tuples, thread_num);


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

        info[t].seed              = rand();
		info[t].selectivity       = selectivity;
		info[t].stride            = stride;
		
		info[t].thread            = t;
		info[t].threads           = thread_num;
		info[t].barrier           = barrier;

		info[t].times[0] = times[0];
		info[t].times[1] = times[1];
		info[t].times[2] = times[2];
		
		info[t].T1_len          = task_len[t]; //same for all the threads (64*), except the last one 
		//printf("task_len[%d] = %d\n", t, task_len[t]);
		pthread_create(&info[t].id, &attr, run, (void*) &info[t]); //&info[t].id, NULL
	}
	
   //finish the execution of all threads......	
	for (t = 0 ; t != thread_num ; ++t)
		pthread_join(info[t].id, NULL);
	
	
	
	for (b = 0 ; b != barrier_num; ++b)
		pthread_barrier_destroy(&barrier[b]);
	
   return;//EXIT_SUCCESS  

}



