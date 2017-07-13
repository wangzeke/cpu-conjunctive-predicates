/*******************************************************************************
 * Copyright (c) 2017
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: To implement Q19 under Hebe: a deterministic execution scheme.
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
#include "perf_counters.h"
#include "3_column_compare_with_literal.h" 


//#include 	"byteslice_column_block.h"
//
#define INTEL_PCM_ENABLE
 
 void print256_num(AvxUnit var)
{
    uint8_t *val = (uint8_t*) &var;
    printf("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", 
           val[0],    val[1],    val[2],    val[3],    val[4],    val[5],    val[6],    val[7],
           val[8+0],  val[8+1],  val[8+2],  val[8+3],  val[8+4],  val[8+5],  val[8+6],  val[8+7],
           val[16+0], val[16+1], val[16+2], val[16+3], val[16+4], val[16+5], val[16+6], val[16+7],
           val[24+0], val[24+1], val[24+2], val[24+3], val[24+4], val[24+5], val[24+6], val[24+7]
		   );
}
void print256_num_neg(AvxUnit var)
{
    uint8_t *val = (uint8_t*) &var;
    printf("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", 
           val[0]^0x80,    val[1]^0x80,    val[2]^0x80,    val[3]^0x80,    val[4]^0x80,    val[5]^0x80,    val[6]^0x80,    val[7]^0x80,
           val[8+0]^0x80,  val[8+1]^0x80,  val[8+2]^0x80,  val[8+3]^0x80,  val[8+4]^0x80,  val[8+5]^0x80,  val[8+6]^0x80,  val[8+7]^0x80,
           val[16+0]^0x80, val[16+1]^0x80, val[16+2]^0x80, val[16+3]^0x80, val[16+4]^0x80, val[16+5]^0x80, val[16+6]^0x80, val[16+7]^0x80,
           val[24+0]^0x80, val[24+1]^0x80, val[24+2]^0x80, val[24+3]^0x80, val[24+4]^0x80, val[24+5]^0x80, val[24+6]^0x80, val[24+7]^0x80
		   );
}
//#define DEBUG_EN

//print256_num_neg(AvxUnit var);
	
#ifdef __INTEL_COMPILER
typedef long si64;
#else
typedef long long si64;
#endif

#include "types_simd.h" 
#include "avx-utility.h" //warning: it is important that we use 128-bit vector for 3 columns.


#define FIRST_COMPARISON_TYPE  Comparator::kEqual //
#define SECOND_COMPARISON_TYPE Comparator::kEqual
#define THIRD_COMPARISON_TYPE  Comparator::kLess //

#define PREDICATE_TYPE Bitwise::kAnd

        uint32_t literal_1_1     = 2;    //Brand#12  
	    uint32_t literal_2_1_1   = 28;   //p_container = SM CASE
	    uint32_t literal_2_1_2   = 26;   //p_container = SM BOX
	    uint32_t literal_2_1_3   = 31;   //p_container = SM PACK
	    uint32_t literal_2_1_4   = 32;   //p_container = SM PKG
        uint32_t literal_3_1_1   = 1;    //qualility: 1
        uint32_t literal_3_1_2   = 11;   //qualility: 11
        uint32_t literal_4_1_1   = 1;    //p_size: 1
        uint32_t literal_4_1_2   = 5;   //p_size: 5
		
        uint32_t literal_1_2     = 8;    //Brand#23  
	    uint32_t literal_2_2_1   = 17;   //p_container = MED BAG
	    uint32_t literal_2_2_2   = 18;   //p_container = MED BOX
	    uint32_t literal_2_2_3   = 23;   //p_container = MED PACK
	    uint32_t literal_2_2_4   = 24;   //p_container = MED PKG
        uint32_t literal_3_2_1   = 11;    //qualility: 11
        uint32_t literal_3_2_2   = 21;   //qualility: 21
        uint32_t literal_4_2_1   = 1;    //p_size: 1
        uint32_t literal_4_2_2   = 10;   //p_size: 10
		
        uint32_t literal_1_3     = 14;   //Brand#34  
	    uint32_t literal_2_3_1   = 12;   //p_container = LG CASE
	    uint32_t literal_2_3_2   = 10;   //p_container = LG BOX
	    uint32_t literal_2_3_3   = 15;   //p_container = LG PACK
	    uint32_t literal_2_3_4   = 16;   //p_container = LG PKG
        uint32_t literal_3_3_1   = 21;    //qualility: 21
        uint32_t literal_3_3_2   = 31;   //qualility: 31
        uint32_t literal_4_3_1   = 1;    //p_size: 1
        uint32_t literal_4_3_2   = 15;   //p_size: 15
		
        uint32_t literal_5_1   = 1;      //l_shipmode: AIR
        uint32_t literal_5_2   = 5;      //l_shipmode: AIR REG
        uint32_t literal_6     = 2;      //l_shipmode: DELIVER IN PERSON
	 
	 
void Q19_cmp_with_literal_P_S(WordUnit* bitmap,  WordUnit len, 
                              ByteUnit** data_1, ByteUnit** data_2, ByteUnit** data_3, 
							  ByteUnit** data_4, ByteUnit** data_5, ByteUnit** data_6
                             ) 
{
#ifdef DEBUG_EN
    uint64_t counter[4] = {0,0,0,0};
#endif
    //Prepare byte-slices of literal
	size_t kNumBytesPerCode_1 = 1;
	size_t kNumBytesPerCode_2 = 1;
	size_t kNumBytesPerCode_3 = 1;
	size_t kNumPaddingBits_1  = 0;
	
	AvxUnit mask_literal_1_1   = avx_set1<ByteUnit>( (ByteUnit)literal_1_1  );   //Brand#12  
	AvxUnit mask_literal_2_1_1 = avx_set1<ByteUnit>( (ByteUnit)literal_2_1_1);   //p_container = SM CASE
	AvxUnit mask_literal_2_1_2 = avx_set1<ByteUnit>( (ByteUnit)literal_2_1_2);   //p_container = SM BOX
	AvxUnit mask_literal_2_1_3 = avx_set1<ByteUnit>( (ByteUnit)literal_2_1_3);   //p_container = SM PACK
	AvxUnit mask_literal_2_1_4 = avx_set1<ByteUnit>( (ByteUnit)literal_2_1_4);   //p_container = SM PKG
	AvxUnit mask_literal_3_1_1 = avx_set1<ByteUnit>( (ByteUnit)literal_3_1_1);   //qualility: 1
	AvxUnit mask_literal_3_1_2 = avx_set1<ByteUnit>( (ByteUnit)literal_3_1_2);   //qualility: 11
	AvxUnit mask_literal_4_1_1 = avx_set1<ByteUnit>( (ByteUnit)literal_4_1_1);   //p_size: 1
	AvxUnit mask_literal_4_1_2 = avx_set1<ByteUnit>( (ByteUnit)literal_4_1_2);   //p_size: 5

	AvxUnit mask_literal_1_2   = avx_set1<ByteUnit>( (ByteUnit)literal_1_2  );   //Brand#12  
	AvxUnit mask_literal_2_2_1 = avx_set1<ByteUnit>( (ByteUnit)literal_2_2_1);   //p_container = SM CASE
	AvxUnit mask_literal_2_2_2 = avx_set1<ByteUnit>( (ByteUnit)literal_2_2_2);   //p_container = SM BOX
	AvxUnit mask_literal_2_2_3 = avx_set1<ByteUnit>( (ByteUnit)literal_2_2_3);   //p_container = SM PACK
	AvxUnit mask_literal_2_2_4 = avx_set1<ByteUnit>( (ByteUnit)literal_2_2_4);   //p_container = SM PKG
	AvxUnit mask_literal_3_2_1 = avx_set1<ByteUnit>( (ByteUnit)literal_3_2_1);   //qualility: 1
	AvxUnit mask_literal_3_2_2 = avx_set1<ByteUnit>( (ByteUnit)literal_3_2_2);   //qualility: 11
	AvxUnit mask_literal_4_2_1 = avx_set1<ByteUnit>( (ByteUnit)literal_4_2_1);   //p_size: 1
	AvxUnit mask_literal_4_2_2 = avx_set1<ByteUnit>( (ByteUnit)literal_4_2_2);   //p_size: 5

	AvxUnit mask_literal_1_3   = avx_set1<ByteUnit>( (ByteUnit)literal_1_3  );   //Brand#12  
	AvxUnit mask_literal_2_3_1 = avx_set1<ByteUnit>( (ByteUnit)literal_2_3_1);   //p_container = SM CASE
	AvxUnit mask_literal_2_3_2 = avx_set1<ByteUnit>( (ByteUnit)literal_2_3_2);   //p_container = SM BOX
	AvxUnit mask_literal_2_3_3 = avx_set1<ByteUnit>( (ByteUnit)literal_2_3_3);   //p_container = SM PACK
	AvxUnit mask_literal_2_3_4 = avx_set1<ByteUnit>( (ByteUnit)literal_2_3_4);   //p_container = SM PKG
	AvxUnit mask_literal_3_3_1 = avx_set1<ByteUnit>( (ByteUnit)literal_3_3_1);   //qualility: 1
	AvxUnit mask_literal_3_3_2 = avx_set1<ByteUnit>( (ByteUnit)literal_3_3_2);   //qualility: 11
	AvxUnit mask_literal_4_3_1 = avx_set1<ByteUnit>( (ByteUnit)literal_4_3_1);   //p_size: 1
	AvxUnit mask_literal_4_3_2 = avx_set1<ByteUnit>( (ByteUnit)literal_4_3_2);   //p_size: 5

	AvxUnit mask_literal_5_1   = avx_set1<ByteUnit>( (ByteUnit)literal_5_1  );   //AIR  
	AvxUnit mask_literal_5_2   = avx_set1<ByteUnit>( (ByteUnit)literal_5_2  );   //AIR REG  
	AvxUnit mask_literal_6     = avx_set1<ByteUnit>( (ByteUnit)literal_6  );   //DELIVER IN PERSON 
		
    //for every NUM_WORD_BITS (64) tuples
    for(size_t offset = 0, bv_word_id = 0; offset < len; offset += NUM_WORD_BITS, bv_word_id++)
	{
        WordUnit bitvector_word = WordUnit(0);  
        //need several iteration of AVX scan
        for(size_t i=0; i < NUM_WORD_BITS; i += NUM_AVX_BITS/8) //generate 64-bit result. 
		{  
			
			AvxUnit tmpA, tmpB;
           // bool move_to_next_segment = true;

            _mm_prefetch((char const*)(data_1[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_2[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_3[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_4[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_5[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_6[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
			
	        ////////////////////////First column: p_brand = ‘[BRAND1]’./////////////////////////////////
			AvxUnit       m_less_1_1,    m_less_1_2,    m_less_1_3,
					   m_greater_1_1, m_greater_1_2, m_greater_1_3,
					     m_equal_1_1,   m_equal_1_2,   m_equal_1_3,
					   m_success_1_1, m_success_1_2, m_success_1_3,
					      m_fail_1_1,    m_fail_1_2,    m_fail_1_3,
						m_result_1_1,  m_result_1_2,  m_result_1_3; //for tmp result of first column....
			
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_1[0]+offset+i)),
                                                          mask_literal_1_1,
                                                                m_less_1_1,
                                                             m_greater_1_1,
                                                               m_equal_1_1,
                                                             m_success_1_1,
                                                                m_fail_1_1); 
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_1_1,
										m_greater_1_1,
										   m_less_1_1,
			                             m_result_1_1 );																
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_1[0]+offset+i)),
                                                          mask_literal_1_2,
                                                                m_less_1_2,
                                                             m_greater_1_2,
                                                               m_equal_1_2,
                                                             m_success_1_2,
                                                                m_fail_1_2); 
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_1_2,
										m_greater_1_2,
										   m_less_1_2,
			                             m_result_1_2 );																
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_1[0]+offset+i)),
                                                          mask_literal_1_3,
                                                                m_less_1_3,
                                                             m_greater_1_3,
                                                               m_equal_1_3,
                                                             m_success_1_3,
                                                                m_fail_1_3); 																
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_1_3,
										m_greater_1_3,
										   m_less_1_3,
			                             m_result_1_3 );	

										 
										 
	        ////////////////////////Second column: p_container in ( ‘SM CASE’, ‘SM BOX’, ‘SM PACK’, ‘SM PKG’)./////////////////////////////////
			AvxUnit       m_less_2_1_1,    m_less_2_1_2,    m_less_2_1_3,     m_less_2_1_4,
					   m_greater_2_1_1, m_greater_2_1_2, m_greater_2_1_3,  m_greater_2_1_4,
					     m_equal_2_1_1,   m_equal_2_1_2,   m_equal_2_1_3,    m_equal_2_1_4,
					   m_success_2_1_1, m_success_2_1_2, m_success_2_1_3,  m_success_2_1_4,
					      m_fail_2_1_1,    m_fail_2_1_2,    m_fail_2_1_3,     m_fail_2_1_4,
						m_result_2_1_1,  m_result_2_1_2,  m_result_2_1_3,   m_result_2_1_4, m_result_2_1; //for tmp result of first column....

			
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_1_1,
                                                                m_less_2_1_1,
                                                             m_greater_2_1_1,
                                                               m_equal_2_1_1,
                                                             m_success_2_1_1,
                                                                m_fail_2_1_1); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_1_2,
                                                                m_less_2_1_2,
                                                             m_greater_2_1_2,
                                                               m_equal_2_1_2,
                                                             m_success_2_1_2,
                                                                m_fail_2_1_2); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_1_3,
                                                                m_less_2_1_3,
                                                             m_greater_2_1_3,
                                                               m_equal_2_1_3,
                                                             m_success_2_1_3,
                                                                m_fail_2_1_3); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_1_4,
                                                                m_less_2_1_4,
                                                             m_greater_2_1_4,
                                                               m_equal_2_1_4,
                                                             m_success_2_1_4,
                                                                m_fail_2_1_4); 																
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_1_1,
										m_greater_2_1_1,
										   m_less_2_1_1,
			                             m_result_2_1_1 );																
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_1_2,
										m_greater_2_1_2,
										   m_less_2_1_2,
			                             m_result_2_1_2 );	
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_1_3,
										m_greater_2_1_3,
										   m_less_2_1_3,
			                             m_result_2_1_3 );	
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_1_4,
										m_greater_2_1_4,
										   m_less_2_1_4,
			                             m_result_2_1_4 );
						m_result_2_1 = avx_or(avx_or(m_result_2_1_1, m_result_2_1_2), 
						                      avx_or(m_result_2_1_3, m_result_2_1_4));	
            
			//bbbbbbbbb
			AvxUnit       m_less_2_2_1,    m_less_2_2_2,    m_less_2_2_3,     m_less_2_2_4,
					   m_greater_2_2_1, m_greater_2_2_2, m_greater_2_2_3,  m_greater_2_2_4,
					     m_equal_2_2_1,   m_equal_2_2_2,   m_equal_2_2_3,    m_equal_2_2_4,
					   m_success_2_2_1, m_success_2_2_2, m_success_2_2_3,  m_success_2_2_4,
					      m_fail_2_2_1,    m_fail_2_2_2,    m_fail_2_2_3,     m_fail_2_2_4,
						m_result_2_2_1,  m_result_2_2_2,  m_result_2_2_3,   m_result_2_2_4, m_result_2_2; //for tmp result of first column....

			
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_2_1,
                                                                m_less_2_2_1,
                                                             m_greater_2_2_1,
                                                               m_equal_2_2_1,
                                                             m_success_2_2_1,
                                                                m_fail_2_2_1); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_2_2,
                                                                m_less_2_2_2,
                                                             m_greater_2_2_2,
                                                               m_equal_2_2_2,
                                                             m_success_2_2_2,
                                                                m_fail_2_2_2); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_2_3,
                                                                m_less_2_2_3,
                                                             m_greater_2_2_3,
                                                               m_equal_2_2_3,
                                                             m_success_2_2_3,
                                                                m_fail_2_2_3); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_2_4,
                                                                m_less_2_2_4,
                                                             m_greater_2_2_4,
                                                               m_equal_2_2_4,
                                                             m_success_2_2_4,
                                                                m_fail_2_2_4); 																
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_2_1,
										m_greater_2_2_1,
										   m_less_2_2_1,
			                             m_result_2_2_1 );																
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_2_2,
										m_greater_2_2_2,
										   m_less_2_2_2,
			                             m_result_2_2_2 );	
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_2_3,
										m_greater_2_2_3,
										   m_less_2_2_3,
			                             m_result_2_2_3 );	
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_2_4,
										m_greater_2_2_4,
										   m_less_2_2_4,
			                             m_result_2_2_4 );
						m_result_2_2 = avx_or(avx_or(m_result_2_2_1, m_result_2_2_2), 
						                      avx_or(m_result_2_2_3, m_result_2_2_4));	

											  ///////////ccccccc//////////
			AvxUnit       m_less_2_3_1,    m_less_2_3_2,    m_less_2_3_3,     m_less_2_3_4,
					   m_greater_2_3_1, m_greater_2_3_2, m_greater_2_3_3,  m_greater_2_3_4,
					     m_equal_2_3_1,   m_equal_2_3_2,   m_equal_2_3_3,    m_equal_2_3_4,
					   m_success_2_3_1, m_success_2_3_2, m_success_2_3_3,  m_success_2_3_4,
					      m_fail_2_3_1,    m_fail_2_3_2,    m_fail_2_3_3,     m_fail_2_3_4,
						m_result_2_3_1,  m_result_2_3_2,  m_result_2_3_3,   m_result_2_3_4, m_result_2_3; //for tmp result of first column....

			
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_3_1,
                                                                m_less_2_3_1,
                                                             m_greater_2_3_1,
                                                               m_equal_2_3_1,
                                                             m_success_2_3_1,
                                                                m_fail_2_3_1); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_3_2,
                                                                m_less_2_3_2,
                                                             m_greater_2_3_2,
                                                               m_equal_2_3_2,
                                                             m_success_2_3_2,
                                                                m_fail_2_3_2); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_3_3,
                                                                m_less_2_3_3,
                                                             m_greater_2_3_3,
                                                               m_equal_2_3_3,
                                                             m_success_2_3_3,
                                                                m_fail_2_3_3); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2_3_4,
                                                                m_less_2_3_4,
                                                             m_greater_2_3_4,
                                                               m_equal_2_3_4,
                                                             m_success_2_3_4,
                                                                m_fail_2_3_4); 																
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_3_1,
										m_greater_2_3_1,
										   m_less_2_3_1,
			                             m_result_2_3_1 );																
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_3_2,
										m_greater_2_3_2,
										   m_less_2_3_2,
			                             m_result_2_3_2 );	
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_3_3,
										m_greater_2_3_3,
										   m_less_2_3_3,
			                             m_result_2_3_3 );	
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_2_3_4,
										m_greater_2_3_4,
										   m_less_2_3_4,
			                             m_result_2_3_4 );
						m_result_2_3 = avx_or(avx_or(m_result_2_3_1, m_result_2_3_2), 
						                      avx_or(m_result_2_3_3, m_result_2_3_4));	


											  
	        ////////////////////////Third column: l_quantity 1: 11
			AvxUnit       m_less_3_1_1,    m_less_3_1_2, 
					   m_greater_3_1_1, m_greater_3_1_2, 
					     m_equal_3_1_1,   m_equal_3_1_2, 
					   m_success_3_1_1, m_success_3_1_2, 
					      m_fail_3_1_1,    m_fail_3_1_2, 
						m_result_3_1_1,  m_result_3_1_2, m_result_3_1; //for tmp result of first column....

            computeKernelWithMask_FIRST<Comparator::kGreaterEqual>(avx_load( (void *)(data_3[0]+offset+i)),
                                                          mask_literal_3_1_1,
                                                                m_less_3_1_1,
                                                             m_greater_3_1_1,
                                                               m_equal_3_1_1,
                                                             m_success_3_1_1,
                                                                m_fail_3_1_1); 
            computeKernelWithMask_FIRST<Comparator::kLessEqual>(avx_load( (void *)(data_3[0]+offset+i)),
                                                          mask_literal_3_1_2,
                                                                m_less_3_1_2,
                                                             m_greater_3_1_2,
                                                               m_equal_3_1_2,
                                                             m_success_3_1_2,
                                                                m_fail_3_1_2); 
						computeMask_one_predicate<Comparator::kGreaterEqual>(
									      m_equal_3_1_1,
										m_greater_3_1_1,
										   m_less_3_1_1,
			                             m_result_3_1_1 );																
						computeMask_one_predicate<Comparator::kLessEqual>(
									      m_equal_3_1_2,
										m_greater_3_1_2,
										   m_less_3_1_2,
			                             m_result_3_1_2 );	
						m_result_3_1 = avx_and( m_result_3_1_1, m_result_3_1_2 );	
 
                  ///////////////bbbbbbbbbbbbbbbb///////////////
			AvxUnit       m_less_3_2_1,    m_less_3_2_2, 
					   m_greater_3_2_1, m_greater_3_2_2, 
					     m_equal_3_2_1,   m_equal_3_2_2, 
					   m_success_3_2_1, m_success_3_2_2, 
					      m_fail_3_2_1,    m_fail_3_2_2, 
						m_result_3_2_1,  m_result_3_2_2, m_result_3_2; //for tmp result of first column....

            computeKernelWithMask_FIRST<Comparator::kGreaterEqual>(avx_load( (void *)(data_3[0]+offset+i)),
                                                          mask_literal_3_2_1,
                                                                m_less_3_2_1,
                                                             m_greater_3_2_1,
                                                               m_equal_3_2_1,
                                                             m_success_3_2_1,
                                                                m_fail_3_2_1); 
            computeKernelWithMask_FIRST<Comparator::kLessEqual>(avx_load( (void *)(data_3[0]+offset+i)),
                                                          mask_literal_3_2_2,
                                                                m_less_3_2_2,
                                                             m_greater_3_2_2,
                                                               m_equal_3_2_2,
                                                             m_success_3_2_2,
                                                                m_fail_3_2_2); 
						computeMask_one_predicate<Comparator::kGreaterEqual>(
									      m_equal_3_2_1,
										m_greater_3_2_1,
										   m_less_3_2_1,
			                             m_result_3_2_1 );																
						computeMask_one_predicate<Comparator::kLessEqual>(
									      m_equal_3_2_2,
										m_greater_3_2_2,
										   m_less_3_2_2,
			                             m_result_3_2_2 );	
						m_result_3_2 = avx_and( m_result_3_2_1, m_result_3_2_2 );	
 
                  ///////////////ccccccccccccccc///////////////
			AvxUnit       m_less_3_3_1,    m_less_3_3_2, 
					   m_greater_3_3_1, m_greater_3_3_2, 
					     m_equal_3_3_1,   m_equal_3_3_2, 
					   m_success_3_3_1, m_success_3_3_2, 
					      m_fail_3_3_1,    m_fail_3_3_2, 
						m_result_3_3_1,  m_result_3_3_2, m_result_3_3; //for tmp result of first column....

            computeKernelWithMask_FIRST<Comparator::kGreaterEqual>(avx_load( (void *)(data_3[0]+offset+i)),
                                                          mask_literal_3_3_1,
                                                                m_less_3_3_1,
                                                             m_greater_3_3_1,
                                                               m_equal_3_3_1,
                                                             m_success_3_3_1,
                                                                m_fail_3_3_1); 
            computeKernelWithMask_FIRST<Comparator::kLessEqual>(avx_load( (void *)(data_3[0]+offset+i)),
                                                          mask_literal_3_3_2,
                                                                m_less_3_3_2,
                                                             m_greater_3_3_2,
                                                               m_equal_3_3_2,
                                                             m_success_3_3_2,
                                                                m_fail_3_3_2); 
						computeMask_one_predicate<Comparator::kGreaterEqual>(
									      m_equal_3_3_1,
										m_greater_3_3_1,
										   m_less_3_3_1,
			                             m_result_3_3_1 );																
						computeMask_one_predicate<Comparator::kLessEqual>(
									      m_equal_3_3_2,
										m_greater_3_3_2,
										   m_less_3_3_2,
			                             m_result_3_3_2 );	
						m_result_3_3 = avx_and( m_result_3_3_1, m_result_3_3_2 );	
 
 
	        ////////////////////////Fourth column: p_size 1: 11
			AvxUnit       m_less_4_1_1,    m_less_4_1_2, 
					   m_greater_4_1_1, m_greater_4_1_2, 
					     m_equal_4_1_1,   m_equal_4_1_2, 
					   m_success_4_1_1, m_success_4_1_2, 
					      m_fail_4_1_1,    m_fail_4_1_2, 
						m_result_4_1_1,  m_result_4_1_2, m_result_4_1; //for tmp result of first column....

            computeKernelWithMask_FIRST<Comparator::kGreaterEqual>(avx_load( (void *)(data_4[0]+offset+i)),
                                                          mask_literal_4_1_1,
                                                                m_less_4_1_1,
                                                             m_greater_4_1_1,
                                                               m_equal_4_1_1,
                                                             m_success_4_1_1,
                                                                m_fail_4_1_1); 
            computeKernelWithMask_FIRST<Comparator::kLessEqual>(avx_load( (void *)(data_4[0]+offset+i)),
                                                          mask_literal_4_1_2,
                                                                m_less_4_1_2,
                                                             m_greater_4_1_2,
                                                               m_equal_4_1_2,
                                                             m_success_4_1_2,
                                                                m_fail_4_1_2); 
						computeMask_one_predicate<Comparator::kGreaterEqual>(
									      m_equal_4_1_1,
										m_greater_4_1_1,
										   m_less_4_1_1,
			                             m_result_4_1_1 );																
						computeMask_one_predicate<Comparator::kLessEqual>(
									      m_equal_4_1_2,
										m_greater_4_1_2,
										   m_less_4_1_2,
			                             m_result_4_1_2 );	
						m_result_4_1 = avx_and( m_result_4_1_1, m_result_4_1_2 );	
 
                  ///////////////bbbbbbbbbbbbbbbb///////////////
			AvxUnit       m_less_4_2_1,    m_less_4_2_2, 
					   m_greater_4_2_1, m_greater_4_2_2, 
					     m_equal_4_2_1,   m_equal_4_2_2, 
					   m_success_4_2_1, m_success_4_2_2, 
					      m_fail_4_2_1,    m_fail_4_2_2, 
						m_result_4_2_1,  m_result_4_2_2, m_result_4_2; //for tmp result of first column....

            computeKernelWithMask_FIRST<Comparator::kGreaterEqual>(avx_load( (void *)(data_4[0]+offset+i)),
                                                          mask_literal_4_2_1,
                                                                m_less_4_2_1,
                                                             m_greater_4_2_1,
                                                               m_equal_4_2_1,
                                                             m_success_4_2_1,
                                                                m_fail_4_2_1); 
            computeKernelWithMask_FIRST<Comparator::kLessEqual>(avx_load( (void *)(data_4[0]+offset+i)),
                                                          mask_literal_4_2_2,
                                                                m_less_4_2_2,
                                                             m_greater_4_2_2,
                                                               m_equal_4_2_2,
                                                             m_success_4_2_2,
                                                                m_fail_4_2_2); 
						computeMask_one_predicate<Comparator::kGreaterEqual>(
									      m_equal_4_2_1,
										m_greater_4_2_1,
										   m_less_4_2_1,
			                             m_result_4_2_1 );																
						computeMask_one_predicate<Comparator::kLessEqual>(
									      m_equal_4_2_2,
										m_greater_4_2_2,
										   m_less_4_2_2,
			                             m_result_4_2_2 );	
						m_result_4_2 = avx_and( m_result_4_2_1, m_result_4_2_2 );	
 
                  ///////////////ccccccccccccccc///////////////
			AvxUnit       m_less_4_3_1,    m_less_4_3_2, 
					   m_greater_4_3_1, m_greater_4_3_2, 
					     m_equal_4_3_1,   m_equal_4_3_2, 
					   m_success_4_3_1, m_success_4_3_2, 
					      m_fail_4_3_1,    m_fail_4_3_2, 
						m_result_4_3_1,  m_result_4_3_2, m_result_4_3; //for tmp result of first column....

            computeKernelWithMask_FIRST<Comparator::kGreaterEqual>(avx_load( (void *)(data_4[0]+offset+i)),
                                                          mask_literal_4_3_1,
                                                                m_less_4_3_1,
                                                             m_greater_4_3_1,
                                                               m_equal_4_3_1,
                                                             m_success_4_3_1,
                                                                m_fail_4_3_1); 
            computeKernelWithMask_FIRST<Comparator::kLessEqual>(avx_load( (void *)(data_4[0]+offset+i)),
                                                          mask_literal_4_3_2,
                                                                m_less_4_3_2,
                                                             m_greater_4_3_2,
                                                               m_equal_4_3_2,
                                                             m_success_4_3_2,
                                                                m_fail_4_3_2); 
						computeMask_one_predicate<Comparator::kGreaterEqual>(
									      m_equal_4_3_1,
										m_greater_4_3_1,
										   m_less_4_3_1,
			                             m_result_4_3_1 );																
						computeMask_one_predicate<Comparator::kLessEqual>(
									      m_equal_4_3_2,
										m_greater_4_3_2,
										   m_less_4_3_2,
			                             m_result_4_3_2 );	
						m_result_4_3 = avx_and( m_result_4_3_1, m_result_4_3_2 );	
 
 
								
                //////////////////////fifth column/////////////////
                  ///////////////ccccccccccccccc///////////////
			AvxUnit       m_less_5_1,    m_less_5_2, 
					   m_greater_5_1, m_greater_5_2, 
					     m_equal_5_1,   m_equal_5_2, 
					   m_success_5_1, m_success_5_2, 
					      m_fail_5_1,    m_fail_5_2, 
						m_result_5_1,  m_result_5_2, m_result_5; //for tmp result of first column....

            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_5[0]+offset+i)),
                                                          mask_literal_5_1,
                                                                m_less_5_1,
                                                             m_greater_5_1,
                                                               m_equal_5_1,
                                                             m_success_5_1,
                                                                m_fail_5_1); 
            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_5[0]+offset+i)),
                                                          mask_literal_5_2,
                                                                m_less_5_2,
                                                             m_greater_5_2,
                                                               m_equal_5_2,
                                                             m_success_5_2,
                                                                m_fail_5_2); 
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_5_1,
										m_greater_5_1,
										   m_less_5_1,
			                             m_result_5_1 );																
						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_5_2,
										m_greater_5_2,
										   m_less_5_2,
			                             m_result_5_2 );	
						m_result_5 = avx_or( m_result_5_1, m_result_5_2 );	

            /////////////////////sixth column//////////////						
			AvxUnit       m_less_6,
					   m_greater_6,
					     m_equal_6,
					   m_success_6,
					      m_fail_6,
						m_result_6; //for tmp result of first column....

            computeKernelWithMask_FIRST<Comparator::kEqual>(avx_load( (void *)(data_6[0]+offset+i)),
                                                          mask_literal_6,
                                                                m_less_6,
                                                             m_greater_6,
                                                               m_equal_6,
                                                             m_success_6,
                                                                m_fail_6); 

						computeMask_one_predicate<Comparator::kEqual>(
									      m_equal_6,
										m_greater_6,
										   m_less_6,
			                             m_result_6 );																
	
										 
                   /////////////combine the result::::generat the vector result for the above computing....//////////////////////////////
            AvxUnit m_result_a = avx_and(
											avx_and(m_result_1_1, m_result_2_1),
											avx_and(m_result_3_1, m_result_4_1)
										); 
            AvxUnit m_result_b = avx_and(
											avx_and(m_result_1_2, m_result_2_2),
											avx_and(m_result_3_2, m_result_4_2)
										); 	
            AvxUnit m_result_c = avx_and(
											avx_and(m_result_1_3, m_result_2_3),
											avx_and(m_result_3_3, m_result_4_3)
										); 

			 AvxUnit m_result = avx_and(avx_or(avx_or(m_result_a, m_result_b), m_result_c),
										avx_and(m_result_5, m_result_6));							
										 /*
            AvxUnit m_result = avx_and(avx_and(m_equal_1, m_equal_2), m_less_3);
*/			
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << (i));
        }
        WordUnit x          = bitvector_word;
	    _mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //bitmap[bv_word_id]  = x; //

    }
} 





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
    uint32_t T3_bit_width; //bit width for different columns in the table T3.

    float    T1_selevitity;
    float    T2_selevitity;
    float    T3_selevitity;
	
    uint64_t T1_len;     //size of table T1 for the current thread.
    uint64_t T2_len;     //size of table T2 for the current thread.
    uint64_t T3_len;     //size of table T2 for the current thread.	
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

	uint32_t p_s_model_start   = d->p_s_model_start;
	uint32_t p_s_model_end     = d->p_s_model_end;

	uint32_t T1_bit_width      = 8; //d->T1_bit_width;
	uint32_t T2_bit_width      = 8;//d->T2_bit_width;
	uint32_t T3_bit_width      = 8;//d->T3_bit_width;
	
	float    T1_selevitity     = d->T1_selevitity;
	float    T2_selevitity     = d->T2_selevitity;
	float    T3_selevitity     = d->T3_selevitity;
		
    int kNumBytesPerCode_1       = (T1_bit_width+7)/8;
	int kNumPaddingBits_1        = kNumBytesPerCode_1 * 8 - T1_bit_width;
    int kNumBytesPerCode_2       = (T2_bit_width+7)/8;
	int kNumPaddingBits_2        = kNumBytesPerCode_2 * 8 - T2_bit_width;
    int kNumBytesPerCode_3       = (T3_bit_width+7)/8;
	int kNumPaddingBits_3        = kNumBytesPerCode_3 * 8 - T3_bit_width;

	
		
  ///////////////////////Generate the input output data..///////////////////////////////////////	
    uint32_t *original_1, *original_2, *original_3,*original_4, *original_5, *original_6;  // input original data.  
	ByteUnit *data_1[4], *data_2[4], *data_3[4], *data_4[4], *data_5[4], *data_6[4];     //
	WordUnit *bitvector;                //ourput bit map for the original data. 
    uint64_t T1_len_aligned = ( ((T1_len + 63)>>6)<<6 ); //upper boundary to 64-byte alignment.
	
  ///////////////////////allocate space for input data and output bit vector///////////////////////////////////////	
   original_1               = (uint32_t *) malloc_memory(T1_len_aligned*sizeof(uint32_t), false);//use 4K page.	
   if (original_1 == NULL) 
   {
         printf ( "input original_malloc 1 fails\n");
         return NULL;
   }
   original_2               = (uint32_t *) malloc_memory(T1_len_aligned*sizeof(uint32_t), false);//use 4K page.	
   if (original_2 == NULL) 
   {
         printf ( "input original_malloc 2 fails\n");
         return NULL;
   }
   original_3               = (uint32_t *) malloc_memory(T1_len_aligned*sizeof(uint32_t), false);//use 4K page.	
   if (original_3 == NULL) 
   {
         printf ( "input original_malloc 3 fails\n");
         return NULL;
   } 
   original_4               = (uint32_t *) malloc_memory(T1_len_aligned*sizeof(uint32_t), false);//use 4K page.	
   if (original_4 == NULL) 
   {
         printf ( "input original_malloc 1 fails\n");
         return NULL;
   }
   original_5               = (uint32_t *) malloc_memory(T1_len_aligned*sizeof(uint32_t), false);//use 4K page.	
   if (original_5 == NULL) 
   {
         printf ( "input original_malloc 2 fails\n");
         return NULL;
   }
   original_6               = (uint32_t *) malloc_memory(T1_len_aligned*sizeof(uint32_t), false);//use 4K page.	
   if (original_6 == NULL) 
   {
         printf ( "input original_malloc 3 fails\n");
         return NULL;
   }    
   
   //consider to use 2M huge table.
   bitvector              = (WordUnit *) malloc_memory(T1_len_aligned/8, huge_table_enable);//use 4K page.	
   if (bitvector == NULL) 
   {
         printf ( "output bitvector_malloc fails\n");
         return NULL;
   }

   
   for (i = 0; i < 1; i++) // malloc memory space for two codes. 
   {
      data_1[i]      = (ByteUnit *)malloc_memory(T1_len_aligned*sizeof(ByteUnit)*4, huge_table_enable);	  
	  if (data_1[i] == NULL) {
            printf ( "&data_1[%d]_malloc for two column fails\n", i);
            return NULL;
         }
      data_2[i]      = (ByteUnit *)malloc_memory(T1_len_aligned*sizeof(ByteUnit), huge_table_enable);	  
	  if (data_2[i] == NULL) {
            printf ( "&data_2[%d]_malloc for two column fails\n", i);
            return NULL;
         }		
      data_3[i]      = (ByteUnit *)malloc_memory(T1_len_aligned*sizeof(ByteUnit), huge_table_enable);	  
	  if (data_3[i] == NULL) {
            printf ( "&data_3[%d]_malloc for two column fails\n", i);
            return NULL;
         }	
      data_4[i]      = (ByteUnit *)malloc_memory(T1_len_aligned*sizeof(ByteUnit)*4, huge_table_enable);	  
	  if (data_4[i] == NULL) {
            printf ( "&data_4[%d]_malloc for two column fails\n", i);
            return NULL;
         }
      data_5[i]      = (ByteUnit *)malloc_memory(T1_len_aligned*sizeof(ByteUnit), huge_table_enable);	  
	  if (data_5[i] == NULL) {
            printf ( "&data_5[%d]_malloc for two column fails\n", i);
            return NULL;
         }		
      data_6[i]      = (ByteUnit *)malloc_memory(T1_len_aligned*sizeof(ByteUnit), huge_table_enable);	  
	  if (data_6[i] == NULL) {
            printf ( "&data_6[%d]_malloc for two column fails\n", i);
            return NULL;
         }				 
   }   
   
   
   
