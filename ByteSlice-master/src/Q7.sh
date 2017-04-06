#!/bin/bash
#output the information to Q7_best_worst_result.txt

echo "Testing Q7 under ByteSlice... "    
				
 for j in 2 3  #4 5 0 1
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q7_best_worst_result.txt
   ./Q7.x 1 $j >>Q7_best_worst_result.txt
 done  

    
 