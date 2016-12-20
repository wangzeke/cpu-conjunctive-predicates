#include	<cassert>
#include    <cstdlib>
#include    <cstring>
#include <assert.h> 
#include <stdlib.h> 
#include <sys/mman.h> 
 
#include "types_simd.h" 
#include "avx-utility.h"
//#include "2_column_compare_with_literal.h"  Comparator::kGreater

#define DEBUG_EN

#define FIRST_COMPARISON_TYPE  Comparator::kGreater//Comparator::kInequal
#define SECOND_COMPARISON_TYPE Comparator::kLess//Comparator::kInequal
#define PREDICATE_TYPE Bitwise::kAnd
	 
	 
//template <size_t BIT_WIDTH, Direction PDIRECTION> 
//template <Comparator CMP>
void two_columns_cmp_with_literal_nP_nS(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2
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
    literal_2 <<= kNumPaddingBits_2;//literal_2 &= ( (1<<(kNumBytesPerCode_2*8 - kNumPaddingBits_2)) - 1);//kCodeMask;
	
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_1; byte_id++){
         ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_1 >> 8*(kNumBytesPerCode_1 - 1 - byte_id)));
         mask_literal_1[byte_id] = avx_set1<ByteUnit>(byte);
    } 
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_2; byte_id++){
		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_2 >> 8*(kNumBytesPerCode_2 - 1 - byte_id)));
         mask_literal_2[byte_id] = avx_set1<ByteUnit>(byte);
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
			AvxUnit tmpA, tmpB;
			
           // bool move_to_next_segment = true;

            //_mm_prefetch((char const*)(data_1[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            //_mm_prefetch((char const*)(data_2[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
			
	        ////////////////////////00000: for the first byte./////////////////////////////////
            computeKernelWithMask_FIRST<FIRST_COMPARISON_TYPE>(avx_load( (void *)(data_1[0]+offset+i)),
                                                          mask_literal_1[0],
                                                          m_less_1,
                                                          m_greater_1,
                                                          m_equal_1,
                                                          m_success_1,
                                                          m_fail_1);             		 

            computeKernelWithMask_FIRST<SECOND_COMPARISON_TYPE>( avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2[0],
                                                          m_less_2,
                                                          m_greater_2,
                                                          m_equal_2,
                                                          m_success_2,
                                                          m_fail_2);    			

														  
          //aggregating the information from all the columns.//// 
			  AvxUnit agg_equal;
			  AvxUnit agg_success;
			  AvxUnit agg_fail;		  
              computeForEarlyStop<PREDICATE_TYPE>( m_equal_1,
                                                  m_success_1,
                                                  m_fail_1,
                                                  m_equal_2,
                                                  m_success_2,
                                                  m_fail_2,
                                                  agg_equal,
                                                  agg_success,
                                                  agg_fail); 