//	  pthread_barrier_wait(barrier++);
	  FILE *fp_p_brand, *fp_p_container, *fp_l_quantity;//
	  char str[128];
	  uint32_t tmp;
      float    tmp_f;
  //1....................: output_p_brand.txt
      if((fp_p_brand=fopen("../../lineitemWT/output_p_brand.txt","r"))==NULL) {
        printf("cannot open output_p_brand.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_p_brand, "%d\n", &tmp) > 0) { 
		 if (i == T1_len) break;
         original_1[i]       = tmp; //tmp_cons; //
         SetTuple(data_1,  i, tmp,  kNumBytesPerCode_1, kNumPaddingBits_1);		  
	     i++;
      }
      fclose(fp_p_brand);
	  
	   
  //2....................: p_container.txt
      if((fp_p_container=fopen("../../lineitemWT/output_p_container.txt","r"))==NULL) {
        printf("cannot open output_p_container.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_p_container, "%d\n", &tmp) > 0) { 
		 if (i == T1_len) break;
         original_2[i]       = tmp; //tmp_cons; //
         SetTuple(data_2,  i, tmp,  kNumBytesPerCode_2, kNumPaddingBits_2);		  
	     i++;
      }
      fclose(fp_p_container);
	  
	  
  //3....................: l_quantity.txt
      if((fp_l_quantity=fopen("../../lineitemWT/l_quantity.txt","r"))==NULL) {
        printf("cannot open l_quantity.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_l_quantity, "%f\n", &tmp_f) > 0) { 
		 if (i == T1_len) break;
		 tmp = (uint32_t) tmp_f;
         //if (i < 10) printf("tmp = %d\n", tmp);		 
         original_3[i]       = tmp; //tmp_cons; //
         SetTuple(data_3,  i, tmp,  kNumBytesPerCode_3, kNumPaddingBits_3);		  
	     i++;
      }
      fclose(fp_l_quantity);


   FILE *fp_p_size, *fp_l_shipmode, *fp_l_shipinstruct;//
  //4....................: p_size.txt
      if((fp_p_size=fopen("../../lineitemWT/p_size.txt","r"))==NULL) {
        printf("cannot open p_size.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_p_size, "%d\n", &tmp) > 0) { 
         original_4[i]       = tmp; //tmp_cons; //
         SetTuple(data_4,  i, tmp,  kNumBytesPerCode_1, kNumPaddingBits_1);		  
	     i++;
      }
      fclose(fp_p_size);

  //5....................: output_l_shipmode.txt
      if((fp_l_shipmode=fopen("../../lineitemWT/output_l_shipmode.txt","r"))==NULL) {
        printf("cannot open output_l_shipmode.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_l_shipmode, "%d\n", &tmp) > 0) { 
         original_5[i]       = tmp; //tmp_cons; //
         SetTuple(data_5,  i, tmp,  kNumBytesPerCode_1, kNumPaddingBits_1);		  
	     i++;
      }
      fclose(fp_l_shipmode); 

  //6....................: output_l_shipinstruct.txt
      if((fp_l_shipinstruct=fopen("../../lineitemWT/output_l_shipinstruct.txt","r"))==NULL) {
        printf("cannot open output_l_shipinstruct.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_l_shipinstruct, "%d\n", &tmp) > 0) { 
         original_6[i]       = tmp; //tmp_cons; //
         SetTuple(data_6,  i, tmp,  kNumBytesPerCode_1, kNumPaddingBits_1);		  
	     i++;
      }
      fclose(fp_l_shipinstruct); 


	  
   for(i=0; i < T1_len_aligned/64; i++){
      bitvector[i] = 0; //it is used to load to L2 TLB when huge table is used. 
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
	    //T1_len = 64;
		
		Q19_cmp_with_literal_P_S(bitvector, T1_len, 
								data_1, data_2, data_3, data_4, data_5, data_6
								); 
										   
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


	  
	 //test the bitmap is right or not....
     //if (d->thread == 0)
	 {   
		 for (size_t ii = 0; ii < T1_len; ii++) //
		{
			//size_t ii = 11;
			bool real  = ( 
			 (
			  (
				(  original_1[ii] == literal_1_1 ) && // 1
				( (original_2[ii] == literal_2_1_1) || (original_2[ii] == literal_2_1_2) || (original_2[ii] == literal_2_1_3) || (original_2[ii] == literal_2_1_4)  ) && //2
				( (original_3[ii] >= literal_3_1_1) && (original_3[ii] <= literal_3_1_2) )  && //3
				( (original_4[ii] >= literal_4_1_1) && (original_4[ii] <= literal_4_1_2) )     //4
			  ) ||
			  (
				(  original_1[ii] == literal_1_2 ) && // 1
				( (original_2[ii] == literal_2_2_1) || (original_2[ii] == literal_2_2_2) || (original_2[ii] == literal_2_2_3) || (original_2[ii] == literal_2_2_4)  ) && //2
				( (original_3[ii] >= literal_3_2_1) && (original_3[ii] <= literal_3_2_2) )  && //3
				( (original_4[ii] >= literal_4_2_1) && (original_4[ii] <= literal_4_2_2) )     //4
			  ) ||
			  (
				(  original_1[ii] == literal_1_3 ) && // 1
				( (original_2[ii] == literal_2_3_1) || (original_2[ii] == literal_2_3_2) || (original_2[ii] == literal_2_3_3) || (original_2[ii] == literal_2_3_4)  ) && //2
				( (original_3[ii] >= literal_3_3_1) && (original_3[ii] <= literal_3_3_2) )  && //3
				( (original_4[ii] >= literal_4_3_1) && (original_4[ii] <= literal_4_3_2) )     //4
			  )
			 ) && 
				( (original_5[ii] == literal_5_1) || (original_5[ii] == literal_5_2) ) && //5
				(  original_6[ii] ==  literal_6  )                                           //6
			);  
				   
			bool eval  = GetBit(bitvector, ii); //bvblock->GetBit(ii); 
            if ( real !=  eval )
			{  
		      printf("original_1[%d] = %d, original_2[%d] = %d, original_3[%d] = %d\n", ii, original_1[ii], ii, original_2[ii], ii, original_3[ii]);
              printf("thread_%d::index_%d:  eval: %d, real: %d \n", d->thread, ii, eval, real);
			  break;
			}
  	    }
	 }	
     //  printf("d-thread = %d, loop = 100, ns = %x\n", d->thread, t1);
	 
		if (d->thread == 0) {
			uint64_t t1 = 0.0;
			for (size_t t = 0 ; t != d->threads ; ++t) {
				t1 += d->times[0][t];
			}
			printf("%2d-%2d-%2d-bit codes, time: %6.3f, codes_per_ns: %6.3f\n", T1_bit_width, T2_bit_width, T3_bit_width,((double)t1 / (double)d->threads), 
			       (T1_len * d->threads * 1.0) / ((double)t1 / (double)d->threads) );
		}


	//free(compressed);
	//free(decompressed);
	//free(bitmap);
	pthread_exit(NULL);
}


  
void main(int argc, char **argv)
{
  uint64_t t, thread_num   = argc > 1 ? atoi(argv[1]) : 1;                  //deflaut to use one thread.
  bool huge_table_enable   = argc > 2 ? atoi(argv[2]) : false;              //deflaut to use normal 4k page.
  uint32_t  T1_bit_width   = argc > 3 ? atoi(argv[3]) : 17;                 //default bit width for 1st column
  uint32_t  T2_bit_width   = argc > 4 ? atoi(argv[4]) : 17;                 //default bit width for 2nd column
  uint32_t  T3_bit_width   = argc > 5 ? atoi(argv[5]) : 17;                 //default bit width for 2nd column
  uint32_t  prefetch_model = argc > 6 ? atoi(argv[6]) : 0;                  //default: enable prefetcher
  uint32_t  p_s_model_start= argc > 7 ? atoi(argv[7]) : 0;                  //0:nP_nS, 1:nP_S, 2:P_ns, 3:P_S,   
  uint32_t  p_s_model_end  = argc > 8 ? atoi(argv[8]) : 0;                  //4:pf_nP_nS, 1:pf_nP_S, 2:pf_P_ns, 3:pf_P_S,   
  float     T1_selevitity  = argc > 9 ?  atof(argv[9])  : 0.5; 
  float     T2_selevitity  = argc > 10 ? atof(argv[10]) : 0.5; 
  float     T3_selevitity  = argc > 11 ? atof(argv[11]) : 0.5; 
  //modify the L2 cache's prefetching model only when the input model is not default value (0). 
  if (prefetch_model != 0)  
    inst_Monitor_Event.prefetch_model = prefetch_model;
    
  uint64_t tuples          = 60490115;  //59987861;//1000000000; //

  printf("tuples = %d, thread number = %d, huge_table_enable = %d, T1_bit_width = %d, T2_bit_width = %d, T3_bit_width = %d, prefetch_model = %d\n", 
         tuples,       thread_num,         huge_table_enable,      T1_bit_width,      T2_bit_width,      T3_bit_width,      inst_Monitor_Event.prefetch_model); 
 

  
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
		info[t].T2_bit_width      = T2_bit_width;
		info[t].T3_bit_width      = T3_bit_width;
		
		info[t].p_s_model_start   = p_s_model_start;
		info[t].p_s_model_end     = p_s_model_end;
		
		info[t].T1_selevitity     = T1_selevitity;
		info[t].T2_selevitity     = T2_selevitity;
		info[t].T3_selevitity     = T3_selevitity;
		
		info[t].thread            = t;
		info[t].threads           = thread_num;
		info[t].barrier           = barrier;

		info[t].times[0] = times[0];
		info[t].times[1] = times[1];
		info[t].times[2] = times[2];
		
		info[t].T1_len          = tuples;  //task_len[t]; //same for all the threads (64*), except the last one 
		printf("task_len[%d] = %d\n", t, task_len[t]);
		
		pthread_create(&info[t].id, &attr, run, (void*) &info[t]); //&info[t].id, NULL
	}
	
   //finish the execution of all threads......	
	for (t = 0 ; t != thread_num ; ++t)
		pthread_join(info[t].id, NULL);
	
	
	
	for (b = 0 ; b != barrier_num; ++b)
		pthread_barrier_destroy(&barrier[b]);
	
   return;//EXIT_SUCCESS  

}



