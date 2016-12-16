#include	<cassert>
#include    <cstdlib>
#include    <cstring>
#include <assert.h> 
#include <stdlib.h> 
#include <sys/mman.h> 
 
#include "types_simd.h" 
#include "avx-utility.h"

#define DEBUG_EN

void print128_num(__m128i var)
{
    uint8_t *val = (uint8_t*) &var;
    printf("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", 
           val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7],
           val[8+0], val[8+1], val[8+2], val[8+3], val[8+4], val[8+5], val[8+6], val[8+7]
		   );
}

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


	 
//template <size_t BIT_WIDTH, Direction PDIRECTION> 
//template <Comparator CMP>
void column_cmp_with_literal_nP_nS(ByteUnit** data_1, uint32_t literal, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode, size_t kNumPaddingBits) 
{
    size_t kNumWordBits = 64;
	size_t kNumAvxBits  = 256;

#ifdef AVX2_DEBUG_ENABLE
    uint64_t counter[4] = {0,0,0,0};
#endif	
    //Prepare byte-slices of literal
    AvxUnit mask_literal[MAX_BYTES_PER_CODE];
    literal &= ( (1<<(kNumBytesPerCode*8 - kNumPaddingBits)) - 1);//kCodeMask;
	
    //padding the literal to byte boundary.. 
	literal <<= kNumPaddingBits;

    for(size_t byte_id=0; byte_id < kNumBytesPerCode; byte_id++){
         ByteUnit byte         = FLIP(static_cast<ByteUnit>(literal >> 8*(kNumBytesPerCode - 1 - byte_id)));
         mask_literal[byte_id] = avx_set1<ByteUnit>(byte);
    } 
	
    //for every kNumWordBits (64) tuples
    for(size_t offset = 0, bv_word_id = 0; offset < len; offset += kNumWordBits, bv_word_id++){
        WordUnit bitvector_word = WordUnit(0);  
        //need several iteration of AVX scan
        for(size_t i=0; i < kNumWordBits; i += kNumAvxBits/8){ //kNumWordBits/4
            AvxUnit m_less    = avx_zero();
            AvxUnit m_greater = avx_zero();
            AvxUnit m_equal; 
            int input_mask;

               // __builtin_prefetch(data_1[0] + offset + i + 1024);
				
                AvxUnit byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[0]+offset+i));
                AvxUnit byteslice2 = mask_literal[0];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[0]+offset+i));
				m_less             =  avx_cmplt<ByteUnit>(byteslice1, byteslice2);
				m_equal            =  avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
#ifdef AVX2_DEBUG_ENABLE
                counter[0]++;
#endif	
			  
                if(kNumBytesPerCode > 1 && (!avx_iszero(m_equal) ) ){

                     byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[1]+offset+i));
                     byteslice2 = mask_literal[1];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[1]+offset+i));

                     m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                     m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
#ifdef AVX2_DEBUG_ENABLE
                     counter[1]++;
#endif							
                     if(kNumBytesPerCode > 2 && (!avx_iszero(m_equal) ) ){

                        byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[2]+offset+i));
                        byteslice2 = mask_literal[2];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[2]+offset+i));

                        m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                        m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
#ifdef AVX2_DEBUG_ENABLE
                        counter[2]++;
#endif								
                        if(kNumBytesPerCode > 3 && (!avx_iszero(m_equal) ) ){

                             byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[3]+offset+i));
                             byteslice2 = mask_literal[3];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[3]+offset+i));

                             m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
#ifdef AVX2_DEBUG_ENABLE
                             counter[3]++;
#endif
                        }
                     }
                }

            AvxUnit m_result;
			m_result          = m_less;
/*			
            switch(CMP){
                case Comparator::kLessEqual:
                    m_result = avx_or(m_less, m_equal);
                    break;
                case Comparator::kLess:
                    m_result = m_less;
                    break;
                case Comparator::kGreaterEqual:
                    m_result = avx_or(m_greater, m_equal);
                    break;
                case Comparator::kGreater:
                    m_result = m_greater;
                    break;
                case Comparator::kEqual:
                    m_result = m_equal;
                    break;
                case Comparator::kInequal:
                    m_result = avx_not(m_equal);
                    break;
                     }
*/					 

            //move mask
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << i);
        }
        WordUnit x          = bitvector_word;
	    bitmap[bv_word_id]  = x; //_mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //

    }
    //bvblock->ClearTail();
