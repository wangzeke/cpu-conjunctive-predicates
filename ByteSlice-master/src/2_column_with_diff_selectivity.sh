#!/bin/bash
#echo $1    
echo $1    #output file name.....

for i in 8 #16 5 5 7  1 4
do
  echo ""  >>$1
  echo "With threads:" $i >>$1
        echo "best case:" >>$1	
				
	    for j in 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001 0.0005 0.0001
		do
		  echo "selectivity of p2:" $j >>$1
          ./2_byteslice_column_block_test.x $i $j 0.7 >result.txt
          #cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "BytesReadFromMC"  >>$1
          cat result.txt |grep "BytesWrittenToMC" >>$1 
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          rm result.txt
		done  

        echo "worst_case:" >>$1		
	    for j in 0.6 0.5 0.4 0.3 0.2 0.1 0.05 0.01 0.005 0.001 0.0005 0.0001
		do
		  echo "selectivity of p2:" $j >>$1
          ./2_byteslice_column_block_test.x $i 0.7 $j >result.txt
          #cat result.txt |grep "p_s_model"  >>$1
          cat result.txt |grep "BytesReadFromMC"  >>$1
          cat result.txt |grep "BytesWrittenToMC" >>$1 
          cat result.txt |grep "codes_per_ns" >>$1
          cat result.txt |grep "real:" >>$1
          rm result.txt
		done 		
done	   
