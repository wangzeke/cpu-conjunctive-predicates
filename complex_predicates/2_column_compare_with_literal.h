#ifndef TWO_COLUMN_COMPARE_WITH_LITERAL_H
#define TWO_COLUMN_COMPARE_WITH_LITERAL_H

#include "types_simd.h"


void two_columns_cmp_with_literal_nP_nS(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2
								  ); 

void two_columns_cmp_with_literal_nP_S(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2
								  ); 
void two_columns_cmp_with_literal_P_nS(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2
								  ); 
void two_columns_cmp_with_literal_P_S(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2
								  ); 								  
#endif