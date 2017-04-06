#!/bin/bash

echo "Testing Q3 under ByteSlice... "    
   ./Q3.x 1 $j >>TPC_H_proposed_result.txt

echo "Testing Q5 under ByteSlice... "    
   ./Q5.x 1 $j >>TPC_H_proposed_result.txt

echo "Testing Q6 under ByteSlice... "    
   ./Q6.x 1 $j >>TPC_H_proposed_result.txt

echo "Testing Q7 under ByteSlice... "    
   ./Q7.x 1 $j >>TPC_H_proposed_result.txt

echo "Testing Q8 under ByteSlice... "    
   ./Q8.x 1 $j >>TPC_H_proposed_result.txt
 
echo "Testing Q12 under ByteSlice... "    
   ./Q12.x 1 $j >>TPC_H_proposed_result.txt

    
 