/*			   	 m_equal_1 = avx_and(m_equal_1, avx_not(m_greater_2) );
			     m_equal_2 = avx_and(m_equal_2, avx_not(m_less_1) ); */
          //refine stage.		  
		  if (!avx_iszero(agg_equal))
		  {      
            m_equal_1 = avx_and(m_equal_1, agg_equal);
            m_equal_2 = avx_and(m_equal_2, agg_equal);
			
	        if ( (kNumBytesPerCode_1 > 1) && !avx_iszero( m_equal_1) ) //for the second segment of predicate 1.
			{
				ByteUnit *addr_tmp   = data_1[1] + offset + i;
					
                tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256( reinterpret_cast<__m256i*>(addr_tmp) ); //data_1[1]+offset+i
                tmpB                 =  mask_literal_1[1];
                computeKernel<FIRST_COMPARISON_TYPE, false>(tmpA,
                                                             tmpB,
                                                             m_less_1,
                                                             m_greater_1,
                                                             m_equal_1);
		    	if ( (kNumBytesPerCode_1 > 2) && !avx_iszero( m_equal_1) )//for the third segment of predicate 1.
		    	{
		    		ByteUnit *addr_tmp   =  data_1[2] + offset + i;
                    tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[2]+offset+i
                    tmpB                 =  mask_literal_1[2];
                    computeKernel<FIRST_COMPARISON_TYPE, false>(tmpA,
                                                                 tmpB,
                                                                 m_less_1,
                                                                 m_greater_1,
                                                                 m_equal_1);		
		        	if ( (kNumBytesPerCode_1 > 3) && !avx_iszero(m_equal_1) )//for the fourth segment of predicate 1.
		        	{
		        		ByteUnit *addr_tmp   =  data_1[3] + offset + i;
                        tmpA                 =  avx_load( (void *)addr_tmp );//_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[3]+offset+i
                        tmpB                 =  mask_literal_1[3];
                        computeKernel<FIRST_COMPARISON_TYPE, true>(tmpA,
                                                                     tmpB,
                                                                     m_less_1,
                                                                     m_greater_1,
                                                                     m_equal_1);
                    }			
                }						   
			}

			if ( (kNumBytesPerCode_2 > 1) && !avx_iszero( m_equal_2) ) //for the second segment of predicate 1.
			{
				ByteUnit *addr_tmp   = data_2[1] + offset + i;
					
                tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256( reinterpret_cast<__m256i*>(addr_tmp) ); //data_1[1]+offset+i
                tmpB                 =  mask_literal_2[1];
                computeKernel<SECOND_COMPARISON_TYPE, false>(tmpA,
                                                             tmpB,
                                                             m_less_2,
                                                             m_greater_2,
                                                             m_equal_2);
		    	if ( (kNumBytesPerCode_2 > 2) && !avx_iszero( m_equal_2) )//for the third segment of predicate 1.
		    	{
		    		ByteUnit *addr_tmp   =  data_2[2] + offset + i;
                    tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[2]+offset+i
                    tmpB                 =  mask_literal_2[2];
                    computeKernel<SECOND_COMPARISON_TYPE, false>(tmpA,
                                                                 tmpB,
                                                                 m_less_2,
                                                                 m_greater_2,
                                                                 m_equal_2);		
		        	if ( (kNumBytesPerCode_2 > 3) && !avx_iszero(m_equal_2) )//for the fourth segment of predicate 1.
		        	{
		        		ByteUnit *addr_tmp   =  data_2[3] + offset + i;
                        tmpA                 =  avx_load( (void *)addr_tmp );//_mm256_loadu_si256(reinterpret_cast<__m256i*>(addr_tmp)); //data_1[3]+offset+i
                        tmpB                 =  mask_literal_2[3];
                        computeKernel<SECOND_COMPARISON_TYPE, true>(tmpA,
                                                                     tmpB,
                                                                     m_less_2,
                                                                     m_greater_2,
                                                                     m_equal_2);
                    }			
                }						   
			}

		  }
		   
            /////////////combine the result::::generat the vector result for the above computing....//////////////////////////////
            //AvxUnit m_result         = avx_and(m_less_2, m_greater_1);
			   AvxUnit m_result;
			   computeFinalMask<FIRST_COMPARISON_TYPE, SECOND_COMPARISON_TYPE, PREDICATE_TYPE>( m_equal_1,
                                 m_greater_1,
                                 m_less_1,
							     m_equal_2,
                                 m_greater_2,
                                 m_less_2,
								 m_result); 				 

            //move mask from vector register to general purpose register (8-bit --> 1-bit).
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << (i));
        }
        WordUnit x          = bitvector_word;
	    bitmap[bv_word_id]  = x; //_mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //

    }
} 

	 
//template <size_t BIT_WIDTH, Direction PDIRECTION> 
//template <Comparator CMP>
void two_columns_cmp_with_literal_nP_S(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2
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
    literal_2 <<= kNumPaddingBits_2;//literal_2 &= ( (1<<(kNumBytesPerCode_2*8 - kNumPaddingBits_2)) - 1);//kCodeMask;
	
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_1; byte_id++){
         ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_1 >> 8*(kNumBytesPerCode_1 - 1 - byte_id)));
         mask_literal_1[byte_id] = avx_set1<ByteUnit>(byte);
    } 
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_2; byte_id++){
		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_2 >> 8*(kNumBytesPerCode_2 - 1 - byte_id)));
         mask_literal_2[byte_id] = avx_set1<ByteUnit>(byte);
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
			AvxUnit tmpA, tmpB;
			
            bool move_to_next_segment = true;

            //_mm_prefetch((char const*)(data_1[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            //_mm_prefetch((char const*)(data_2[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
			
	        ////////////////////////00000: for the first byte./////////////////////////////////
            computeKernelWithMask_FIRST<FIRST_COMPARISON_TYPE>(avx_load( (void *)(data_1[0]+offset+i)),
                                                          mask_literal_1[0],
                                                          m_less_1,
                                                          m_greater_1,
                                                          m_equal_1,
                                                          m_success_1,
                                                          m_fail_1);             		 

            computeKernelWithMask_FIRST<SECOND_COMPARISON_TYPE>( avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2[0],
                                                          m_less_2,
                                                          m_greater_2,
                                                          m_equal_2,
                                                          m_success_2,
                                                          m_fail_2);    			

														  
          //aggregating the information from all the columns.//// 
			  AvxUnit agg_equal;
			  AvxUnit agg_success;
			  AvxUnit agg_fail;		  
              computeForEarlyStop<PREDICATE_TYPE>( m_equal_1,
                                                  m_success_1,
                                                  m_fail_1,
                                                  m_equal_2,
                                                  m_success_2,
                                                  m_fail_2,
                                                  agg_equal,
                                                  agg_success,
                                                  agg_fail); 
		  if (!avx_iszero(agg_equal))
		  {      
            m_equal_1 = avx_and(m_equal_1, agg_equal);
            m_equal_2 = avx_and(m_equal_2, agg_equal);
	    
			if ( (kNumBytesPerCode_1 > 1) && !avx_iszero( m_equal_1) ) //for the second segment of predicate 1.
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

			if ( (kNumBytesPerCode_2 > 1) && !avx_iszero( m_equal_2) ) //for the second segment of predicate 1.
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

		  }
		   
            /////////////combine the result::::generat the vector result for the above computing....//////////////////////////////
            //AvxUnit m_result         = avx_and(m_less_2, m_greater_1);
			   AvxUnit m_result;
			   computeFinalMask<FIRST_COMPARISON_TYPE, SECOND_COMPARISON_TYPE, PREDICATE_TYPE>( m_equal_1,
                                 m_greater_1,
                                 m_less_1,
							     m_equal_2,
                                 m_greater_2,
                                 m_less_2,
								 m_result); 				 

            //move mask from vector register to general purpose register (8-bit --> 1-bit).
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << (i));
        }
        WordUnit x          = bitvector_word;
	    _mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //bitmap[bv_word_id]  = x; //

    }
} 

 
	 
