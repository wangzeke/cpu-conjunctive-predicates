#!/usr/bin/env python




import os



def pomoto(old_list,new_list):
    for i in range(len(old_list)):
        if i>0 and old_list[i] != old_list[i-1] and old_list[i] is not None:
            new_list.append(old_list[i])
        elif i ==0:
            new_list.append(old_list[i])



#filename_list = ["name.txt"]

filename_list = ["n1_name.txt", "n2_name.txt", "r1_name.txt", "r2_name.txt", "p_type.txt", "c_mktsegment.txt"]
output_filename_prefix = "output_"
dict_filename_prefix = "dict_"

for k in range(len(filename_list)):
    raw_name = []
    clean_name=[]
    null_count = 0
    
    filename = filename_list[k]
    output_filename = output_filename_prefix + filename
    dict_filename = dict_filename_prefix + filename
    

    with open(filename, "r") as name1:
        lines = name1.readlines()   
        
        for data in lines:                 
            name_value = data.strip()
            if not name_value:
                break
            if name_value == "null":
                null_count += 1
                continue
            raw_name.append(name_value)
    name1.close()

    print "load finished"
    
    raw_sorted_name = sorted(raw_name)

    print "sort finished"

    pomoto(raw_sorted_name, clean_name)

    print "clean date get"

    
    with open(output_filename, "w") as f1:
        for i in range(len(raw_name)):
            f1.write(str(1 + clean_name.index(raw_name[i])))
            f1.write("\n")
            #print clean_date.index(raw_date[i])
        for i in range(null_count):
            f1.write(str(0))           
            f1.write("\n")
    f1.close()

    print "output finished"
    
    with open(dict_filename, "w") as d1:
        d1.write(str(0))
        d1.write(" ")
        d1.write("null")        
        d1.write("\n")
        for i in range(len(clean_name)):
            d1.write(str(i+1))
            d1.write(" ")
            d1.write(clean_name[i])
            d1.write("\n")
    d1.close()

    print "dict finished"
