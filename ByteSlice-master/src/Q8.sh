#!/bin/bash
#output the information to Q8_best_worst_result.txt

echo "Testing Q8 under ByteSlice... "    
				
 for j in 0 1 2 3 4 5 6 7 
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q8_best_worst_result.txt
   ./Q8.x 1 $j >>Q8_best_worst_result.txt
 done  

    
 