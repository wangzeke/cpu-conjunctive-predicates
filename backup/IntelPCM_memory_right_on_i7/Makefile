#
# Copyright (c) 2009-2010 Intel Corporation
# written by Roman Dementiev
#

# Changelog:
#
#  - Added targets for building libraries and modified default target. (cagri)
#

# Now only builds the shared library by default
all: msrtest.x pcm-msr.x libpcm_2_11.a 

CC=g++
OPT= -g -O3 -pthread -fPIC -std=c++0x

msr.o: msr.h msr.cpp 
	$(CC) $(OPT) -c msr.cpp
pci.o: pci.h pci.cpp
	$(CC) $(OPT) -c pci.cpp
cpucounters.o: msr.h msr.cpp cpucounters.h cpucounters.cpp types.h
	$(CC) $(OPT) -c cpucounters.cpp
client_bw.o: msr.h msr.cpp cpucounters.h client_bw.cpp types.h
	$(CC) $(OPT) -c client_bw.cpp
perf_counters.o: perf_counters.h perf_counters.cpp 
	$(CC) $(OPT) -c perf_counters.cpp

####target to generate the lib for other programs to use########	
libpcm_2_11.a: msr.o cpucounters.o perf_counters.o pci.o client_bw.o
	ar r libpcm_2_11.a msr.o cpucounters.o perf_counters.o pci.o client_bw.o
#################################################################	
	
	
msrtest.x: msrtest.cpp msr.o cpucounters.o perf_counters.o pci.o cpucounters.h  msr.h  types.h client_bw.o
	$(CC) $(OPT) msrtest.cpp -o msrtest.x msr.o cpucounters.o perf_counters.o pci.o client_bw.o
pcm-msr.x: pcm-msr.cpp msr.o cpucounters.o perf_counters.o pci.o cpucounters.h  msr.h  types.h client_bw.o
	$(CC) $(OPT) pcm-msr.cpp -o pcm-msr.x msr.o cpucounters.o perf_counters.o pci.o client_bw.o

	
	
clean:
	rm -rf *.x *.o *.a
