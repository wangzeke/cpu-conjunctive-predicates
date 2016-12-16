#!/bin/bash

  for j in 0 1 2
  do
   echo "Model (0_MMX, 1_AVX, 2_AVX2):" $j >>impact_of_simd_result.txt
   for k in 1 2 3 4 5 6 7 8 16 
   do
    ./simd_impact.x $k $j >result.txt
     cat result.txt |grep "BytesReadFromMC"  >>impact_of_simd_result.txt
     cat result.txt |grep "BytesWrittenToMC" >>impact_of_simd_result.txt 
     cat result.txt |grep "codes_per_ns" >>impact_of_simd_result.txt
     rm result.txt  
   done
  done
