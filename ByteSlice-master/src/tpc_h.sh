#!/bin/bash
#echo "Welcome to "    
echo "TPC-H result under ByteSlice....:::::::"    #output file name.....
echo "TPC-H Q3 under ByteSlice....:::::::" >TPC-H-result.txt   
./Q3.x >TPC-H-result.txt
echo "TPC-H Q5 under ByteSlice....:::::::" >TPC-H-result.txt   
./Q5.x >TPC-H-result.txt
echo "TPC-H Q6 under ByteSlice....:::::::" >TPC-H-result.txt   
./Q6.x >TPC-H-result.txt
echo "TPC-H Q7 under ByteSlice....:::::::" >TPC-H-result.txt   
./Q7.x >TPC-H-result.txt
echo "TPC-H Q8 under ByteSlice....:::::::" >TPC-H-result.txt   
./Q8.x >TPC-H-result.txt
echo "TPC-H Q12 under ByteSlice....:::::::" >TPC-H-result.txt   
./Q12.x >TPC-H-result.txt
 