#ifndef COLUMN_COMPARE_WITH_LITERAL_H
#define COLUMN_COMPARE_WITH_LITERAL_H

#include "types_simd.h"
								  
void five_columns_cmp_with_literal_nP_nS(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2,
								  ByteUnit** data_3, uint32_t literal_3, size_t kNumBytesPerCode_3, size_t kNumPaddingBits_3,
								  ByteUnit** data_4, uint32_t literal_4, size_t kNumBytesPerCode_4, size_t kNumPaddingBits_4,
								  ByteUnit** data_5, uint32_t literal_5, size_t kNumBytesPerCode_5, size_t kNumPaddingBits_5
								  ); 		

void four_columns_cmp_with_literal_nP_S(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2,
								  ByteUnit** data_3, uint32_t literal_3, size_t kNumBytesPerCode_3, size_t kNumPaddingBits_3,
								  ByteUnit** data_4, uint32_t literal_4, size_t kNumBytesPerCode_4, size_t kNumPaddingBits_4
								  ); 		

void four_columns_cmp_with_literal_P_nS(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2,
								  ByteUnit** data_3, uint32_t literal_3, size_t kNumBytesPerCode_3, size_t kNumPaddingBits_3,
								  ByteUnit** data_4, uint32_t literal_4, size_t kNumBytesPerCode_4, size_t kNumPaddingBits_4
								  ); 		

void four_columns_cmp_with_literal_P_S(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2,
								  ByteUnit** data_3, uint32_t literal_3, size_t kNumBytesPerCode_3, size_t kNumPaddingBits_3,
								  ByteUnit** data_4, uint32_t literal_4, size_t kNumBytesPerCode_4, size_t kNumPaddingBits_4
								  ); 		

								  
	
#endif