#####input#####
#run the code with the command:::::make bit_width=BITWIDTH
############### -g

CC=icc
OPT=  -O3 -pthread -fPIC -std=c++11 -fopenmp -lrt 
COMMONTOOL = thread_tool.o  file_tool.o cpu_mapping.o

all: q4_main 

thread_tool.o: thread_tool.cpp avx-utility.h types.h  
	$(CC) $(OPT) -c thread_tool.cpp

file_tool.o: file_tool.cpp   
	$(CC) $(OPT) -c file_tool.cpp

column_compare.o: column_compare.cpp types.h avx-utility.h
	$(CC) $(OPT) -c column_compare.cpp
	
huge_page.o: huge_page.cpp 
	$(CC) $(OPT) -c huge_page.cpp

cpu_mapping.o: cpu_mapping.cpp 
	$(CC) $(OPT) -c cpu_mapping.cpp
	
q4_main: q4_main.cpp avx-utility.h types.h $(COMMONTOOL) common_tool.h column_compare.o
	$(CC) $(OPT) q4_main.cpp -o q4_main $(COMMONTOOL) column_compare.o 

clean_test:
	rm -rf *.o	 
	
clean:
	rm -rf *.o	q4_main 