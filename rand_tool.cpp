/*******************************************************************************
 * Copyright (c) 2016
 * The National University of Singapore, Xtra Group
 *
 * Author: Zeke Wang (wangzeke638 AT gmail.com)
 * 
 * Description: this file contains the defination for random number generation. 

 * See file LICENSE.md for details.
 *******************************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


typedef struct rand_state_32 {
	uint32_t num[625];
	size_t index;
} rand32_t;

rand32_t *rand32_init(uint32_t seed)
{
	rand32_t *state = (rand32_t *) malloc(sizeof(rand32_t));
	uint32_t *n = state->num;
	size_t i;
	n[0] = seed;
	for (i = 0 ; i != 623 ; ++i)
		n[i + 1] = 0x6c078965 * (n[i] ^ (n[i] >> 30));
	state->index = 624;
	return state;
}

uint32_t rand32_next(rand32_t *state)
{
	uint32_t y, *n = state->num;
	if (state->index == 624) {
		size_t i = 0;
		do {
			y = n[i] & 0x80000000;
			y += n[i + 1] & 0x7fffffff;
			n[i] = n[i + 397] ^ (y >> 1);
			n[i] ^= 0x9908b0df & -(y & 1);
		} while (++i != 227);
		n[624] = n[0];
		do {
			y = n[i] & 0x80000000;
			y += n[i + 1] & 0x7fffffff;
			n[i] = n[i - 227] ^ (y >> 1);
			n[i] ^= 0x9908b0df & -(y & 1);
		} while (++i != 624);
		state->index = 0;
	}
	y = n[state->index++];
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680;
	y ^= (y << 15) & 0xefc60000;
	y ^= (y >> 18);
	return y;
}
