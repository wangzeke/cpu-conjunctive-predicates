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
#ifndef AVX_UTILITY_H
#define AVX_UTILITY_H


#include    <cstdint>
#include    <x86intrin.h>

template <typename T>
inline T FLIP(T value){
    constexpr T offset = (static_cast<T>(1) << (sizeof(T)*8 - 1));
    return static_cast<T>(value ^ offset);
}


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
 
// move mask
inline uint32_t avx_movemask(const __m128i &a){
    return _mm_movemask_epi8(a);
}


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



#endif  //AVX_UTILITY_128_H
