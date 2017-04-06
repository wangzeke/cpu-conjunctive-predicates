#!/bin/bash
#output the information to Q3_best_worst_result.txt

echo "Testing Q3 under ByteSlice... "    
				
 for j in 0 1 2 3 4 5
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q3_best_worst_result.txt
   ./Q3.x 1 $j >>Q3_best_worst_result.txt
 done  

    