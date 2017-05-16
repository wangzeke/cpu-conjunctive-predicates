#####input#####
#run the code with the command:::::make bit_width=BITWIDTH
############### -g

CC=icc
OPT=  -O3 -pthread -fPIC -std=c++11 -fopenmp -lrt 
COMMONTOOL = thread_tool.o  file_tool.o cpu_mapping.o perf_counters.o libpcm_2_11.a


all: simd_impact

thread_tool.o: thread_tool.cpp avx-utility.h types.h  
	$(CC) $(OPT) -c thread_tool.cpp

file_tool.o: file_tool.cpp   
	$(CC) $(OPT) -c file_tool.cpp

huge_page.o: huge_page.cpp 
	$(CC) $(OPT) -c huge_page.cpp

cpu_mapping.o: cpu_mapping.cpp 
	$(CC) $(OPT) -c cpu_mapping.cpp
	
perf_counters.o: perf_counters.cpp   
	$(CC) $(OPT) -c perf_counters.cpp 
	

simd_impact: simd_impact.cpp avx-utility.h types.h $(COMMONTOOL) common_tool.h 
	$(CC) $(OPT) simd_impact.cpp -o simd_impact $(COMMONTOOL) 
	
clean_test:
	rm -rf *.o	 
	
clean:
	rm -rf *.o	simd_impact 