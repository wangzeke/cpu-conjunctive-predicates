/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains the function definations of the thread_tool.cpp
 *******************************************************************************/
#ifndef COLUMN_COMPARE_H
#define COLUMN_COMPARE_H

#include "types.h"
	 
void two_column_cmp(ByteUnit** data_1, ByteUnit** data_2, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode); 


#endif