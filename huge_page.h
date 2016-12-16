/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains header of huge page functions. 
 * Warning: please make sure the command: echo 2048 >/proc/sys/vm/nr_hugepages is executed before using
 * the huge table-based memory allocation. 
 * See file LICENSE.md for details.
 *******************************************************************************/
#ifndef HUGE_PAGE_H
#define HUGE_PAGE_H

void *malloc_huge_pages(size_t size);

void free_huge_pages(void *ptr);
 
#endif