//template <size_t BIT_WIDTH, Direction PDIRECTION> 
//template <Comparator CMP>
void two_columns_cmp_with_literal_P_nS(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2
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
    literal_2 <<= kNumPaddingBits_2;//literal_2 &= ( (1<<(kNumBytesPerCode_2*8 - kNumPaddingBits_2)) - 1);//kCodeMask;
	
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_1; byte_id++){
         ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_1 >> 8*(kNumBytesPerCode_1 - 1 - byte_id)));
         mask_literal_1[byte_id] = avx_set1<ByteUnit>(byte);
    } 
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_2; byte_id++){
		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_2 >> 8*(kNumBytesPerCode_2 - 1 - byte_id)));
         mask_literal_2[byte_id] = avx_set1<ByteUnit>(byte);
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
			AvxUnit tmpA, tmpB;
			
            bool move_to_next_segment = true;

            _mm_prefetch((char const*)(data_1[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_2[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
			
	        ////////////////////////00000: for the first byte./////////////////////////////////
            computeKernelWithMask_FIRST<FIRST_COMPARISON_TYPE>(avx_load( (void *)(data_1[0]+offset+i)),
                                                          mask_literal_1[0],
                                                          m_less_1,
                                                          m_greater_1,
                                                          m_equal_1,
                                                          m_success_1,
                                                          m_fail_1);             		 

            computeKernelWithMask_FIRST<SECOND_COMPARISON_TYPE>( avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2[0],
                                                          m_less_2,
                                                          m_greater_2,
                                                          m_equal_2,
                                                          m_success_2,
                                                          m_fail_2);    			

														  
          //aggregating the information from all the columns.//// 
			  AvxUnit agg_equal;
			  AvxUnit agg_success;
			  AvxUnit agg_fail;		  
              computeForEarlyStop<PREDICATE_TYPE>( m_equal_1,
                                                  m_success_1,
                                                  m_fail_1,
                                                  m_equal_2,
                                                  m_success_2,
                                                  m_fail_2,
                                                  agg_equal,
                                                  agg_success,
                                                  agg_fail); 
		  if (!avx_iszero(agg_equal))
		  {      
            m_equal_1 = avx_and(m_equal_1, agg_equal);
            m_equal_2 = avx_and(m_equal_2, agg_equal);
	   
			if ( (kNumBytesPerCode_1 > 1) && !avx_iszero( m_equal_1) ) //for the second segment of predicate 1.
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

			if ( (kNumBytesPerCode_2 > 1) && !avx_iszero( m_equal_2) ) //for the second segment of predicate 1.
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

		  }
		   
            /////////////combine the result::::generat the vector result for the above computing....//////////////////////////////
            //AvxUnit m_result         = avx_and(m_less_2, m_greater_1);
			   AvxUnit m_result;
			   computeFinalMask<FIRST_COMPARISON_TYPE, SECOND_COMPARISON_TYPE, PREDICATE_TYPE>( m_equal_1,
                                 m_greater_1,
                                 m_less_1,
							     m_equal_2,
                                 m_greater_2,
                                 m_less_2,
								 m_result); 				 

            //move mask from vector register to general purpose register (8-bit --> 1-bit).
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << (i));
        }
        WordUnit x          = bitvector_word;
	    bitmap[bv_word_id]  = x; //_mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //

    }
} 

 
	 
