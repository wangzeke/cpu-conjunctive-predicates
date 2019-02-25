/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file is used to benchmark the basic execution pattern stpr,
 * which takes into account the compount effect of branch misprediction and cache miss..
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


//I need to add the folder of complex predicte to the search path.
#include <sched.h>              /* CPU_ZERO, CPU_SET */
#include "types_simd.h"
#include "avx-utility.h"
#include "cpu_mapping.h"
#include "common_tool.h"
#include "column_compare.h"
#include "memory_tool.h"
#include "rand_tool.h"
#include "perf_counters.h"


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
    0	 
};



  
void main(int argc, char **argv)
{
	uint64_t tuples           = 1024*1024*1024; //
	uint32_t sel_table_size   = 16*1024;
	uint32_t  exec_model      = argc > 1? atoi(argv[1]) : 0;
	uint32_t  inner_loop_size = argc > 2? atoi(argv[2]) : tuples;
	printf("tuples = %d,  inner_loop_size = %d\n", tuples, inner_loop_size); 

	//initlize random number seed...
	int seed       = rand();
	rand32_t *gen  = rand32_init(seed);

	float    selectivity;
	uint64_t i, t; //deflaut to use all threads. hardware_threads()
  
    ///////////////////////Generate the input output data..///////////////////////////////////////	
    ByteUnit *original;  // input original data.  
	ByteUnit *selectivity_array;  //generate the random value from 0-255.
	ByteUnit *bitvector; //ourput bit map for the original data. 
    //uint64_t T1_len_aligned = ( ((tuples + 63)>>6)<<6 ); //upper boundary to 64-byte alignment.
	
    ///////////////////////allocate space for input data where we perform stpr////////////////////	
    original     = (ByteUnit *) malloc_memory(tuples*sizeof(ByteUnit), false);//use 4K page.	
    if (original == NULL)     {
          printf ("input original_malloc fails\n");
          return;
    }
    for(i=0; i < tuples; i+=8){
      original[i] =  1;
    }
   
   
   /////////////////////////allocate space for the selectivity array/////////////////////// 
    selectivity_array      = (ByteUnit *)malloc_memory(sel_table_size*sizeof(ByteUnit), false);	  
    if (selectivity_array == NULL) {
         printf ( "selectivity_array_malloc fails\n");
         return;
    }
    for(i=0; i < sel_table_size; i++){
      char tmp = rand32_next(gen) & ( (1<<8) - 1 ); 
      selectivity_array[i] =  tmp;
    }    
	

	double f_result[200];         double s_result[200];
	float selectivity_tmp = 0.00;
  
  
	for (int jj = 0; jj <= 100; jj++)
	{   
		selectivity      = selectivity_tmp;
		selectivity_tmp  = selectivity_tmp + 0.01;
		uint32_t literal =  (uint32_t)( selectivity*(float)(1<<8) ); //do greater than literal...
		
	#ifdef INTEL_PCM_ENABLE		
		PCM_initPerformanceMonitor(&inst_Monitor_Event, NULL);
		PCM_start();
	#endif	
	
		uint64_t sel_counter= 0;
		ByteUnit sum        = 0; 
	   	AvxUnit  m_constant = avx_set1<ByteUnit>(100);
	   	AvxUnit  m_sum      = avx_zero();
	
	   	//log the start time for stpr.
	   	uint64_t t1 = thread_time(); //
		if (exec_model == 0)
		{
	   		for (uint64_t i = 0; i < tuples/32; i++)
			{   
				if (selectivity_array[i&(sel_table_size-1)] < literal) 
				{
					//AvxUnit m_equal      = avx_ones();
					//AvxUnit mask_greater = avx_zero();
					//AvxUnit m_data_in    = avx_load( (ByteUnit *)&original[(i*32)&(inner_loop_size-1)] ); //
	   				//mask_greater         = avx_or(mask_greater, avx_and(m_equal, avx_cmpgt<ByteUnit>(m_data_in, m_constant)));
					//m_sum                = avx_or(m_sum, mask_greater); 
					AvxUnit m_data_in    = avx_load( (ByteUnit *)&original[(i*32)&(inner_loop_size-1)] ); //
					m_sum                = avx_or( m_sum, avx_cmpgt<ByteUnit>(m_data_in, m_constant) ); 
					sel_counter++;
				}
			}
		}
		else if (exec_model == 1)
		{
	   		AvxUnit  m_sum_0      = avx_zero(); uint64_t sel_counter_0 = 0;
	   		AvxUnit  m_sum_1      = avx_zero(); uint64_t sel_counter_1 = 0;
	   		AvxUnit  m_sum_2      = avx_zero(); uint64_t sel_counter_2 = 0;
	   		AvxUnit  m_sum_3      = avx_zero(); uint64_t sel_counter_3 = 0;
	   		for (uint64_t i = 0; i < tuples/32; i += 4)
			{   
				if (selectivity_array[(i+0)&(sel_table_size-1)] < literal) 
				{
					AvxUnit m_data_in    = avx_load( (ByteUnit *)&original[((i+0)*32)&(inner_loop_size-1)] ); //
					m_sum_0              = avx_or( m_sum_0, avx_cmpgt<ByteUnit>(m_data_in, m_constant) ); 
					sel_counter_0++;
				}
				if (selectivity_array[(i+1)&(sel_table_size-1)] < literal) 
				{
					AvxUnit m_data_in    = avx_load( (ByteUnit *)&original[((i+1)*32)&(inner_loop_size-1)] ); //
					m_sum_1              = avx_or( m_sum_1, avx_cmpgt<ByteUnit>(m_data_in, m_constant) ); 
					sel_counter_1++;
				}
				if (selectivity_array[(i+2)&(sel_table_size-1)] < literal) 
				{
					AvxUnit m_data_in    = avx_load( (ByteUnit *)&original[((i+2)*32)&(inner_loop_size-1)] ); //
					m_sum_2              = avx_or( m_sum_2, avx_cmpgt<ByteUnit>(m_data_in, m_constant) ); 
					sel_counter_2++;
				}
				if (selectivity_array[(i+0)&(sel_table_size-1)] < literal) 
				{
					AvxUnit m_data_in    = avx_load( (ByteUnit *)&original[((i+3)*32)&(inner_loop_size-1)] ); //
					m_sum_3              = avx_or( m_sum_3, avx_cmpgt<ByteUnit>(m_data_in, m_constant) ); 
					sel_counter_3++;
				}								
			}

			m_sum       = avx_or( avx_or(m_sum_0, m_sum_1), avx_or(m_sum_2, m_sum_3) );
			sel_counter = sel_counter_0 + sel_counter_1 + sel_counter_2 + sel_counter_3;
		}
		else 
		{
			printf("the current execution model: %d is not supported. \n", exec_model);
			return;
		}

	    uint32_t u_sum = _mm256_movemask_epi8(m_sum);  
	
	   	//log the end time for stpr.
	    t1 = thread_time() - t1;
	
	#ifdef INTEL_PCM_ENABLE			
		PCM_stop();
		printf("=====print the profiling result==========tuples = %d, real selectivity = %f\n", tuples, (float)sel_counter/(float)(tuples/32) );
		PCM_printResults();		
		PCM_cleanup();
	#endif
		f_result[jj] = (double)t1/(double)(tuples);
		s_result[jj] = (float)sel_counter/(float)(tuples/32);
		printf("(%d) time required for each code is %f ns/code \n", u_sum, (double)t1/(double)tuples );
	}


	for (uint32_t jj = 0; jj < 101; jj++)
    {
		printf("%3d: selectivity: %f, cost: %f ns/code.\n", jj, s_result[jj], f_result[jj]);
	}

}



