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


void SetTuple_2(ByteUnit** data_, size_t pos, 
                        uint32_t value_1, int kNumBytesPerCode_1, int kNumPaddingBits_1,//first  column
						uint32_t value_2, int kNumBytesPerCode_2, int kNumPaddingBits_2 //second column
               )
{
   value_1   <<= kNumPaddingBits_1; //padding to the left significant position
   value_2   <<= kNumPaddingBits_2;
   
   size_t slide_size = 32; //AVX2...

   //0:::::::for data_[0], must be 2 bytes for one function call.
   data_[0][(pos/slide_size)*2*slide_size + pos%slide_size             ] = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-1)*8)));
   data_[0][(pos/slide_size)*2*slide_size + pos%slide_size + slide_size] = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-1)*8)));

   //1:::::::for data_[1], may have, when either bytepercode > 1. 
   if ( (kNumBytesPerCode_1>1) | (kNumBytesPerCode_2>1) )
   {
	   if ((kNumBytesPerCode_1>1) & (kNumBytesPerCode_2>1) ) //both codes have the bytes to store....
	   {
         data_[1][(pos/slide_size)*2*slide_size + pos%slide_size             ] = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-2)*8)));
         data_[1][(pos/slide_size)*2*slide_size + pos%slide_size + slide_size] = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-2)*8)));
	   }
	   else if (kNumBytesPerCode_1>1) //only value_1 has at least 2 bytes.
         data_[1][pos]       = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-2)*8)));
	   else if (kNumBytesPerCode_2>1) //only value_2 has at least 2 bytes.
         data_[1][pos]       = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-2)*8)));
   }
   
   //2:::::::for data_[2], may have, when either bytepercode > 2. 
   if ( (kNumBytesPerCode_1>2) | (kNumBytesPerCode_2>2) )
   {
	   if ((kNumBytesPerCode_1>2) & (kNumBytesPerCode_2>2) ) //both codes have the bytes to store....
	   {
         data_[2][(pos/slide_size)*2*slide_size + pos%slide_size             ] = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-3)*8)));
         data_[2][(pos/slide_size)*2*slide_size + pos%slide_size + slide_size] = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-3)*8)));
	   }
	   else if (kNumBytesPerCode_1>2) //only value_1 has at least 3 bytes.
         data_[2][pos]       = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-3)*8)));
	   else if (kNumBytesPerCode_2>2) //only value_2 has at least 3 bytes.
         data_[2][pos]       = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-3)*8)));
   }

   //3:::::::for data_[3], may have, when either bytepercode > 3. 
   if ( (kNumBytesPerCode_1>3) | (kNumBytesPerCode_2>3) )
   {
	   if ((kNumBytesPerCode_1>3) & (kNumBytesPerCode_2>3) ) //both codes have the bytes to store....
	   {
         data_[3][(pos/slide_size)*2*slide_size + pos%slide_size             ] = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-4)*8)));
         data_[3][(pos/slide_size)*2*slide_size + pos%slide_size + slide_size] = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-4)*8)));
	   }
	   else if (kNumBytesPerCode_1>3) //only value_1 has at least 3 bytes.
         data_[3][pos]       = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-4)*8)));
	   else if (kNumBytesPerCode_2>3) //only value_2 has at least 3 bytes.
         data_[3][pos]       = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-4)*8)));
   }
}

