#!/bin/bash
#echo $1    
echo $1    #output file name.....

for i in 1 #5 5 7   1 4 8 16 
do
  echo ""  >>$1 
  echo "With threads:" $i >>$1
	    for j in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
		do
		  echo "bitwidth:" $j >>$1
          ./2_column_compare_with_literal_test.x $i 1 $j $j 0 1 1 0.5 0.01 >result.txt
          cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          echo ""  >>$1	
          rm result.txt
		done  
		
		 
     done	   