#ifdef AVX2_DEBUG_ENABLE	
	printf("%d_%d_%d_%d\n", counter[0], counter[1], counter[2], counter[3]);
#endif	 
}


void column_cmp_with_literal_nP_S(ByteUnit** data_1, uint32_t literal, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode, size_t kNumPaddingBits) 
{
    size_t kNumWordBits = 64;
	size_t kNumAvxBits  = 256;

#ifdef AVX2_DEBUG_ENABLE
    uint64_t counter[4] = {0,0,0,0};
#endif	
    //Prepare byte-slices of literal
    AvxUnit mask_literal[MAX_BYTES_PER_CODE];
    literal &= ( (1<<(kNumBytesPerCode*8 - kNumPaddingBits)) - 1);//kCodeMask;
	
    //padding the literal to byte boundary.. 
	literal <<= kNumPaddingBits;

    for(size_t byte_id=0; byte_id < kNumBytesPerCode; byte_id++){
         ByteUnit byte         = FLIP(static_cast<ByteUnit>(literal >> 8*(kNumBytesPerCode - 1 - byte_id)));
         mask_literal[byte_id] = avx_set1<ByteUnit>(byte);
    } 
	
    //for every kNumWordBits (64) tuples
    for(size_t offset = 0, bv_word_id = 0; offset < len; offset += kNumWordBits, bv_word_id++){
        WordUnit bitvector_word = WordUnit(0);  
        //need several iteration of AVX scan
        for(size_t i=0; i < kNumWordBits; i += kNumAvxBits/8){ //kNumWordBits/4
            AvxUnit m_less    = avx_zero();
            AvxUnit m_greater = avx_zero();
            AvxUnit m_equal; 
            int input_mask;

                //__builtin_prefetch(data_1[0] + offset + i + 1024);
				
                AvxUnit byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[0]+offset+i));
                AvxUnit byteslice2 = mask_literal[0];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[0]+offset+i));
				m_less             =  avx_cmplt<ByteUnit>(byteslice1, byteslice2);
				m_equal            =  avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
#ifdef AVX2_DEBUG_ENABLE
                counter[0]++;
#endif	
			  
                if(kNumBytesPerCode > 1 && (!avx_iszero(m_equal) ) ){

                     byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[1]+offset+i));
                     byteslice2 = mask_literal[1];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[1]+offset+i));

                     m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                     m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
#ifdef AVX2_DEBUG_ENABLE
                     counter[1]++;
#endif							
                     if(kNumBytesPerCode > 2 && (!avx_iszero(m_equal) ) ){

                        byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[2]+offset+i));
                        byteslice2 = mask_literal[2];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[2]+offset+i));

                        m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                        m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
#ifdef AVX2_DEBUG_ENABLE
                        counter[2]++;
#endif								
                        if(kNumBytesPerCode > 3 && (!avx_iszero(m_equal) ) ){

                             byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[3]+offset+i));
                             byteslice2 = mask_literal[3];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[3]+offset+i));

                             m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
#ifdef AVX2_DEBUG_ENABLE
                             counter[3]++;
#endif
                        }
                     }
                }

            AvxUnit m_result;
			m_result          = m_less;
/*			
            switch(CMP){
                case Comparator::kLessEqual:
                    m_result = avx_or(m_less, m_equal);
                    break;
                case Comparator::kLess:
                    m_result = m_less;
                    break;
                case Comparator::kGreaterEqual:
                    m_result = avx_or(m_greater, m_equal);
                    break;
                case Comparator::kGreater:
                    m_result = m_greater;
                    break;
                case Comparator::kEqual:
                    m_result = m_equal;
                    break;
                case Comparator::kInequal:
                    m_result = avx_not(m_equal);
                    break;
                     }
*/					 

            //move mask
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << i);
        }
        WordUnit x          = bitvector_word;
	    _mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //bitmap[bv_word_id]  = x;

    }
    //bvblock->ClearTail();