//template <size_t BIT_WIDTH, Direction PDIRECTION> 
//template <Comparator CMP>
void two_columns_cmp_with_literal_P_S(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2
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
    literal_2 <<= kNumPaddingBits_2;//literal_2 &= ( (1<<(kNumBytesPerCode_2*8 - kNumPaddingBits_2)) - 1);//kCodeMask;
	
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_1; byte_id++){
         ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_1 >> 8*(kNumBytesPerCode_1 - 1 - byte_id)));
         mask_literal_1[byte_id] = avx_set1<ByteUnit>(byte);
    } 
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_2; byte_id++){
		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_2 >> 8*(kNumBytesPerCode_2 - 1 - byte_id)));
         mask_literal_2[byte_id] = avx_set1<ByteUnit>(byte);
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
			AvxUnit tmpA, tmpB;
			
            bool move_to_next_segment = true;

            _mm_prefetch((char const*)(data_1[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_2[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
			
	        ////////////////////////00000: for the first byte./////////////////////////////////
            computeKernelWithMask_FIRST<FIRST_COMPARISON_TYPE>(avx_load( (void *)(data_1[0]+offset+i)),
                                                          mask_literal_1[0],
                                                          m_less_1,
                                                          m_greater_1,
                                                          m_equal_1,
                                                          m_success_1,
                                                          m_fail_1);             		 

            computeKernelWithMask_FIRST<SECOND_COMPARISON_TYPE>( avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_2[0],
                                                          m_less_2,
                                                          m_greater_2,
                                                          m_equal_2,
                                                          m_success_2,
                                                          m_fail_2);    			

														  
          //aggregating the information from all the columns.//// 
			  AvxUnit agg_equal;
			  AvxUnit agg_success;
			  AvxUnit agg_fail;		  
              computeForEarlyStop<PREDICATE_TYPE>( m_equal_1,
                                                  m_success_1,
                                                  m_fail_1,
                                                  m_equal_2,
                                                  m_success_2,
                                                  m_fail_2,
                                                  agg_equal,
                                                  agg_success,
                                                  agg_fail); 
		  if (!avx_iszero(agg_equal))
		  {      
            m_equal_1 = avx_and(m_equal_1, agg_equal);
            m_equal_2 = avx_and(m_equal_2, agg_equal);
	  
			if ( (kNumBytesPerCode_1 > 1) && !avx_iszero( m_equal_1) ) //for the second segment of predicate 1.
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

			if ( (kNumBytesPerCode_2 > 1) && !avx_iszero( m_equal_2) ) //for the second segment of predicate 1.
			{
				ByteUnit *addr_tmp   = data_2[1] + offset + i;
					
                tmpA                 =  avx_load( (void *)addr_tmp ); //_mm256_loadu_si256( reinterpret_cast<__m256i*>(addr_tmp) ); //data_1[1]+offset+i
                tmpB                 =  mask_literal_2[1];
                computeKernelWithMask<FIRST_COMPARISON_TYPE>(tmpA,
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
                    computeKernelWithMask<FIRST_COMPARISON_TYPE>(tmpA,
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
                        computeKernelWithMask<FIRST_COMPARISON_TYPE>(tmpA,
                                                                     tmpB,
                                                                     m_less_2,
                                                                     m_greater_2,
                                                                     m_equal_2,
                                                                     m_success_2,
                                                                     m_fail_2);
                    }			
                }						   
			}

		  }
		   
            /////////////combine the result::::generat the vector result for the above computing....//////////////////////////////
            //AvxUnit m_result         = avx_and(m_less_2, m_greater_1);
			   AvxUnit m_result;
			   computeFinalMask<FIRST_COMPARISON_TYPE, SECOND_COMPARISON_TYPE, PREDICATE_TYPE>( m_equal_1,
                                 m_greater_1,
                                 m_less_1,
							     m_equal_2,
                                 m_greater_2,
                                 m_less_2,
								 m_result); 				 

            //move mask from vector register to general purpose register (8-bit --> 1-bit).
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << (i));
        }
        WordUnit x          = bitvector_word;
	    _mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //bitmap[bv_word_id]  = x; //

    }
} 

 
 

 	 