#####input#####
#run the code with the command:::::make bit_width=BITWIDTH
############### #-DBIT_WIDTH_BYTESLICE=$(bit_width)   -DBIT_WIDTH_BYTESLICE=$(bit_width)     byteslice_column_block_test.x

CC=icc
OPT= -g -O3 -pthread -fPIC -std=c++11 -fopenmp -lrt 

all: Q19.x Q17.x Q10.x Q14.x Q15.x Q1.x Q12.x Q8.x Q7.x Q6.x Q5.x Q3.x 4_byteslice_column_block_test.x 3_byteslice_column_block_test.x 2_byteslice_column_block_test.x byteslice_column_block_test.x byteslice_column_block.o bitvector.o bitvector_block.o bitvector_iterator.o naive_column_block.o sequential_binary_file.o
#

byteslice_column_block.o: byteslice_column_block.h byteslice_column_block.cpp avx-utility.h column_block.h  
	$(CC) $(OPT) -c byteslice_column_block.cpp 

bitvector.o: bitvector.h bitvector.cpp  
	$(CC) $(OPT) -c bitvector.cpp

bitvector_block.o: bitvector_block.h bitvector_block.cpp  
	$(CC) $(OPT) -c bitvector_block.cpp

bitvector_iterator.o: bitvector_iterator.h bitvector_iterator.cpp  
	$(CC) $(OPT) -c bitvector_iterator.cpp

naive_column_block.o: naive_column_block.h naive_column_block.cpp  
	$(CC) $(OPT) -c naive_column_block.cpp

sequential_binary_file.o: sequential_binary_file.h sequential_binary_file.cpp  
	$(CC) $(OPT) -c sequential_binary_file.cpp
	
cpu_mapping.o: cpu_mapping.cpp 
	$(CC) $(OPT) -c cpu_mapping.cpp
		
byteslice_column_block_test.x: byteslice_column_block.h byteslice_column_block_test.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) byteslice_column_block_test.cpp -o byteslice_column_block_test.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a

	
2_byteslice_column_block_test.x: byteslice_column_block.h 2_byteslice_column_block_test.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) 2_byteslice_column_block_test.cpp -o 2_byteslice_column_block_test.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a

	
3_byteslice_column_block_test.x: byteslice_column_block.h 3_byteslice_column_block_test.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) 3_byteslice_column_block_test.cpp -o 3_byteslice_column_block_test.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a

4_byteslice_column_block_test.x: byteslice_column_block.h 4_byteslice_column_block_test.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) 4_byteslice_column_block_test.cpp -o 4_byteslice_column_block_test.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a

4_byteslice_column_c_block_test.x: byteslice_column_block.h 4_byteslice_column_c_block_test.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) 4_byteslice_column_c_block_test.cpp -o 4_byteslice_column_c_block_test.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a

	
Q3.x: byteslice_column_block.h Q3.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q3.cpp -o Q3.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a

Q5.x: byteslice_column_block.h Q5.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q5.cpp -o Q5.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
	
Q6.x: byteslice_column_block.h Q6.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q6.cpp -o Q6.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
		
Q7.x: byteslice_column_block.h Q7.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q7.cpp -o Q7.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a

Q8.x: byteslice_column_block.h Q8.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q8.cpp -o Q8.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
	
Q12.x: byteslice_column_block.h Q12.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q12.cpp -o Q12.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a

Q10.x: byteslice_column_block.h Q10.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q10.cpp -o Q10.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
		
Q1.x: byteslice_column_block.h Q1.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q1.cpp -o Q1.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
	
Q14.x: byteslice_column_block.h Q14.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q14.cpp -o Q14.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
	
Q15.x: byteslice_column_block.h Q15.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q15.cpp -o Q15.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
	
Q17.x: byteslice_column_block.h Q17.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q17.cpp -o Q17.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
	
Q19.x: byteslice_column_block.h Q19.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) Q19.cpp -o Q19.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
		
haihang_test.x: byteslice_column_block.h haihang_test.cpp avx-utility.h column_block.h byteslice_column_block.o cpu_mapping.o bitvector.o bitvector_block.o bitvector_iterator.o sequential_binary_file.o  
	$(CC) $(OPT) haihang_test.cpp -o haihang_test.x byteslice_column_block.o bitvector.o bitvector_block.o cpu_mapping.o bitvector_iterator.o sequential_binary_file.o libpcm_2_11.a
	
#msrtest.x: msrtest.cpp msr.o cpucounters.o perf_counters.o pci.o cpucounters.h  msr.h  types.h client_bw.o
#	$(CC) $(OPT) msrtest.cpp -o msrtest.x msr.o cpucounters.o perf_counters.o pci.o client_bw.o

huge_page.o: huge_page.cpp 
	$(CC) $(OPT) -c huge_page.cpp


clean_test:
	rm -rf byteslice_column_block_test.x	 
	
clean:
	rm -rf *.o	