#ifdef AVX2_DEBUG_ENABLE	
	printf("%d_%d_%d_%d\n", counter[0], counter[1], counter[2], counter[3]);
#endif	 
}

void column_cmp_with_literal_P_nS(ByteUnit** data_1, uint32_t literal, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode, size_t kNumPaddingBits) 
{
    size_t kNumWordBits = 64;
	size_t kNumAvxBits  = 256;

#ifdef AVX2_DEBUG_ENABLE
    uint64_t counter[4] = {0,0,0,0};
#endif	
    //Prepare byte-slices of literal
    AvxUnit mask_literal[MAX_BYTES_PER_CODE];
    literal &= ( (1<<(kNumBytesPerCode*8 - kNumPaddingBits)) - 1);//kCodeMask;
	
    //padding the literal to byte boundary.. 
	literal <<= kNumPaddingBits;

    for(size_t byte_id=0; byte_id < kNumBytesPerCode; byte_id++){
         ByteUnit byte         = FLIP(static_cast<ByteUnit>(literal >> 8*(kNumBytesPerCode - 1 - byte_id)));
         mask_literal[byte_id] = avx_set1<ByteUnit>(byte);
    } 
	
    //for every kNumWordBits (64) tuples
    for(size_t offset = 0, bv_word_id = 0; offset < len; offset += kNumWordBits, bv_word_id++){
        WordUnit bitvector_word = WordUnit(0);  
        //need several iteration of AVX scan
        for(size_t i=0; i < kNumWordBits; i += kNumAvxBits/8){ //kNumWordBits/4
            AvxUnit m_less    = avx_zero();
            AvxUnit m_greater = avx_zero();
            AvxUnit m_equal; 
            int input_mask;

                __builtin_prefetch(data_1[0] + offset + i + 1024);
				
                AvxUnit byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[0]+offset+i));
                AvxUnit byteslice2 = mask_literal[0];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[0]+offset+i));
				m_less             =  avx_cmplt<ByteUnit>(byteslice1, byteslice2);
				m_equal            =  avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
#ifdef AVX2_DEBUG_ENABLE
                counter[0]++;
#endif	
			  
                if(kNumBytesPerCode > 1 && (!avx_iszero(m_equal) ) ){

                     byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[1]+offset+i));
                     byteslice2 = mask_literal[1];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[1]+offset+i));

                     m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                     m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
#ifdef AVX2_DEBUG_ENABLE
                     counter[1]++;
#endif							
                     if(kNumBytesPerCode > 2 && (!avx_iszero(m_equal) ) ){

                        byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[2]+offset+i));
                        byteslice2 = mask_literal[2];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[2]+offset+i));

                        m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                        m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
#ifdef AVX2_DEBUG_ENABLE
                        counter[2]++;
#endif								
                        if(kNumBytesPerCode > 3 && (!avx_iszero(m_equal) ) ){

                             byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[3]+offset+i));
                             byteslice2 = mask_literal[3];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[3]+offset+i));

                             m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
#ifdef AVX2_DEBUG_ENABLE
                             counter[3]++;
#endif
                        }
                     }
                }

            AvxUnit m_result;
			m_result          = m_less;
/*			
            switch(CMP){
                case Comparator::kLessEqual:
                    m_result = avx_or(m_less, m_equal);
                    break;
                case Comparator::kLess:
                    m_result = m_less;
                    break;
                case Comparator::kGreaterEqual:
                    m_result = avx_or(m_greater, m_equal);
                    break;
                case Comparator::kGreater:
                    m_result = m_greater;
                    break;
                case Comparator::kEqual:
                    m_result = m_equal;
                    break;
                case Comparator::kInequal:
                    m_result = avx_not(m_equal);
                    break;
                     }
*/					 

            //move mask
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << i);
        }
        WordUnit x          = bitvector_word;
	    bitmap[bv_word_id]  = x; //_mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //

    }
    //bvblock->ClearTail();
#ifdef AVX2_DEBUG_ENABLE	
	printf("%d_%d_%d_%d\n", counter[0], counter[1], counter[2], counter[3]);
#endif	 
}

