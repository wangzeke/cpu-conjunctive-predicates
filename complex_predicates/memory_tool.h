/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains the header for memory_tool.h. 

 * See file LICENSE.md for details.
 *******************************************************************************/
#ifndef MEMORY_TOOL_H
#define MEMORY_TOOL_H

void *malloc_memory(size_t size, bool huge_page_enable);

void free_memory(void *ptr, bool huge_page_enable);

#endif

