/*******************************************************************************
 * Copyright (c) 2017
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: To implement Q6 under fine-grained batch processing of predicates.
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
 
//#define DEBUG_EN

//print256_num_neg(AvxUnit var);
	
#ifdef __INTEL_COMPILER
typedef long si64;
#else
typedef long long si64;
#endif

#include "types_simd.h" 
#include "avx-utility.h" //warning: it is important that we use 128-bit vector for 3 columns.

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


#define FIRST_COMPARISON_TYPE  Comparator::kEqual //
#define SECOND_COMPARISON_TYPE Comparator::kGreaterEqual
#define THIRD_COMPARISON_TYPE  Comparator::kLessEqual //
#define FOURTH_COMPARISON_TYPE Comparator::kEqual    //

#define PREDICATE_TYPE Bitwise::kAnd
	 
	 
void Q8_cmp_with_literal_P_S(WordUnit* bitmap, WordUnit len, 
                                  ByteUnit** data_1, uint32_t literal_1, size_t kNumBytesPerCode_1, size_t kNumPaddingBits_1,
								  ByteUnit** data_2, uint32_t literal_2, size_t kNumBytesPerCode_2, size_t kNumPaddingBits_2,
								  ByteUnit** data_3, uint32_t literal_3, size_t kNumBytesPerCode_3, size_t kNumPaddingBits_3,
								                     uint32_t literal_4
								  ) 
{

#ifdef AVX2_DEBUG_ENABLE
    uint64_t counter[4] = {0,0,0,0};
#endif	
    //Prepare byte-slices of literal
	kNumBytesPerCode_1 = 1;
	kNumBytesPerCode_2 = 2;
	kNumBytesPerCode_3 = 1;
	
    AvxUnit mask_literal_1[kNumBytesPerCode_1];
    literal_1 <<= kNumPaddingBits_1;
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_1; byte_id++){
         ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_1 >> 8*(kNumBytesPerCode_1 - 1 - byte_id)));
         mask_literal_1[byte_id] = avx_set1<ByteUnit>(byte);
    } 

    AvxUnit mask_literal_2[kNumBytesPerCode_2];	
    AvxUnit mask_literal_3[kNumBytesPerCode_2];		
    literal_2 <<= kNumPaddingBits_2;
    literal_3 <<= kNumPaddingBits_2;	
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_2; byte_id++){

		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_2 >> 8*(kNumBytesPerCode_2 - 1 - byte_id)));
         mask_literal_2[byte_id] = avx_set1<ByteUnit>(byte);	
                  byte           = FLIP(static_cast<ByteUnit>(literal_3 >> 8*(kNumBytesPerCode_2 - 1 - byte_id)));
         mask_literal_3[byte_id] = avx_set1<ByteUnit>(byte);			 
    } 



    AvxUnit mask_literal_4[kNumBytesPerCode_3];		
    literal_4 <<= kNumPaddingBits_3;
    for(size_t byte_id=0; byte_id < kNumBytesPerCode_3; byte_id++){
		 ByteUnit byte           = FLIP(static_cast<ByteUnit>(literal_4 >> 8*(kNumBytesPerCode_3 - 1 - byte_id)));
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

            _mm_prefetch((char const*)(data_1[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_2[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
            _mm_prefetch((char const*)(data_3[0] + offset + i + PREFETCHING_DISTANCE), HINT_LEVEL);
			
	        ////////////////////////approximate stage: for the first byte./////////////////////////////////
            computeKernelWithMask_LAST<FIRST_COMPARISON_TYPE>(avx_load( (void *)(data_1[0]+offset+i)),
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
														  
            computeKernelWithMask_FIRST<THIRD_COMPARISON_TYPE>(avx_load( (void *)(data_2[0]+offset+i)),
                                                          mask_literal_3[0],
                                                          m_less_3,
                                                          m_greater_3,
                                                          m_equal_3,
                                                          m_success_3,
                                                          m_fail_3);    			

            computeKernelWithMask_LAST<FOURTH_COMPARISON_TYPE>(avx_load( (void *)(data_3[0]+offset+i)),
                                                          mask_literal_4[0],
                                                          m_less_4,
                                                          m_greater_4,
                                                          m_equal_4,
                                                          m_success_4,
                                                          m_fail_4);   
		
														  
          //aggregating the information from all the columns.//// 
          //aggregating the information from all the columns.//// 
			  AvxUnit agg_equal,   agg_equal_a,   agg_equal_b,   agg_equal_c;
			  AvxUnit agg_success, agg_success_a, agg_success_b, agg_success_c;
			  AvxUnit agg_fail,    agg_fail_a,    agg_fail_b,    agg_fail_c;		  
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
												  
              computeForEarlyStop<PREDICATE_TYPE>(agg_equal_a,
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
              m_equal_2 = avx_and(m_equal_2, agg_equal);
              m_equal_3 = avx_and(m_equal_3, agg_equal);
	         if ( (kNumBytesPerCode_2 > 1) && ((!avx_iszero( m_equal_3)) || (!avx_iszero( m_equal_2))) ) //for the remaining segments of predicate 1.
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
                tmpB                 =  mask_literal_3[1];
                computeKernelWithMask<THIRD_COMPARISON_TYPE>(tmpA,
                                                             tmpB,
                                                             m_less_3,
                                                             m_greater_3,
                                                             m_equal_3,
                                                             m_success_3,
                                                             m_fail_3);

															 
			 }
		  }
		   
            /////////////combine the result::::generat the vector result for the above computing....//////////////////////////////
			   AvxUnit m_result_1, m_result_2, m_result_3, m_result_4, m_result_5, m_result_tmp, m_result_tmp_1, m_result_tmp_2, m_result;

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
							                                m_result_tmp_2);
               computeConjunctivePredicates<PREDICATE_TYPE>(m_result_3,
                                                            m_result_4,
							                                m_result_tmp_1);
               computeConjunctivePredicates<PREDICATE_TYPE>(m_result_tmp_2,
                                                            m_result_tmp_1,
							                                m_result);
		
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
	uint32_t T2_bit_width      = 12;//d->T2_bit_width;
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

        uint32_t literal_1   = 2;    //America
	    uint32_t literal_2   = 1861; //1995-01-01
        uint32_t literal_3   = 2604; //1996-12-31
        uint32_t literal_4   = 4;    //ECONOMY ANODIZED STEEL
		
		
  ///////////////////////Generate the input output data..///////////////////////////////////////	
    uint32_t *original_1, *original_2, *original_3;  // input original data.  
	ByteUnit *data_1[4], *data_2[4], *data_3[4];     //
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
   //consider to use 2M huge table.
   bitvector              = (WordUnit *) malloc_memory(T1_len_aligned/8, huge_table_enable);//use 4K page.	
   if (bitvector == NULL) 
   {
         printf ( "output bitvector_malloc fails\n");
         return NULL;
   }

   
   for (i = 0; i < 4; i++) // malloc memory space for two codes. 
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
   }   
   
   

	  FILE *fp_r_name, *fp_o_orderdate, *fp_p_type;//
	  char str[128];
	  uint32_t tmp;
      float    tmp_f;
  //1....................: output_r1_name
      if((fp_r_name=fopen("../../lineitemWT/output_r1_name.txt","r"))==NULL) {
        printf("cannot open output_r1_name.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_r_name, "%d\n", &tmp) > 0) { 
         original_1[i]       = tmp; //tmp_cons; //
         SetTuple(data_1,  i, tmp,  kNumBytesPerCode_1, kNumPaddingBits_1);			  
	     i++;
    }
    fclose(fp_r_name);
  //2....................: output_o_orderdate.txt
      if((fp_o_orderdate=fopen("../../lineitemWT/output_o_orderdate.txt","r"))==NULL) {
        printf("cannot open output_o_orderdate.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_o_orderdate, "%d\n", &tmp) > 0) { 
         original_2[i]       = tmp; //tmp_cons; //
         SetTuple(data_2,  i, tmp,  kNumBytesPerCode_2, kNumPaddingBits_2);				  
	     i++;
    }
    fclose(fp_o_orderdate);
  //3....................: output_p_type.txt
      if((fp_p_type=fopen("../../lineitemWT/output_p_type.txt","r"))==NULL) {
        printf("cannot open output_p_type.txt/n");
        exit(1);
      }
	  i = 0;
      while(fscanf(fp_p_type, "%d\n", &tmp) > 0) { 
         original_3[i]       = tmp; //tmp_cons; //
         SetTuple(data_3,  i, tmp,  kNumBytesPerCode_3, kNumPaddingBits_3);			  
	     i++;
    }
    fclose(fp_p_type);




  //  printf("after set data_.\n");  
	
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
		
                    Q8_cmp_with_literal_P_S(bitvector, T1_len, 
                                            data_1, literal_1, kNumBytesPerCode_1, kNumPaddingBits_1,
								            data_2, literal_2, kNumBytesPerCode_2, kNumPaddingBits_2,
                                            data_3, literal_3, kNumBytesPerCode_3, kNumPaddingBits_3,
											        literal_4
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
/*            bool real  = ( (original_1[ii] >= literal_1)&&(original_1[ii] <  literal_2) && 
			               (original_2[ii] >= literal_3)&&(original_2[ii] <= literal_4) && 
						   (original_3[ii] <  literal_5));  */
			bool real  = ((original_1[ii] == literal_1)&&
			              (original_2[ii] >= literal_2)&&
						  (original_2[ii] <= literal_3)&&
						  (original_3[ii] == literal_4)
						  ); 						   
	
