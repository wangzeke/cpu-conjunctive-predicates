/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains the common utility about file handling. 
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

#include "types_simd.h"
#include "avx-utility.h"


bool GetBit_from_byte(ByteUnit *bitmap, size_t pos){
	size_t kNumWordBits = 8;
    size_t word_id = pos / kNumWordBits;
    size_t offset = pos % kNumWordBits;
    WordUnit mask = 1 << offset;
    return (bitmap[word_id] & mask);
}


bool GetBit(WordUnit *bitmap, size_t pos){
	size_t kNumWordBits = 64;
    size_t word_id = pos / kNumWordBits;
    size_t offset = pos % kNumWordBits;
    WordUnit mask = 1ULL << offset;
    return (bitmap[word_id] & mask);
}


uint64_t read_one_column(char *in_file, uint32_t *o_array)
{
    //std::vector<int> myintArray;

    std::string line;
    std::ifstream myfile(in_file); //"f1.txt"
	
    uint64_t count = 0; 
	uint32_t result;
	
    while ( std::getline(myfile, line) ) //for (uint32_t result; )
    {
		result = std::stoi(line); 
		
        o_array[count] = result; //myintArray.push_back(result);

        count++; ///std::cout << "Read in the number: " << result << "\n\n";
    }
	return count;
}

//to the right only....
 uint32_t GetTuple(ByteUnit** data_, size_t pos, int kNumBytesPerCode, int kNumPaddingBits) 
 {
    uint32_t ret = 0;
    switch(kNumBytesPerCode){
        case 4:
            ret = (static_cast<uint32_t>(FLIP(data_[0][pos])) << 24) |
                    (static_cast<uint32_t>(FLIP(data_[1][pos])) << 16) |
                    (static_cast<uint32_t>(FLIP(data_[2][pos])) << 8) |
                    static_cast<uint32_t>(FLIP(data_[3][pos]));
            break;
        case 3:
            ret = (static_cast<uint32_t>(FLIP(data_[0][pos])) << 16) |
                    (static_cast<uint32_t>(FLIP(data_[1][pos])) << 8) |
                    static_cast<uint32_t>(FLIP(data_[2][pos]));
            break;
        case 2:
            ret = (static_cast<uint32_t>(FLIP(data_[0][pos])) << 8) |
                    static_cast<uint32_t>(FLIP(data_[1][pos]));
            break;
        case 1:
            ret = static_cast<uint32_t>(FLIP(data_[0][pos]));
            break;
    }

    ret >>= kNumPaddingBits;

    return ret;
 }

void SetTuple(ByteUnit** data_, size_t pos, uint32_t value, int kNumBytesPerCode, int kNumPaddingBits)
{
   value <<= kNumPaddingBits;
    switch(kNumBytesPerCode){
        case 4:
            data_[0][pos] = FLIP(static_cast<ByteUnit>(value >> 24));
            data_[1][pos] = FLIP(static_cast<ByteUnit>(value >> 16));
            data_[2][pos] = FLIP(static_cast<ByteUnit>(value >> 8));
            data_[3][pos] = FLIP(static_cast<ByteUnit>(value));
            break;
        case 3:
            data_[0][pos] = FLIP(static_cast<ByteUnit>(value >> 16));
            data_[1][pos] = FLIP(static_cast<ByteUnit>(value >> 8));
            data_[2][pos] = FLIP(static_cast<ByteUnit>(value));
            break;
        case 2:
            data_[0][pos] = FLIP(static_cast<ByteUnit>(value >> 8));
            data_[1][pos] = FLIP(static_cast<ByteUnit>(value));
            break;
        case 1:
            data_[0][pos] = FLIP(static_cast<ByteUnit>(value));
            break;
    }
}

