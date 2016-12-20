#!/bin/bash
echo $1    #prefetching model and streaming model, 0:disable 1:enable
echo $2    #output file name.....

for i in 1 2 3 4 5 6 7 8 16 
do
  echo ""  >>$2
  echo "With threads:" $i >>$2
  for j in 3 7 11 15 19 23 27 31 
  do
   echo "With width:" $j >>$2
   for k in 0 7 8 15
   do
     for l in 0 1
	 do
       ./impact_of_prefetcher_tlb.x $i $l $j $k $2 >result.txt
       cat result.txt |grep "huge_table_enable"  >>$2
       cat result.txt |grep "BytesReadFromMC"  >>$2
       cat result.txt |grep "BytesWrittenToMC" >>$2 
       cat result.txt |grep "codes_per_ns" >>$2
       rm result.txt 
     done	   
   done
  done
done
