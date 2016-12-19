/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * Import form Cagri Balkesen.
 * Description: Provides cpu mapping utility function.
 *******************************************************************************/

 
#ifndef CPU_MAPPING_H
#define CPU_MAPPING_H


/** 
 * if the custom cpu mapping file exists, logical to physical mappings are
 * initialized from that file, otherwise it will be round-robin 
 */
#ifndef CUSTOM_CPU_MAPPING
#define CUSTOM_CPU_MAPPING "cpu-mapping.txt"
#endif

/**
 * Returns SMT aware logical to physical CPU mapping for a given thread id.
 */
int get_cpu_id(int thread_id);

/** @} */

#endif /* CPU_MAPPING_H */
