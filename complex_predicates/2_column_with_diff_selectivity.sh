#!/bin/bash
#echo $1    
echo $1    #output file name.....

for i in 1 #5 5 7   1 4 8 16 
do
  echo ""  >>$1 
  echo "With threads:" $i >>$1
	    for j in 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001
		do
		  echo "selectivity of p2:" $j >>$1
          ./2_column_compare_with_literal_test.x $i 1 17 17 0 1 1 0.7 $j >result.txt
          cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "Instructions-retired"  >>$1
          cat result.txt |grep "L3Misses"  >>$1
          cat result.txt |grep "CyclesLostDueL3CacheMisses"  >>$1
          cat result.txt |grep "BytesReadFromMC"  >>$1
          cat result.txt |grep "BytesWrittenToMC" >>$1 
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          echo ""  >>$1	
          rm result.txt
		done  
		
		 
     done	   
