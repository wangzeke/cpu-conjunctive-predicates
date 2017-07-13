#!/bin/bash
#output the information to Q3_best_worst_result.txt

echo "Testing Q1 under ByteSlice... "    
				
 for j in 0 1
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q1_best_worst_result.txt
   ./Q1.x 1 $j >>Q1_best_worst_result.txt
 done  

    