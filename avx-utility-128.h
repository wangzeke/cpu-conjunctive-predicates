/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: define the SIMD assemable with AVX (m128i). 
 *
 * See file LICENSE.md for details.
 *******************************************************************************/
#ifndef AVX_UTILITY_128_H
#define AVX_UTILITY_128_H

#include    "types_simd.h"
#include    <cstdint>
#include    <x86intrin.h>

// Load
#ifdef STREAM_LOAD_ENABLE
inline __m128i avx_load(void *mem_info){
    return _mm_stream_load_si128( (__m128i*)mem_info );
}
#else
inline __m128i avx_load(void *mem_info){
    return _mm_load_si128( (__m128i*)mem_info );
}
#endif



// Compare less
template <typename T>
inline __m128i avx_cmplt(const __m128i &a, const __m128i &b){
    switch(sizeof(T)){
        case 1:
            return _mm_cmpgt_epi8(b, a);
        case 2:
            return _mm_cmpgt_epi16(b, a);
        case 4:
            return _mm_cmpgt_epi32(b, a); 
        case 8:
            return _mm_cmpgt_epi64(b, a);
    }
}
// Compare greater
template <typename T>
inline __m128i avx_cmpgt(const __m128i &a, const __m128i &b){
    switch(sizeof(T)){
        case 1:
		    
            return _mm_cmpgt_epi8(a, b);
        case 2:
            return _mm_cmpgt_epi16(a, b);
        case 4:
            return _mm_cmpgt_epi32(a, b);
        case 8:
            return _mm_cmpgt_epi64(a, b);
    }
}

// Compare equal
template <typename T>
inline __m128i avx_cmpeq(const __m128i &a, const __m128i &b){
    switch(sizeof(T)){
        case 1:
            return _mm_cmpeq_epi8(b, a);
        case 2:
            return _mm_cmpeq_epi16(b, a);
        case 4:
            return _mm_cmpeq_epi32(b, a);
        case 8:
            return _mm_cmpeq_epi64(b, a);
    }
}

// Set1
template <typename T>
inline __m128i avx_set1(T a){
    switch(sizeof(T)){
        case 1:
            return _mm_set1_epi8(static_cast<int8_t>(a));
        case 2:
            return _mm_set1_epi16(static_cast<int16_t>(a));
        case 4:
            return _mm_set1_epi32(static_cast<int32_t>(a));
        case 8:
            return _mm_set1_epi64x(static_cast<int64_t>(a));
    }
}

// Zero
inline __m128i avx_zero(){
    return _mm_setzero_si128();
}

// All ones
inline __m128i avx_ones(){
    return _mm_set1_epi64x(-1ULL);
}

// Bitwise AND
inline __m128i avx_and(const __m128i &a, const __m128i &b){
    return _mm_and_si128(a, b);
}

// Bitwise OR
inline __m128i avx_or(const __m128i &a, const __m128i &b){
    return _mm_or_si128(a, b);
}

// Bitwise XOR
inline __m128i avx_xor(const __m128i &a, const __m128i &b){
    return _mm_xor_si128(a, b);
}

// Bitwise NOT
inline __m128i avx_not(const __m128i &a){
    return _mm_xor_si128(a, avx_ones());
}

// Bitwise (NOT a) AND b
inline __m128i avx_andnot(const __m128i &a, const __m128i &b){
    return _mm_andnot_si128(a, b);
}

// Test is zero
inline bool avx_iszero(const __m128i &a){
    return _mm_testz_si128(a, a);
}


#if 0
//Scan Kernel2 --- Optimized on Scan Kernel
//to remove unnecessary equal comparison for last byte slice
//add mask_from_column: 1 means that one code is right for early stop.  
template <Comparator CMP, bool LAST_BYTE>
inline void computeKernel(const AvxUnit128 &byteslice1,
                          const AvxUnit128 &byteslice2,
                          AvxUnit128 &mask_less,
                          AvxUnit128 &mask_greater,
                          AvxUnit128 &mask_equal) 
{
    //internal ByteSlice --- not last BS                                                        
    if(!LAST_BYTE){ 
        switch(CMP){
            case Comparator::kEqual:
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;	
            case Comparator::kInequal:
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLess:
            case Comparator::kLessEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kGreater:
            case Comparator::kGreaterEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
        }
    }
    //it is the last byte..... last BS: no need to compute mask_equal for some comparisons
    else //if(BYTE_ID == kNumBytesPerCode - 1)   
    {
		switch(CMP){
            case Comparator::kEqual:
            case Comparator::kInequal:
                mask_equal   = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLessEqual:
                mask_less    = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_equal   = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kLess:
                mask_less    = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                break;
            case Comparator::kGreaterEqual:
                mask_greater = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));
                mask_equal   = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                break;
            case Comparator::kGreater:
                mask_greater = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));
                break;
        }
    }
}



