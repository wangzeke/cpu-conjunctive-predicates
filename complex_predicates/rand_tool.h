/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains the header for random number generation. 

 * See file LICENSE.md for details.
 *******************************************************************************/


#ifndef _RAND_H_
#define _RAND_H_


typedef struct rand_state_32 rand32_t;

rand32_t *rand32_init(uint32_t seed);

uint32_t rand32_next(rand32_t *state);


#endif
