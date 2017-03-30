#!/bin/bash
   for stride in 1 2 4 8 16 32 64 
   do
     echo "For the selectivites from 0 to 1 with step 0.1. memory performance impact of Stride:" $stride  >>impact_of_branch_diff_stride1.txt
     ./impact_of_branch_on_cpu.x 1 $stride >>impact_of_branch_diff_stride1.txt
   done
