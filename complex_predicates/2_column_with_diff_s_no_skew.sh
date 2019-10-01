#!/bin/bash
#echo $1    
echo $1    #output file name.....

for i in 1 #5 5 7   1 4 8 16 
do
  echo ""  >>$1 
  echo "With threads:" $i >>$1
	    for j in 0.05 0.04 0.03 0.004 0.0003
		do
		  echo "1, selectivity of p2:" $j >>$1
          	./2_column_compare_with_literal_test.x $i 1 17 17 0 1 1 0.5 $j >result.txt
          cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          echo ""  >>$1	
          rm result.txt

		  echo "2, selectivity of p2:" $j >>$1
          ./2_column_compare_with_literal_test.x $i 1 17 17 0 1 1 0.5 $j >result.txt
          cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          echo ""  >>$1	
          rm result.txt

		  echo "3, selectivity of p2:" $j >>$1
          ./2_column_compare_with_literal_test.x $i 1 17 17 0 1 1 0.5 $j >result.txt
          cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          echo ""  >>$1	
          rm result.txt

		  echo "4, selectivity of p2:" $j >>$1
          ./2_column_compare_with_literal_test.x $i 1 17 17 0 1 1 0.5 $j >result.txt
          cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          echo ""  >>$1	
          rm result.txt

		  echo "5, selectivity of p2:" $j >>$1
          ./2_column_compare_with_literal_test.x $i 1 17 17 0 1 1 0.5 $j >result.txt
          cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          echo ""  >>$1	
          rm result.txt


		done  
		
		 
     done	   