WordUnit GetTuple_2(ByteUnit** data_, size_t pos, size_t index, int kNumBytesPerCode, int kNumPaddingBits) 
{  
    WordUnit ret = 0ULL;
	size_t slide_size = 32;
	size_t offset = (index == 1)? slide_size:0;
	
    switch(kNumBytesPerCode){
        case 4:
            ret = (  static_cast<WordUnit>(FLIP(data_[0][(pos/slide_size)*2*slide_size + pos%slide_size + offset])) << 24) |
                    (static_cast<WordUnit>(FLIP(data_[1][(pos/slide_size)*2*slide_size + pos%slide_size + offset])) << 16) |
                    (static_cast<WordUnit>(FLIP(data_[2][(pos/slide_size)*2*slide_size + pos%slide_size + offset])) << 8) |
                     static_cast<WordUnit>(FLIP(data_[3][(pos/slide_size)*2*slide_size + pos%slide_size + offset]));
            break;
        case 3:
            ret = (  static_cast<WordUnit>(FLIP(data_[0][(pos/slide_size)*2*slide_size + pos%slide_size + offset])) << 16) |
                    (static_cast<WordUnit>(FLIP(data_[1][(pos/slide_size)*2*slide_size + pos%slide_size + offset])) << 8) |
                     static_cast<WordUnit>(FLIP(data_[2][(pos/slide_size)*2*slide_size + pos%slide_size + offset]));
            break;
        case 2:
            ret = ( static_cast<WordUnit>(FLIP(data_[0][(pos/slide_size)*2*slide_size + pos%slide_size + offset])) << 8) |
                    static_cast<WordUnit>(FLIP(data_[1][(pos/slide_size)*2*slide_size + pos%slide_size + offset]));
            break;
        case 1:
            ret =   static_cast<WordUnit>(FLIP(data_[0][(pos/slide_size)*2*slide_size + pos%slide_size + offset]));
            break;
    }

    ret >>= kNumPaddingBits;

    return ret;
}

/////////////////suppose kNumBytesPerCode_1 <= kNumBytesPerCode_2 <= kNumBytesPerCode_3//////////////////
void SetTuple_3(ByteUnit** data_, size_t pos, 
                uint32_t value_1, int kNumBytesPerCode_1, int kNumPaddingBits_1,//first  column
				uint32_t value_2, int kNumBytesPerCode_2, int kNumPaddingBits_2,//second column 
                uint32_t value_3, int kNumBytesPerCode_3, int kNumPaddingBits_3 //third  column
               )
{
   value_1   <<= kNumPaddingBits_1; //padding to the left significant position
   value_2   <<= kNumPaddingBits_2;
   value_3   <<= kNumPaddingBits_3;
   
   size_t slide_size = 16; //AVX...
 
   int i = 0;
   //1:::::::at least one column is done... minBytesPerCode_A >= 1:::::::::::::::::
   for (i = 0; i < kNumBytesPerCode_1; i++)
   {
     data_[i][(pos/slide_size)*3*slide_size + (pos%slide_size)               ] = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-1-i)*8)));
     data_[i][(pos/slide_size)*3*slide_size + (pos%slide_size) +   slide_size] = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-1-i)*8)));
     data_[i][(pos/slide_size)*3*slide_size + (pos%slide_size) + 2*slide_size] = FLIP(static_cast<ByteUnit>(value_3 >> ((kNumBytesPerCode_3-1-i)*8)));
   }

   //2:::::::for the case: kNumBytesPerCode_2 > kNumBytesPerCode_1:::::::::::::::::
   for (int i = kNumBytesPerCode_1; i < kNumBytesPerCode_2; i++) //while (i < kNumBytesPerCode_2)//
   {
     data_[i][(pos/slide_size)*2*slide_size + (pos%slide_size)               ] = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-1-i)*8)));
     data_[i][(pos/slide_size)*2*slide_size + (pos%slide_size) +   slide_size] = FLIP(static_cast<ByteUnit>(value_3 >> ((kNumBytesPerCode_3-1-i)*8)));
	 //i++;
   }

   //while (i < kNumBytesPerCode_3)//3:::::::for the case: kNumBytesPerCode_2 > kNumBytesPerCode_1:::::::::::::::::
   for (int i = kNumBytesPerCode_2; i < kNumBytesPerCode_3; i++)
   {
     data_[i][pos]       = FLIP(static_cast<ByteUnit>(value_3 >> ((kNumBytesPerCode_3-1-i)*8)));  
	 //i++;
   }
   
}


