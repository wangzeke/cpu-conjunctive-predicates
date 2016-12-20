#include	<cassert>
#include    <cstdlib>
#include    <cstring>
#include <assert.h> 
#include <stdlib.h> 
#include <sys/mman.h> 

//warning: it is important that we use 128-bit vector for 4 columns.

#include "types_simd.h" 
#include "avx-utility.h" 

#define DEBUG_EN

#define FIRST_COMPARISON_TYPE  Comparator::kGreater
#define SECOND_COMPARISON_TYPE Comparator::kLess
#define THIRD_COMPARISON_TYPE  Comparator::kGreater
#define FOURTH_COMPARISON_TYPE Comparator::kLess


#define PREDICATE_TYPE Bitwise::kAnd
	 
#define PREDICATE_TYPE_1 Bitwise::KOR

//template <size_t BIT_WIDTH, Direction PDIRECTION> 
//template <Comparator CMP>
void four_columns_cmp_with_literal_nP_nS(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2,
								  ByteUnit** data_3, uint32_t literal_3, size_t kNumBytesPerCode_3, size_t kNumPaddingBits_3,
								  ByteUnit** data_4, uint32_t literal_4, size_t kNumBytesPerCode_4, size_t kNumPaddingBits_4
								  ) 
{
// size_t kNumWordBits = 64;
// size_t kNumAvxBits  = 256;
//#define NUM_WORD_BITS 64 
//#define NUM_AVX_BITS 256
 
#ifdef AVX2_DEBUG_ENABLE
    uint64_t counter[4] = {0,0,0,0};
#endif	
    //Prepare byte-slices of literal
    AvxUnit mask_literal_1[kNumBytesPerCode_1];
    literal_1 <<= kNumPaddingBits_1;//literal_1 &= ( (1<<(kNumBytesPerCode_1*8 - kNumPaddingBits_1)) - 1);//kCodeMask;
    AvxUnit mask_literal_2[kNumBytesPerCode_2];
    literal_2 <<= kNumPaddingBits_2;
    AvxUnit mask_literal_3[kNumBytesPerCode_3];
    literal_3 <<= kNumPaddingBits_3;
    AvxUnit mask_literal_4[kNumBytesPerCode_4];
    literal_4 <<= kNumPaddingBits_4;
	
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_1; byte_id++){
         ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_1 >> 8*(kNumBytesPerCode_1 - 1 - byte_id)));
         mask_literal_1[byte_id] = avx_set1<ByteUnit>(byte);
    } 
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_2; byte_id++){
		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_2 >> 8*(kNumBytesPerCode_2 - 1 - byte_id)));
         mask_literal_2[byte_id] = avx_set1<ByteUnit>(byte);
    } 
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_3; byte_id++){
		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_3 >> 8*(kNumBytesPerCode_3 - 1 - byte_id)));
         mask_literal_3[byte_id] = avx_set1<ByteUnit>(byte);
    } 	
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_4; byte_id++){
		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_4 >> 8*(kNumBytesPerCode_4 - 1 - byte_id)));
         mask_literal_4[byte_id] = avx_set1<ByteUnit>(byte);
    } 		
    //for every NUM_WORD_BITS (64) tuples
    for(size_t offset = 0, bv_word_id = 0; offset < len; offset += NUM_WORD_BITS, bv_word_id++)
	{
        WordUnit bitvector_word = WordUnit(0);  
        //need several iteration of AVX scan
        for(size_t i=0; i < NUM_WORD_BITS; i += NUM_AVX_BITS/8) //generate 64-bit result. 
		{  
            AvxUnit m_less_1;
            AvxUnit m_greater_1;
            AvxUnit m_equal_1;
			AvxUnit m_success_1;
			AvxUnit m_fail_1;
            AvxUnit m_less_2;
            AvxUnit m_greater_2;
            AvxUnit m_equal_2; 			
			AvxUnit m_success_2;
			AvxUnit m_fail_2;
            AvxUnit m_less_3;
            AvxUnit m_greater_3;
            AvxUnit m_equal_3; 			
			AvxUnit m_success_3;
			AvxUnit m_fail_3;
            AvxUnit m_less_4;
            AvxUnit m_greater_4;
            AvxUnit m_equal_4; 			
			AvxUnit m_success_4;
			AvxUnit m_fail_4;
			AvxUnit tmpA, tmpB;
			
           // bool move_to_next_segment = true;

            //_mm_prefetch((char const*)(data_1[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            //_mm_prefetch((char const*)(data_2[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            //_mm_prefetch((char const*)(data_3[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            //_mm_prefetch((char const*)(data_4[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
			
	        ////////////////////////approximate stage: for the first byte./////////////////////////////////
            computeKernelWithMask_FIRST<FIRST_COMPARISON_TYPE>(avx_load( (void *)(data_1[0]+offset+i)),
                                                          mask_literal_1[0],
                                                          m_less_1,
                                                          m_greater_1,
                                                          m_equal_1,
                                                          m_success_1,
                                                          m_fail_1);             		 

            computeKernelWithMask_FIRST<SECOND_COMPARISON_TYPE>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2[0],
                                                          m_less_2,
                                                          m_greater_2,
                                                          m_equal_2,
                                                          m_success_2,
                                                          m_fail_2);    			

            computeKernelWithMask_FIRST<THIRD_COMPARISON_TYPE>(avx_load( (void *)(data_3[0]+offset+i)),
                                                          mask_literal_3[0],
                                                          m_less_3,
                                                          m_greater_3,
                                                          m_equal_3,
                                                          m_success_3,
                                                          m_fail_3);    			

            computeKernelWithMask_FIRST<FOURTH_COMPARISON_TYPE>(avx_load( (void *)(data_4[0]+offset+i)),
                                                          mask_literal_4[0],
                                                          m_less_4,
                                                          m_greater_4,
                                                          m_equal_4,
                                                          m_success_4,
                                                          m_fail_4);    															  
          //aggregating the information from all the columns.//// 
			  AvxUnit agg_equal,   agg_equal_a,   agg_equal_b;
			  AvxUnit agg_success, agg_success_a, agg_success_b;
			  AvxUnit agg_fail,    agg_fail_a,    agg_fail_b;		  
              computeForEarlyStop<PREDICATE_TYPE>(m_equal_1,
                                                  m_success_1,
                                                  m_fail_1,
                                                  m_equal_2,
                                                  m_success_2,
                                                  m_fail_2,
                                                  agg_equal_a,
                                                  agg_success_a,
                                                  agg_fail_a); 
              computeForEarlyStop<PREDICATE_TYPE>(m_equal_3,
                                                  m_success_3,
                                                  m_fail_3,
                                                  m_equal_4,
                                                  m_success_4,
                                                  m_fail_4,
                                                  agg_equal_b,
                                                  agg_success_b,
                                                  agg_fail_b); 
            computeForEarlyStop<PREDICATE_TYPE_1>(agg_equal_a,
                                                  agg_success_a,
                                                  agg_fail_a,
                                                  agg_equal_b,
                                                  agg_success_b,
                                                  agg_fail_b,
                                                  agg_equal,
                                                  agg_success,
                                                  agg_fail); 
												  

          //refine stage.		  
		  if (!avx_iszero(agg_equal))
		  {      
            m_equal_1 = avx_and(m_equal_1, agg_equal);
            m_equal_2 = avx_and(m_equal_2, agg_equal);
            m_equal_3 = avx_and(m_equal_3, agg_equal);
			
	        if ( (kNumBytesPerCode_1 > 1) && !avx_iszero( m_equal_1) ) //for the remaining segments of predicate 1.
			{
				ByteUnit *addr_tmp   = data_1[1] + offset + i;
					
                tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256( reinterpret_cast<__m256i*>(addr_tmp) ); //data_1[1]+offset+i
                tmpB                 =  mask_literal_1[1];
                computeKernelWithMask<FIRST_COMPARISON_TYPE>(tmpA,
                                                             tmpB,
                                                             m_less_1,
                                                             m_greater_1,
                                                             m_equal_1,
                                                             m_success_1,
                                                             m_fail_1);
		    	if ( (kNumBytesPerCode_1 > 2) && !avx_iszero( m_equal_1) )//for the third segment of predicate 1.
		    	{
		    		ByteUnit *addr_tmp   =  data_1[2] + offset + i;
                    tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[2]+offset+i
                    tmpB                 =  mask_literal_1[2];
                    computeKernelWithMask<FIRST_COMPARISON_TYPE>(tmpA,
                                                                 tmpB,
                                                                 m_less_1,
                                                                 m_greater_1,
                                                                 m_equal_1,
                                                                 m_success_1,
                                                                 m_fail_1);		
		        	if ( (kNumBytesPerCode_1 > 3) && !avx_iszero(m_equal_1) )//for the fourth segment of predicate 1.
		        	{
		        		ByteUnit *addr_tmp   =  data_1[3] + offset + i;
                        tmpA                 =  avx_load( (void *)addr_tmp );//_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[3]+offset+i
                        tmpB                 =  mask_literal_1[3];
                        computeKernelWithMask<FIRST_COMPARISON_TYPE>(tmpA,
                                                                     tmpB,
                                                                     m_less_1,
                                                                     m_greater_1,
                                                                     m_equal_1,
                                                                     m_success_1,
                                                                     m_fail_1);
                    }			
                }						   
			}

			if ( (kNumBytesPerCode_2 > 1) && !avx_iszero( m_equal_2) ) //for the remaining segments of predicate 2.
			{
				ByteUnit *addr_tmp   = data_2[1] + offset + i;
					
                tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256( reinterpret_cast<__m256i*>(addr_tmp) ); //data_1[1]+offset+i
                tmpB                 =  mask_literal_2[1];
                computeKernelWithMask<SECOND_COMPARISON_TYPE>(tmpA,
                                                             tmpB,
                                                             m_less_2,
                                                             m_greater_2,
                                                             m_equal_2,
                                                             m_success_2,
                                                             m_fail_2);
		    	if ( (kNumBytesPerCode_2 > 2) && !avx_iszero( m_equal_2) )//for the third segment of predicate 1.
		    	{
		    		ByteUnit *addr_tmp   =  data_2[2] + offset + i;
                    tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[2]+offset+i
                    tmpB                 =  mask_literal_2[2];
                    computeKernelWithMask<SECOND_COMPARISON_TYPE>(tmpA,
                                                                 tmpB,
                                                                 m_less_2,
                                                                 m_greater_2,
                                                                 m_equal_2,
                                                                 m_success_2,
                                                                 m_fail_2);		
		        	if ( (kNumBytesPerCode_2 > 3) && !avx_iszero(m_equal_2) )//for the fourth segment of predicate 1.
		        	{
		        		ByteUnit *addr_tmp   =  data_2[3] + offset + i;
                        tmpA                 =  avx_load( (void *)addr_tmp );//_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[3]+offset+i
                        tmpB                 =  mask_literal_2[3];
                        computeKernelWithMask<SECOND_COMPARISON_TYPE>(tmpA,
                                                                     tmpB,
                                                                     m_less_2,
                                                                     m_greater_2,
                                                                     m_equal_2,
                                                                     m_success_2,
                                                                     m_fail_2);
                    }			
                }						   
			}

			if ( (kNumBytesPerCode_3 > 1) && !avx_iszero( m_equal_3) ) //for the remaining segments of predicate 3.
			{
				ByteUnit *addr_tmp   = data_3[1] + offset + i;
					
                tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256( reinterpret_cast<__m256i*>(addr_tmp) ); //data_1[1]+offset+i
                tmpB                 =  mask_literal_3[1];
                computeKernelWithMask<THIRD_COMPARISON_TYPE>(tmpA,
                                                             tmpB,
                                                             m_less_3,
                                                             m_greater_3,
                                                             m_equal_3,
                                                             m_success_3,
                                                             m_fail_3);
		    	if ( (kNumBytesPerCode_3 > 2) && !avx_iszero( m_equal_3) )//for the third segment of predicate 1.
		    	{
		    		ByteUnit *addr_tmp   =  data_3[2] + offset + i;
                    tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[2]+offset+i
                    tmpB                 =  mask_literal_3[2];
                    computeKernelWithMask<THIRD_COMPARISON_TYPE>(tmpA,
                                                                 tmpB,
                                                                 m_less_3,
                                                                 m_greater_3,
                                                                 m_equal_3,
                                                                 m_success_3,
                                                                 m_fail_3);		
		        	if ( (kNumBytesPerCode_3 > 3) && !avx_iszero(m_equal_3) )//for the fourth segment of predicate 1.
		        	{
		        		ByteUnit *addr_tmp   =  data_3[3] + offset + i;
                        tmpA                 =  avx_load( (void *)addr_tmp );//_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[3]+offset+i
                        tmpB                 =  mask_literal_3[3];
                        computeKernelWithMask<THIRD_COMPARISON_TYPE>(tmpA,
                                                                     tmpB,
                                                                     m_less_3,
                                                                     m_greater_3,
                                                                     m_equal_3,
                                                                     m_success_3,
                                                                     m_fail_3);
                    }			
                }						   
			}


			if ( (kNumBytesPerCode_4 > 1) && !avx_iszero( m_equal_4) ) //for the remaining segments of predicate 4.
			{
				ByteUnit *addr_tmp   = data_4[1] + offset + i;
					
                tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256( reinterpret_cast<__m256i*>(addr_tmp) ); //data_1[1]+offset+i
                tmpB                 =  mask_literal_4[1];
                computeKernelWithMask<FOURTH_COMPARISON_TYPE>(tmpA,
                                                             tmpB,
                                                             m_less_4,
                                                             m_greater_4,
                                                             m_equal_4,
                                                             m_success_4,
                                                             m_fail_4);
		    	if ( (kNumBytesPerCode_4 > 2) && !avx_iszero( m_equal_4) )//for the third segment of predicate 1.
		    	{
		    		ByteUnit *addr_tmp   =  data_4[2] + offset + i;
                    tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[2]+offset+i
                    tmpB                 =  mask_literal_4[2];
                    computeKernelWithMask<FOURTH_COMPARISON_TYPE>(tmpA,
                                                                 tmpB,
                                                                 m_less_4,
                                                                 m_greater_4,
                                                                 m_equal_4,
                                                                 m_success_4,
                                                                 m_fail_4);		
		        	if ( (kNumBytesPerCode_4 > 3) && !avx_iszero(m_equal_4) )//for the fourth segment of predicate 1.
		        	{
		        		ByteUnit *addr_tmp   =  data_4[3] + offset + i;
                        tmpA                 =  avx_load( (void *)addr_tmp );//_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[3]+offset+i
                        tmpB                 =  mask_literal_4[3];
                        computeKernelWithMask<FOURTH_COMPARISON_TYPE>(tmpA,
                                                                     tmpB,
                                                                     m_less_4,
                                                                     m_greater_4,
                                                                     m_equal_4,
                                                                     m_success_4,
                                                                     m_fail_4);
                    }			
                }						   
			}
			
		  }
            /////////////combine the result::::generat the vector result for the above computing....//////////////////////////////
            //AvxUnit m_result         = avx_and(m_less_2, m_greater_1);
			   AvxUnit m_result_1, m_result_2, m_result_3, m_result_4, m_result_tmp, m_result_tmp_1, m_result;

               computeMask_one_predicate<FIRST_COMPARISON_TYPE>(
							     m_equal_1,
                                 m_greater_1,
                                 m_less_1,
			                     m_result_1 );
               computeMask_one_predicate<SECOND_COMPARISON_TYPE>(
							     m_equal_2,
                                 m_greater_2,
                                 m_less_2,
			                     m_result_2 );	
               computeMask_one_predicate<THIRD_COMPARISON_TYPE>(
							     m_equal_3,
                                 m_greater_3,
                                 m_less_3,
			                     m_result_3 );	
               computeMask_one_predicate<FOURTH_COMPARISON_TYPE>(
							     m_equal_4,
                                 m_greater_4,
                                 m_less_4,
			                     m_result_4 );				
               computeConjunctivePredicates<PREDICATE_TYPE>(m_result_1,
                                                            m_result_2,
							                                m_result_tmp);
               computeConjunctivePredicates<PREDICATE_TYPE>(m_result_3,
                                                            m_result_4,
							                                m_result_tmp_1);
             computeConjunctivePredicates<PREDICATE_TYPE_1>(m_result_tmp,
                                                            m_result_tmp_1,
							                                m_result);
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << (i));
        }
        WordUnit x          = bitvector_word;
	    bitmap[bv_word_id]  = x; //_mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //

    }
} 

