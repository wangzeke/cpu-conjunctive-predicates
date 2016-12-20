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

#include "types_simd.h" 

bool GetBit_from_byte(ByteUnit *bitmap, size_t pos);

bool GetBit(WordUnit *bitmap, size_t pos);

uint64_t read_one_column(char *in_file, uint32_t *o_array);

void SetTuple(ByteUnit** data_, size_t pos, uint32_t value, int kNumBytesPerCode, int kNumPaddingBits);

uint32_t GetTuple(ByteUnit** data_, size_t pos, int kNumBytesPerCode, int kNumPaddingBits); 

void SetTuple_2(ByteUnit** data_, size_t pos, 
                        uint32_t value_1, int kNumBytesPerCode_1, int kNumPaddingBits_1,//first  column
						uint32_t value_2, int kNumBytesPerCode_2, int kNumPaddingBits_2 //second column
               );
			   
WordUnit GetTuple_2(ByteUnit** data_, size_t pos, size_t index, int kNumBytesPerCode, int kNumPaddingBits); 	


void SetTuple_3(ByteUnit** data_, size_t pos, 
                uint32_t value_1, int kNumBytesPerCode_1, int kNumPaddingBits_1,//first  column
				uint32_t value_2, int kNumBytesPerCode_2, int kNumPaddingBits_2,//second column 
                uint32_t value_3, int kNumBytesPerCode_3, int kNumPaddingBits_3 //third  column
               );	

void SetTuple_4(ByteUnit** data_, size_t pos, 
                uint32_t value_1, int kNumBytesPerCode_1, int kNumPaddingBits_1,//first  column
				uint32_t value_2, int kNumBytesPerCode_2, int kNumPaddingBits_2,//second column 
                uint32_t value_3, int kNumBytesPerCode_3, int kNumPaddingBits_3, //third column
				uint32_t value_4, int kNumBytesPerCode_4, int kNumPaddingBits_4
               );			   
#endif