/////////////////suppose kNumBytesPerCode_1 <= kNumBytesPerCode_2 <= kNumBytesPerCode_3//////////////////
void SetTuple_4(ByteUnit** data_, size_t pos, 
                uint32_t value_1, int kNumBytesPerCode_1, int kNumPaddingBits_1,//first  column
				uint32_t value_2, int kNumBytesPerCode_2, int kNumPaddingBits_2,//second column 
                uint32_t value_3, int kNumBytesPerCode_3, int kNumPaddingBits_3, //third column
				uint32_t value_4, int kNumBytesPerCode_4, int kNumPaddingBits_4
               )
{
   value_1   <<= kNumPaddingBits_1; //padding to the left significant position
   value_2   <<= kNumPaddingBits_2;
   value_3   <<= kNumPaddingBits_3;
   value_4   <<= kNumPaddingBits_4;
    
   size_t slide_size = 16; //AVX...
 
   int i = 0;
   //1:::::::at least one column is done... minBytesPerCode_A >= 1:::::::::::::::::
   for (i = 0; i < kNumBytesPerCode_1; i++)
   {
     data_[i][(pos/slide_size)*4*slide_size + (pos%slide_size)               ] = FLIP(static_cast<ByteUnit>(value_1 >> ((kNumBytesPerCode_1-1-i)*8)));
     data_[i][(pos/slide_size)*4*slide_size + (pos%slide_size) +   slide_size] = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-1-i)*8)));
     data_[i][(pos/slide_size)*4*slide_size + (pos%slide_size) + 2*slide_size] = FLIP(static_cast<ByteUnit>(value_3 >> ((kNumBytesPerCode_3-1-i)*8)));
     data_[i][(pos/slide_size)*4*slide_size + (pos%slide_size) + 3*slide_size] = FLIP(static_cast<ByteUnit>(value_4 >> ((kNumBytesPerCode_4-1-i)*8)));
   }
  
   //2:::::::for the case: kNumBytesPerCode_2 > kNumBytesPerCode_1:::::::::::::::::
   for (int i = kNumBytesPerCode_1; i < kNumBytesPerCode_2; i++) //while (i < kNumBytesPerCode_2)//
   {
     data_[i][(pos/slide_size)*3*slide_size + (pos%slide_size)               ] = FLIP(static_cast<ByteUnit>(value_2 >> ((kNumBytesPerCode_2-1-i)*8)));
     data_[i][(pos/slide_size)*3*slide_size + (pos%slide_size) +   slide_size] = FLIP(static_cast<ByteUnit>(value_3 >> ((kNumBytesPerCode_3-1-i)*8)));
     data_[i][(pos/slide_size)*3*slide_size + (pos%slide_size) + 2*slide_size] = FLIP(static_cast<ByteUnit>(value_4 >> ((kNumBytesPerCode_4-1-i)*8)));
   }

   //3:::::::for the case: kNumBytesPerCode_3 > kNumBytesPerCode_2:::::::::::::::::
   for (int i = kNumBytesPerCode_2; i < kNumBytesPerCode_3; i++) //while (i < kNumBytesPerCode_2)//
   {
     data_[i][(pos/slide_size)*2*slide_size + (pos%slide_size)               ] = FLIP(static_cast<ByteUnit>(value_3 >> ((kNumBytesPerCode_3-1-i)*8)));
     data_[i][(pos/slide_size)*2*slide_size + (pos%slide_size) +   slide_size] = FLIP(static_cast<ByteUnit>(value_4 >> ((kNumBytesPerCode_4-1-i)*8)));
   }

   //4:::::::::for the case: kNumBytesPerCode_4 > kNumBytesPerCode_3 
   for (int i = kNumBytesPerCode_3; i < kNumBytesPerCode_4; i++)
   {
     data_[i][pos]       = FLIP(static_cast<ByteUnit>(value_4 >> ((kNumBytesPerCode_4-1-i)*8)));  
   }
   
}



