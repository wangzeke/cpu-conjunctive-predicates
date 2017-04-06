#!/bin/bash
#output the information to Q5_best_worst_result.txt

echo "Testing Q5 under ByteSlice... "    
				
 for j in 0 1 2 3 4 5
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q5_best_worst_result.txt
   ./Q5.x 1 $j >>Q5_best_worst_result.txt
 done  

    
 