/*              bool real  = ( (original_1[ii] > literal_1)&&(original_1[ii] <  literal_2) && 
			               (original_2[ii] > literal_3)&&(original_2[ii] < literal_4) && 
						   (original_3[ii] <  literal_5)); */					   
			bool eval  = GetBit(bitvector, ii); //bvblock->GetBit(ii); 
            if ( real !=  eval )
			{
              printf("thread_%d::index_%d:  eval: %d, real: %d \n", d->thread, ii, eval, real);
			  printf("original_1[ii] = %x, literal_1 = %x, sign (==) = %d\n", original_1[ii], literal_1, (original_1[ii] == literal_1) );
			  printf("original_2[ii] = %x, literal_2 = %x, sign (>=) = %d\n", original_2[ii], literal_2, (original_2[ii] >= literal_2) );
			  printf("original_2[ii] = %x, literal_3 = %x, sign (<=) = %d\n", original_2[ii], literal_3, (original_2[ii] <= literal_3) );
			  printf("original_3[ii] = %x, literal_4 = %x, sign (>) = %d\n", original_3[ii], literal_4, (original_3[ii] == literal_4) );			  
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
    
  uint64_t tuples          = 60490115;//1000000000; //

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
		
		info[t].T1_len          = task_len[t]; //same for all the threads (64*), except the last one 
		//printf("task_len[%d] = %d\n", t, task_len[t]);
		pthread_create(&info[t].id, &attr, run, (void*) &info[t]); //&info[t].id, NULL
	}
	
   //finish the execution of all threads......	
	for (t = 0 ; t != thread_num ; ++t)
		pthread_join(info[t].id, NULL);
	
	
	
	for (b = 0 ; b != barrier_num; ++b)
		pthread_barrier_destroy(&barrier[b]);
	
   return;//EXIT_SUCCESS  

}



