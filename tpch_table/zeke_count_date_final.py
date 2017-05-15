#!/usr/bin/env python




import os

def sanitize(time_string):
    if '-' in time_string:    
        splitter='-'
    else:
        return(0)
    (years, month, date) = time_string.split(splitter)
    temp = ((int(years)-1990)*12*31+(int(month)-1)*31+ int(date))
    return temp
    


#filename_list = ["date.txt"]
#filename_list = ["l_commitdate.txt"]
filename_list = ["l_commitdate.txt", "l_receiptdate.txt", "l_shipdate.txt", "o_orderdate.txt"]
output_filename_prefix = "output_"

for k in range(len(filename_list)):
    raw_date = []
    clean_date=[]
    
    filename = filename_list[k]
    output_filename = output_filename_prefix + filename
    

    with open(filename, "r") as date1, open(output_filename, "w") as f1:
        lines = date1.readlines()
        for data in lines:
            if not data:
                break
            date_value = data.strip()
            f1.write(str((sanitize(date_value))))
            f1.write("\n")
        date1.close()
        f1.close()

    print "finished"
    
        
 
