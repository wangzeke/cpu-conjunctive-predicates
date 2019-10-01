/**
 * @file    genzipf.h
 * @version $Id: genzipf.h 3017 2012-12-07 10:56:20Z bcagri $
 *
 * Data generation with Zipf distribution.
 *
 * @author Jens Teubner <jens.teubner@inf.ethz.ch>
 *
 * (c) 2011 ETH Zurich, Systems Group
 *
 * $Id: genzipf.h 3017 2012-12-07 10:56:20Z bcagri $
 */

#ifndef GENZIPF_H
#define GENZIPF_H

//#include "types.h" /* tuple_t */
#include "stdint.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef uint32_t item_t;

void gen_zipf (uint32_t  stream_size,
               uint32_t  alphabet_size,
               double    zipf_factor,
               uint32_t* output);

double *gen_zipf_lut (double zipf_factor, 
                      unsigned int alphabet_size);
#endif  /* GENZIPF_H */
