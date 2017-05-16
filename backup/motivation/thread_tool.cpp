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

	
#ifdef __INTEL_COMPILER
typedef long si64;
#else
typedef long long si64;
#endif


uint32_t min(uint32_t x, uint32_t y) { return x < y ? x : y; }
uint32_t max(uint32_t x, uint32_t y) { return x > y ? x : y; }
	
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


 uint64_t compute_task_len_for_each_thread(std::vector<uint64_t> & task_len, uint64_t size, uint64_t threads)
 {
   if (threads == 1) //only using one thread
   {
	  task_len[0] = size;
	  return size;
   }
   else
   {
     uint64_t reminder             = 	size/threads;
     uint64_t size_for_each_thread = 	(reminder+63)&(0xffffffc0);
	 for (int i = 0; i < threads-1; i++)
	 {
		task_len[i] = size_for_each_thread;
	 }
     task_len[threads-1] = size - size_for_each_thread*(threads-1);
     return size_for_each_thread;
   }
 } 


