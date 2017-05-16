#include	<cassert>
#include    <cstdlib>
#include    <cstring>
#include <assert.h> 
#include <stdlib.h> 
#include <sys/mman.h> 
 
#include "types.h" 
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

#if 0
template <Comparator CMP, size_t BYTE_ID>
inline void ByteSliceColumnBlock<BIT_WIDTH, PDIRECTION>::ScanKernel2
                                                        (const AvxUnit &byteslice1,
                                                         const AvxUnit &byteslice2,
                                                         AvxUnit &mask_less,
                                                         AvxUnit &mask_greater,
                                                         AvxUnit &mask_equal) const {

    //internal ByteSlice --- not last BS                                                        
    if(BYTE_ID < kNumBytesPerCode - 1){ 
        switch(CMP){
            case Comparator::kEqual:
            case Comparator::kInequal:
                mask_equal = 
                    avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLess:
            case Comparator::kLessEqual:
                mask_less = 
                    avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_equal = 
                    avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kGreater:
            case Comparator::kGreaterEqual:
                mask_greater = //avx_cmpgt<ByteUnit>(byteslice1, byteslice2);
                    avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));
                mask_equal = 
                    avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
        }
    }
    //last BS: no need to compute mask_equal for some comparisons
    else if(BYTE_ID == kNumBytesPerCode - 1){   
        switch(CMP){
            case Comparator::kEqual:
            case Comparator::kInequal:
                mask_equal = 
                    avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLessEqual:
                mask_less = 
                    avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_equal = 
                    avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLess:
                mask_less = 
                    avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                break;
            case Comparator::kGreaterEqual:
                mask_greater =
                    avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));
                mask_equal = 
                    avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kGreater:
                mask_greater =
                    avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));
                break;
        }
    }
    //otherwise, do nothing
}
#endif
 
	 
//template <size_t BIT_WIDTH, Direction PDIRECTION> 
//template <Comparator CMP>
void two_column_cmp(ByteUnit** data_1, ByteUnit** data_2, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode) 
{
    size_t kNumWordBits = 64;
	size_t kNumAvxBits  = 256;
    //for every kNumWordBits (64) tuples
    for(size_t offset = 0, bv_word_id = 0; offset < len; offset += kNumWordBits, bv_word_id++){
        WordUnit bitvector_word = WordUnit(0); 
        //need several iteration of AVX scan
        for(size_t i=0; i < kNumWordBits; i += kNumAvxBits/8){ //kNumWordBits/4
            AvxUnit m_less = avx_zero();
            AvxUnit m_greater = avx_zero();
            AvxUnit m_equal; 
            int input_mask;

//            if((OPT==Bitwise::kSet) ||  0 != input_mask){
                __builtin_prefetch(data_1[0] + offset + i + 1024);
                __builtin_prefetch(data_2[0] + offset + i + 1024);
				
/*				
                ScanKernel2<CMP, 0>(
                        _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_1[0]+offset+i)),
                        _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[0]+offset+i)),
                        m_less,
                        m_greater,
                        m_equal);
*/						
                AvxUnit byteslice1 = _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_1[0]+offset+i));
                AvxUnit byteslice2 = _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[0]+offset+i));
				m_less             =  avx_cmplt<ByteUnit>(byteslice1, byteslice2);
				m_equal            =  avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
				
 #ifdef DEBUG_EN1				
				printf("data offset (byte): 0x%x\n", offset+i);
				printf("data_1[0]:");
				print256_num_neg(byteslice1);//m128i_toString( m128_tmp);
				printf("data_2[0]:");
				print256_num_neg(byteslice2);//m128i_toString( m128_tmp);
				printf("m_less:");				
				print256_num_neg(m_less);//m128i_toString( m128_tmp);
				printf("m_equal:");
				print256_num_neg(m_equal);//m128i_toString( m128_tmp);				
 #endif
			  
                if(kNumBytesPerCode > 1 && (!avx_iszero(m_equal) ) ){
/*					
                    ScanKernel2<CMP, 1>(
                            _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_1[1]+offset+i)),
                            _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[1]+offset+i)),
                            m_less,
                            m_greater,
                            m_equal);
*/
                     byteslice1 = _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_1[1]+offset+i));
                     byteslice2 = _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[1]+offset+i));
/*			    	 m_less     =  avx_cmplt<ByteUnit>(byteslice1, byteslice2);
			    	 m_equal    =  avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
*/
                     m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                     m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
							
                     if(kNumBytesPerCode > 2 && (!avx_iszero(m_equal) ) ){
/*						
                        ScanKernel2<CMP, 2>(
                                _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_1[2]+offset+i)),
                                _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[2]+offset+i)),
                                m_less,
                                m_greater,
                                m_equal);
*/
                        byteslice1 = _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_1[2]+offset+i));
                        byteslice2 = _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[2]+offset+i));
/*                      m_less     =  avx_cmplt<ByteUnit>(byteslice1, byteslice2);
                        m_equal    =  avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
*/
                        m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                        m_equal    =  avx_and(m_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
								
                        if(kNumBytesPerCode > 3 && (!avx_iszero(m_equal) ) ){
/*
						ScanKernel2<CMP, 3>(
                                    _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_1[3]+offset+i)),
                                    _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[3]+offset+i)),
                                    m_less,
                                    m_greater,
                                    m_equal);
*/
                             byteslice1 = _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_1[3]+offset+i));
                             byteslice2 = _mm256_lddqu_si256(reinterpret_cast<__m256i*>(data_2[3]+offset+i));
/*                           m_less     =  avx_cmplt<ByteUnit>(byteslice1, byteslice2);
                             m_equal    =  avx_cmpeq<ByteUnit>(byteslice1, byteslice2);
*/
                             m_less     = avx_or(m_less, avx_and(m_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                        }
                     }
                }
//            }

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
	    bitmap[bv_word_id]  = x;
/*	   
        switch(OPT){
            case Bitwise::kSet:
                break;
            case Bitwise::kAnd:
                x &= bvblock->GetWordUnit(bv_word_id);
                break;
            case Bitwise::kOr:
                x |= bvblock->GetWordUnit(bv_word_id);
                break;
        }
		
        bvblock->SetWordUnit(x, bv_word_id);
*/
    }
    //bvblock->ClearTail();
	
}