//Scan Kernel2 --- Optimized on Scan Kernel
//to remove unnecessary equal comparison for last byte slice
//add mask_from_column: 0 means that one code is right for early stop.  
template <Comparator CMP>
inline void computeKernelWithMask(const AvxUnit128 &byteslice1,
                          const AvxUnit128 &byteslice2,
                          AvxUnit128 &mask_less,
                          AvxUnit128 &mask_greater,
                          AvxUnit128 &mask_equal,
                          AvxUnit128 &mask_from_column) 
{
        //internal ByteSlice --- not last BS                                                        
        switch(CMP){
            case Comparator::kEqual:
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
				mask_from_column = mask_equal;
                break;	
            case Comparator::kInequal:
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                mask_from_column = avx_not(mask_equal);	
                break;
            case Comparator::kLess:
            case Comparator::kLessEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                mask_from_column = avx_not(mask_greater);	
                break;
            case Comparator::kGreater:
            case Comparator::kGreaterEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                mask_from_column = avx_not(mask_less);	
                break;
        }
}
#endif


 
template <Comparator CMP>
inline void computeKernelWithMask(const AvxUnit128 &byteslice1,
                                  const AvxUnit128 &byteslice2,
                                        AvxUnit128 &mask_less,
                                        AvxUnit128 &mask_greater,
                                        AvxUnit128 &mask_equal,
                                        AvxUnit128 &mask_success,
						                AvxUnit128 &mask_fail) 
{
//	if (!avx_iszero(mask_equal))
        switch(CMP){
            case Comparator::kEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
				mask_fail        = avx_or(mask_less, mask_greater); //avx_not(mask_equal); //
                break;	
            case Comparator::kInequal:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
				mask_success     = avx_or(mask_less, mask_greater);	//avx_not(mask_equal); //		 	
                break;
            case Comparator::kLess:
            case Comparator::kLessEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                mask_fail        = mask_greater;
				mask_success     = mask_less;	
                break;
            case Comparator::kGreater:
            case Comparator::kGreaterEqual:
                mask_less        = avx_or(mask_less, avx_and(mask_equal, avx_cmplt<ByteUnit>(byteslice1, byteslice2)));
                mask_greater     = avx_or(mask_greater, avx_and(mask_equal, avx_cmpgt<ByteUnit>(byteslice1, byteslice2)));				
                mask_equal       = avx_and(mask_equal, avx_cmpeq<ByteUnit>(byteslice1, byteslice2));
                mask_fail        = mask_less;
				mask_success     = mask_greater;	
                break;
        }
} 

template <Bitwise OP>
inline void computeForEarlyStop(const AvxUnit128 &input_equal_1,
                                const AvxUnit128 &input_success_1,
                                const AvxUnit128 &input_fail_1,
							    const AvxUnit128 &input_equal_2,
                                const AvxUnit128 &input_success_2,
                                const AvxUnit128 &input_fail_2,
								      AvxUnit128 &output_equal,
                                      AvxUnit128 &output_success,
                                      AvxUnit128 &output_fail) 
{
        switch(OP){
            case Bitwise::kAnd:
                output_equal       = avx_and( avx_not(avx_or(input_fail_1, input_fail_2)), avx_or(input_equal_1, input_equal_2));
				output_success     = avx_and( input_success_1, input_success_2);
				output_fail        = avx_or ( input_fail_1, input_fail_2);
                break;	
            case Bitwise::kOr:
                output_equal       = avx_and( avx_not(avx_or(input_success_1, input_success_2)), avx_or(input_equal_1, input_equal_2));
				output_success     = avx_or ( input_success_1, input_success_2);
				output_fail        = avx_and( input_fail_1, input_fail_2);
                break;
        }
}

template <Comparator CMP1, Comparator CMP2, Bitwise OP>
   inline void computeFinalMask(const AvxUnit128 &input_equal_1,
                                const AvxUnit128 &input_greater_1,
                                const AvxUnit128 &input_less_1,
							    const AvxUnit128 &input_equal_2,
                                const AvxUnit128 &input_greater_2,
                                const AvxUnit128 &input_less_2,
								      AvxUnit128 &output_mmask) 
{
	AvxUnit128 output_mmask_1, output_mmask_2;
    switch(CMP1){
        case Comparator::kEqual:
            output_mmask_1       = input_equal_1;
            break;	
        case Comparator::kInequal:
            output_mmask_1       = avx_not(input_equal_1);
            break;
        case Comparator::kLess:
            output_mmask_1       = input_less_1;
            break;
        case Comparator::kLessEqual:
            output_mmask_1       = avx_or(input_less_1, input_equal_1);
            break;
        case Comparator::kGreater:
            output_mmask_1       = input_greater_1;
            break;
		case Comparator::kGreaterEqual:
             output_mmask_1       = avx_or(input_greater_1, input_equal_1);
            break;
    }	
    switch(CMP2){
        case Comparator::kEqual:
            output_mmask_2       = input_equal_2;
            break;	
        case Comparator::kInequal:
            output_mmask_2       = avx_not(input_equal_2);
            break;
        case Comparator::kLess:
            output_mmask_2       = input_less_2;
            break;
        case Comparator::kLessEqual:
            output_mmask_2       = avx_or(input_less_2, input_equal_2);
            break;
        case Comparator::kGreater:
            output_mmask_2       = input_greater_2;
            break;
		case Comparator::kGreaterEqual:
             output_mmask_2       = avx_or(input_greater_2, input_equal_2);
            break;
    }		
    switch(OP){
        case Bitwise::kAnd:
            output_mmask       = avx_and( output_mmask_1, output_mmask_2);
            break;	
        case Bitwise::kOr:
            output_mmask       = avx_or( output_mmask_1, output_mmask_2);
            break;
    }
}



#endif  //AVX_UTILITY_128_H
