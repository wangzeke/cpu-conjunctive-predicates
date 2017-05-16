/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains . 

 * See file LICENSE.md for details.
 *******************************************************************************/
#ifndef FILE_TOOL_H
#define FILE_TOOL_H

#include "types.h" 

bool GetBit(WordUnit *bitmap, size_t pos);

uint64_t read_one_column(char *in_file, uint32_t *o_array);

void SetTuple(ByteUnit** data_, size_t pos, uint32_t value, int kNumBytesPerCode, int kNumPaddingBits);

uint32_t GetTuple(ByteUnit** data_, size_t pos, int kNumBytesPerCode, int kNumPaddingBits); 

#endif

