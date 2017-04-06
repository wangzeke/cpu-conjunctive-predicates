#!/bin/bash
#output the information to Q12_best_worst_result.txt

echo "Testing Q12 under ByteSlice... "    
				
 for j in 4 5 6 7 #0 1 2 3 
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q12_best_worst_result.txt
   ./Q12.x 1 $j >>Q12_best_worst_result.txt
 done  

    
 