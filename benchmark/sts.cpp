/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file is used to benchmark the basic execution pattern sts,
 * which writes 64-bit data to memory each time.
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
#include <cmath>
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


//#define INTEL_PCM_ENABLE


	
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
   

  
	for (int jj = 0; jj <= 100; jj++)
	{   
		
	#ifdef INTEL_PCM_ENABLE		
		PCM_initPerformanceMonitor(&inst_Monitor_Event, NULL);
		PCM_start();
	#endif	
		
	   	//log the start time for stpr.
	   	uint64_t t1 = thread_time(); //
		if (exec_model == 0)
		{
	   		for (uint64_t i = 0; i < tuples/8; i++)
	   			((uint64_t *)original)[i&(inner_loop_size/8-1)] = 0x12345678; //_mm_stream_si64 (__int64* mem_addr, __int64 a)
		}
		else if (exec_model == 1)
		{
	   		for (uint64_t i = 0; i < tuples/8; i++)
	   			_mm_stream_si64 ((__int64*) original + ( i&(inner_loop_size/8-1) ), 0x12345678);

		}
		else 
		{
			printf("the current execution model: %d is not supported. \n", exec_model);
			return;
		}

	   	//log the end time for stpr.
	    t1 = thread_time() - t1;
	
	#ifdef INTEL_PCM_ENABLE			
		PCM_stop();
		printf("=====print the profiling result==========bytes = %d\n", tuples );
		PCM_printResults();		
		PCM_cleanup();
	#endif
		printf("%d: time required for each bit is %f ns/code \n", jj, (double)t1/(double)tuples/8.0 );
	}

}



