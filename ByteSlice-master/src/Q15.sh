#!/bin/bash
#output the information to Q15_best_worst_result.txt

echo "Testing Q15 under ByteSlice... "    
				
 for j in 0 1
 do
   echo "Execution model: " $j
   echo "Execution model: " $j >>Q15_best_worst_result.txt
   ./Q15.x 1 $j >>Q15_best_worst_result.txt
 done   

     