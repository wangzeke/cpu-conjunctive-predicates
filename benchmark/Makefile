#####input#####
#run the code with the command:::::make bit_width=BITWIDTH
############### -g

CC=icc
OPT=  -O3 -pthread -fPIC -fopenmp -std=c++11 -lrt -I../complex_predicates/
#-std=c++0x  -std=gnu++11 
COMMONTOOL = thread_tool.o rand_tool.o file_tool.o cpu_mapping.o huge_page.o memory_tool.o ../libpcm_2_11.a 


all: stpr.x 
 
#####################common functions########################################## ../complex_predicates/
rand_tool.o: ../complex_predicates/rand_tool.cpp   
	$(CC) $(OPT) -c ../complex_predicates/rand_tool.cpp
thread_tool.o: ../complex_predicates/thread_tool.cpp ####avx-utility.h types.h  
	$(CC) $(OPT) -c ../complex_predicates/thread_tool.cpp
file_tool.o: ../complex_predicates/file_tool.cpp   
	$(CC) $(OPT) -c ../complex_predicates/file_tool.cpp
huge_page.o: ../complex_predicates/huge_page.cpp 
	$(CC) $(OPT) -c ../complex_predicates/huge_page.cpp
cpu_mapping.o: ../complex_predicates/cpu_mapping.cpp 
	$(CC) $(OPT) -c ../complex_predicates/cpu_mapping.cpp
memory_tool.o: ../complex_predicates/memory_tool.cpp   
	$(CC) $(OPT) -c ../complex_predicates/memory_tool.cpp
#########################end of common functions################################

# check the impact of hardware prefetcher, branch and PCM. Also the one column case.  common_tool.h avx-utility.h types.h 
stpr.x: stpr.cpp $(COMMONTOOL) 
	$(CC) $(OPT) stpr.cpp -o stpr.x $(COMMONTOOL)
#####################end of motivations##########################################	

clean:
	rm -rf *.o *.x
