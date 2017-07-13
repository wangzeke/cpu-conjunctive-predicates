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

#include   "types_simd.h"

#include "cpu_mapping.h"
#include "common_tool.h"
#include "column_compare.h"
#include "memory_tool.h"
#include "rand_tool.h"
//
#include "column_compare_with_literal.h" 



//#include 	"byteslice_column_block.h"
//
#define INTEL_PCM_ENABLE

#ifdef INTEL_PCM_ENABLE


#include "perf_counters.h"

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
    0	 
};

#endif

typedef struct {
	pthread_t id;

	int thread;
	int threads;
    
	bool huge_table_enable;
	uint32_t p_s_model_start;
	uint32_t p_s_model_end;
 	
    int       seed;
	uint64_t *times[3];
	pthread_barrier_t *barrier;
	
    uint32_t T1_bit_width; //bit width for different columns in the table T1.
    uint32_t T2_bit_width; //bit width for different columns in the table T2.
    float    T1_selevitity;
    float    T2_selevitity;
	
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
	uint32_t T1_bit_width      = d->T1_bit_width;
	uint32_t T2_bit_width      = d->T2_bit_width;
	
	uint32_t p_s_model_start   = d->p_s_model_start;
	uint32_t p_s_model_end     = d->p_s_model_end;
	float    T1_selevitity     = d->T1_selevitity;
	float    T2_selevitity     = d->T2_selevitity;
		
    int kNumBytesPerCode_1       = (T1_bit_width+7)/8;
	int kNumPaddingBits_1        = kNumBytesPerCode_1 * 8 - T1_bit_width;
    int kNumBytesPerCode_2       = (T2_bit_width+7)/8;
	int kNumPaddingBits_2        = kNumBytesPerCode_2 * 8 - T2_bit_width;

	//printf("kNumBytesPerCode_1 = %d, kNumPaddingBits_1 = %d, kNumBytesPerCode_2 = %d, kNumPaddingBits_2 = %d\n", 
	//        kNumBytesPerCode_1, kNumPaddingBits_1, kNumBytesPerCode_2, kNumPaddingBits_2);
	
	uint32_t literal_1           = (uint32_t) ( (1-T1_selevitity) * (float)((1<<T1_bit_width)-1) ); //do greater than literal...
	uint32_t literal_2           = (uint32_t) (    T2_selevitity  * (float)((1<<T2_bit_width)-1) );   //do less    than literal...

  ///////////////////////Generate the input output data..///////////////////////////////////////	
    uint32_t *original_1, *original_2;  // input original data.  
	ByteUnit *data_1[4], *data_2[4];    //,  *data_2[4];  //compressed to byte boundary.
	WordUnit *bitvector;                //ourput bit map for the original data. 
    uint64_t T1_len_aligned = ( ((T1_len + 63)>>6)<<6 ); //upper boundary to 64-byte alignment.
	
  ///////////////////////allocate space for input data and output bit vector///////////////////////////////////////	
   original_1               = (uint32_t *) malloc_memory(T1_len_aligned*sizeof(uint32_t), false);//use 4K page.	
   if (original_1 == NULL) 
   {
         printf ( "input original_malloc fails\n");
         return NULL;
   }

   //consider to use 2M huge table.
   bitvector                = (WordUnit *) malloc_memory(T1_len_aligned/8, huge_table_enable);//use 4K page.	
   if (bitvector == NULL) 
   {
         printf ( "output bitvector_malloc fails\n");
         return NULL;
   }
   for (i = 0; i < 4; i++) // malloc memory space for two codes. 
   {
      data_1[i]      = (ByteUnit *)malloc_memory(T1_len_aligned*sizeof(ByteUnit), huge_table_enable);	  
	  if (data_1[i] == NULL) {
            printf ( "&data_1[%d]_malloc for two column fails\n", i);
            return NULL;
         }
   }
   
   //assign random value for two columns....
   for(i=0; i < T1_len; i++){
      uint32_t tmp_1 = rand32_next(gen) & ( (1<<T1_bit_width) - 1 ); //tmp_cons; //rand32_next(gen) >> (32 - bits);
      original_1[i]  = tmp_1; //
  
      SetTuple(data_1,  i, tmp_1,  kNumBytesPerCode_1, kNumPaddingBits_1);

   }
   for(i=0; i < T1_len_aligned/64; i++){
      bitvector[i] = 0; //it is used to load to L2 TLB when huge table is used. 
   }

   
	
	
	
for (uint32_t p_s_model = p_s_model_start; p_s_model <= p_s_model_end; p_s_model++)	
{	
	barrier = d->barrier; //reuse the barrier resource.../////
	
	if (d->thread == 0)
	{
	  printf("p_s_model = %d\n", p_s_model);
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
		
    ///////////////////////second barrier.to sync all the threads then begin to execute the code./////////////////////		
	  pthread_barrier_wait(barrier++);
		uint64_t t1 = thread_time(); //
	
      //do the job.....	
	  if (p_s_model == 0)
        column_cmp_with_literal_nP_nS(data_1, literal_1, bitvector, T1_len, kNumBytesPerCode_1, kNumPaddingBits_1);  
      //column_cmp_with_literal_nP_nS(data_, literal, bitvector, T1_len, kNumBytesPerCode, kNumPaddingBits); 
	  else if (p_s_model == 1)
        column_cmp_with_literal_nP_S(data_1, literal_1, bitvector, T1_len, kNumBytesPerCode_1, kNumPaddingBits_1);  
	  else if (p_s_model == 2)
        column_cmp_with_literal_P_nS(data_1, literal_1, bitvector, T1_len, kNumBytesPerCode_1, kNumPaddingBits_1);  
	  else if (p_s_model == 3)
        column_cmp_with_literal_P_S(data_1, literal_1, bitvector, T1_len, kNumBytesPerCode_1, kNumPaddingBits_1);  								 
								   
	///////////////////////third barrier to make sure all the threads have finished the execution/////////////////////		
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
     d->times[0][d->thread] = t1;
	 
	 
    ///////////////fourth barrier to make sure no too much noise comes from the other threads when the thread collects the statistics/////////////////////		
	  pthread_barrier_wait(barrier++);


#if 1	  
	 //test the bitmap is right or not....
     //if (d->thread == 0)
	 {   
		 for (size_t ii = 0; ii < T1_len; ii++) //
		{
            bool real  = ( (original_1[ii] < literal_1) ); 
			bool eval  = GetBit(bitvector, ii); //bvblock->GetBit(ii); 
            if (real !=  eval )
			{ 
		      //printf("original_1[%d] = 0x%x, literal_1 = 0x%x\n", ii, original_1[ii], literal_1);
		      //printf("original_2[%d] = 0x%x, literal_2 = 0x%x\n", ii, original_2[ii], literal_2);
              printf("thread_%d::index_%d:  eval: %d, real: %d \n", d->thread, ii, eval, real);
			  break;
			}
  	    }
	 }	
     //  printf("d-thread = %d, loop = 100, ns = %x\n", d->thread, t1);
#endif		
		
		
		if (d->thread == 0) {
			uint64_t t1 = 0.0;
			for (size_t t = 0 ; t != d->threads ; ++t) {
				t1 += d->times[0][t];
			}
			printf("%2d-%2d-bit codes, time: %6.3f, codes_per_ns: %6.3f\n", T1_bit_width, T2_bit_width, ((double)t1 / (double)d->threads), 
			       (T1_len * d->threads * 1.0) / ((double)t1 / (double)d->threads) );
		}
}
	

	//free(compressed);
	//free(decompressed);
	//free(bitmap);
	pthread_exit(NULL);
}


  
void main(int argc, char **argv)
{
  uint64_t t, thread_num   = argc > 1 ? atoi(argv[1]) : hardware_threads(); //deflaut to use all threads.
  bool huge_table_enable   = argc > 2 ? atoi(argv[2]) : false;              //deflaut to use normal 4k page.
  uint32_t  T1_bit_width   = argc > 3 ? atoi(argv[3]) : 17;                 //default bit width for 1st column
  uint32_t  prefetch_model = argc > 4 ? atoi(argv[4]) : 0;                  //default: enable prefetcher
  float     T1_selevitity  = argc > 5 ? atof(argv[5]) : 0.5; 
  uint32_t  p_s_model_start= argc > 6 ? atoi(argv[6]) : 0;                  //0:nP_nS, 1:nP_S, 2:P_ns, 3:P_S,   
  uint32_t  p_s_model_end  = argc > 7 ? atoi(argv[7]) : 0;                  //4:pf_nP_nS, 1:pf_nP_S, 2:pf_P_ns, 3:pf_P_S,   
  //modify the L2 cache's prefetching model only when the input model is not default value (0). 
  if (prefetch_model != 0)  
    inst_Monitor_Event.prefetch_model = prefetch_model;
   
  uint64_t tuples          = 1000*1000*1000; //

  printf("tuples = %d, thread number = %d, huge_table_enable = %d, T1_bit_width = %d, prefetch_model = %d\n", tuples, thread_num, huge_table_enable, T1_bit_width, prefetch_model); 


  
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
		info[t].huge_table_enable = huge_table_enable;
		info[t].T1_bit_width      = T1_bit_width;
		//info[t].T2_bit_width      = T2_bit_width;		
		info[t].p_s_model_start   = p_s_model_start;
		info[t].p_s_model_end     = p_s_model_end;

		info[t].T1_selevitity     = T1_selevitity;
		//info[t].T2_selevitity     = T2_selevitity;

		
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



