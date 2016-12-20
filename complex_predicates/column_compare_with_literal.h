#ifndef COLUMN_COMPARE_WITH_LITERAL_H
#define COLUMN_COMPARE_WITH_LITERAL_H

#include "types_simd.h"
	 
void column_cmp_with_literal_nP_nS(ByteUnit** data_1, uint32_t literal, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode, size_t kNumPaddingBits); 
	 
void column_cmp_with_literal_nP_S(ByteUnit** data_1, uint32_t literal, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode, size_t kNumPaddingBits); 
	 
void column_cmp_with_literal_P_nS(ByteUnit** data_1, uint32_t literal, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode, size_t kNumPaddingBits); 
	 
void column_cmp_with_literal_P_S(ByteUnit** data_1, uint32_t literal, WordUnit* bitmap, WordUnit len, size_t kNumBytesPerCode, size_t kNumPaddingBits); 


#endif