void column_cmp_with_literal_P_S(ByteUnit** data_1, uint32_t literal, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode, size_t kNumPaddingBits) 
{
    size_t kNumWordBits = 64;
	size_t kNumAvxBits  = 256;

#ifdef AVX2_DEBUG_ENABLE
    uint64_t counter[4] = {0,0,0,0};
#endif	
    //Prepare byte-slices of literal
    AvxUnit mask_literal[MAX_BYTES_PER_CODE];
    literal &= ( (1<<(kNumBytesPerCode*8 - kNumPaddingBits)) - 1);//kCodeMask;
	
    //padding the literal to byte boundary.. 
	literal <<= kNumPaddingBits;

    for(size_t byte_id=0; byte_id < kNumBytesPerCode; byte_id++){
         ByteUnit byte         = FLIP(static_cast<ByteUnit>(literal >> 8*(kNumBytesPerCode - 1 - byte_id)));
         mask_literal[byte_id] = avx_set1<ByteUnit>(byte);
    } 
	
    //for every kNumWordBits (64) tuples
    for(size_t offset = 0, bv_word_id = 0; offset < len; offset += kNumWordBits, bv_word_id++){
        WordUnit bitvector_word = WordUnit(0);  
        //need several iteration of AVX scan
        for(size_t i=0; i < kNumWordBits; i += kNumAvxBits/8){ //kNumWordBits/4
            AvxUnit m_less    = avx_zero();
            AvxUnit m_greater = avx_zero();
            AvxUnit m_equal; 
            int input_mask;

                __builtin_prefetch(data_1[0] + offset + i + 1024);
				
                AvxUnit byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[0]+offset+i));
                AvxUnit byteslice2 = mask_literal[0];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[0]+offset+i));
				m_less             =  avx_cmplt<ByteUnit>(byteslice1, byteslice2);
				m_equal            =  avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
#ifdef AVX2_DEBUG_ENABLE
                counter[0]++;
#endif	
			  
                if(kNumBytesPerCode > 1 && (!avx_iszero(m_equal) ) ){

                     byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[1]+offset+i));
                     byteslice2 = mask_literal[1];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[1]+offset+i));

                     m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                     m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
#ifdef AVX2_DEBUG_ENABLE
                     counter[1]++;
#endif							
                     if(kNumBytesPerCode > 2 && (!avx_iszero(m_equal) ) ){

                        byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[2]+offset+i));
                        byteslice2 = mask_literal[2];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[2]+offset+i));

                        m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                        m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
#ifdef AVX2_DEBUG_ENABLE
                        counter[2]++;
#endif								
                        if(kNumBytesPerCode > 3 && (!avx_iszero(m_equal) ) ){

                             byteslice1 = _mm256_loadu_si256(reinterpret_cast<__m256i*>(data_1[3]+offset+i));
                             byteslice2 = mask_literal[3];//_mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[3]+offset+i));

                             m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
#ifdef AVX2_DEBUG_ENABLE
                             counter[3]++;
#endif
                        }
                     }
                }

            AvxUnit m_result;
			m_result          = m_less;
/*			
            switch(CMP){
                case Comparator::kLessEqual:
                    m_result = avx_or(m_less, m_equal);
                    break;
                case Comparator::kLess:
                    m_result = m_less;
                    break;
                case Comparator::kGreaterEqual:
                    m_result = avx_or(m_greater, m_equal);
                    break;
                case Comparator::kGreater:
                    m_result = m_greater;
                    break;
                case Comparator::kEqual:
                    m_result = m_equal;
                    break;
                case Comparator::kInequal:
                    m_result = avx_not(m_equal);
                    break;
                     }
*/					 

            //move mask
            uint32_t mmask = _mm256_movemask_epi8(m_result);
		
            //save in temporary bit vector
            bitvector_word |= (static_cast<WordUnit>(mmask) << i);
        }
        WordUnit x          = bitvector_word;
	    _mm_stream_si64((__int64*) &bitmap[bv_word_id], x); //bitmap[bv_word_id]  = x;

    }
    //bvblock->ClearTail();
#ifdef AVX2_DEBUG_ENABLE	
	printf("%d_%d_%d_%d\n", counter[0], counter[1], counter[2], counter[3]);
#endif	 
}

