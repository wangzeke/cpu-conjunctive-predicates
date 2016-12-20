/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains the utility for managing memory operations. 
 * 
 * See file LICENSE.md for details.
 *******************************************************************************/
 
#include <assert.h> 
#include <stdlib.h> 
#include <sys/mman.h> 
#include <string.h>
#include <stdio.h>
  
#include "huge_page.h"

//malloc the memory with huge table or not.
void *malloc_memory(size_t size, bool huge_page_enable)
{
	if (huge_page_enable == true)
	{
		return malloc_huge_pages(size);
	}
	else
	{
	  void *p_ptr;
      size_t ret  = posix_memalign((void**)&p_ptr, 128, size);   
	  if (ret) {
            printf ( "in malloc memory, posix_memalign fails: %s\n", strerror(ret) );
            return NULL;
         }
      return p_ptr;		 
	}
}

//free the memory with huge table or not.
void free_memory(void *ptr, bool huge_page_enable)
{
	if (huge_page_enable == true)
	{
		return free_huge_pages(ptr) ;
	}
	else
	{
       free(ptr);	 
	}
}
