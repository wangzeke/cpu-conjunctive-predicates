#!/bin/bash
#output the information to Q14_best_worst_result.txt

echo "Testing Q1 under ByteSlice... "    
				
 for j in 0 1
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q14_best_worst_result.txt
   ./Q14.x 1 $j >>Q14_best_worst_result.txt
 done  

     