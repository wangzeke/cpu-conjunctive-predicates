#!/bin/bash
#output the information to Q17_best_worst_result.txt

echo "Testing Q17 under ByteSlice... "    
				
 for j in 0 1 2 3 4 5
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q17_best_worst_result.txt
   ./Q17.x 1 $j >>Q17_best_worst_result.txt
 done  

     