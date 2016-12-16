/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains the function definations of the thread_tool.cpp
 *******************************************************************************/
#ifndef THREAD_TOOL_H
#define THREAD_TOOL_H

#include "types_simd.h"
	 
uint64_t thread_time(void);

void bind_thread(int thread, int threads); 

void *alloc(uint64_t bytes);

int hardware_threads(void);

uint64_t compute_task_len_for_each_thread(std::vector<uint64_t> & task_len, uint64_t size, uint64_t threads);


#endif