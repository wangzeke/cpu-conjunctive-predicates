#!/bin/bash
#output the information to Q7_best_worst_result.txt

echo "Testing Q6 under ByteSlice... "    
				
 for j in 0 1 2 3 4 5 6 7 8 9 10 11   #
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q6_best_worst_result.txt
   ./Q6.x 1 $j >>Q6_best_worst_result.txt
 done  

    
 