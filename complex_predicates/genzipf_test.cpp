/**
 * @file
 *
 * Test Zipf-distributed input data.
 *
 * @author Zeke wang <zeke.wang@inf.ethz.ch>
 *
 * (c) ETH Zurich, Systems Group
 */
#include <iostream>
#include <fstream>
using namespace std;
#include "genzipf.h"
#define NUM 50

void main(int argc, char **argv)
{
	double    zipf_factor    = argc > 1? atof(argv[1]) : 0.0;
    uint32_t  alphabet_size  = argc > 2? atoi(argv[2]) : 256;
    uint32_t  model          = argc > 3? atoi(argv[3]) : 0;

    printf("zipf_factor = %f, alphabet_size = %d, model = %d\n", zipf_factor, alphabet_size, model); 

	uint32_t tuples          = 1000*1000*1000;
	uint32_t*zipf_data       = (uint32_t*)malloc(tuples*sizeof(uint32_t));

	gen_zipf(tuples, alphabet_size, zipf_factor, zipf_data);
	printf("zipf data is generated.\n");

	ofstream myfile ("skew_data.txt");


	uint32_t count[NUM]       = {0};
	for (uint32_t i = 0; i < tuples; i++)
	{	
		uint32_t cmp_base = 132;
		uint32_t cmp_inc  = 132;
		for (uint32_t j = 0; j < NUM; j++)
		{
			if (zipf_data[i] < cmp_base)
			{
				count[j]++;
			}
			cmp_base       += cmp_inc;
		}
		if (myfile.is_open())
			myfile << zipf_data[i] << '\n';
	}
	//printf the cdf...
	uint32_t cmp_base = 132;
	uint32_t cmp_inc  = 132;
    for (int i = 0; i < NUM; i++)
    {
        printf("index:%d, cmp_base = %d:  %d\n", i, cmp_base, count[i]);
        cmp_base += cmp_inc;
    }

    free(zipf_data); 

}


#if 0
void main(int argc, char **argv)
{
	double    zipf_factor    = argc > 1? atof(argv[1]) : 0.0;
    uint32_t  alphabet_size  = argc > 2? atoi(argv[2]) : 256;
    printf("zipf_factor = %f, alphabet_size = %d\n", zipf_factor, alphabet_size); 

	uint32_t tuples          = 1000*1000*1000;
	uint32_t*zipf_data       = (uint32_t*)malloc(tuples*sizeof(uint32_t));

	gen_zipf(tuples, alphabet_size, zipf_factor, zipf_data);
	printf("zipf data is generated.\n");


	uint32_t count[NUM]       = {0};
	for (uint32_t i = 0; i < tuples; i++)
	{	
		uint32_t cmp_base = 132;
		uint32_t cmp_inc  = 132;
		for (uint32_t j = 0; j < NUM; j++)
		{
			if (zipf_data[i] < cmp_base)
			{
				count[j]++;
			}
			cmp_base       += cmp_inc;
		}
	}
	//printf the cdf...
	uint32_t cmp_base = 132;
	uint32_t cmp_inc  = 132;
    for (int i = 0; i < NUM; i++)
    {
        printf("index:%d, cmp_base = %d:  %d\n", i, cmp_base, count[i]);
        cmp_base += cmp_inc;
    }

    free(zipf_data); 

}


void main(int argc, char **argv)
{
	double    zipf_factor        = argc > 1? atof(argv[1]) : 0.0;
    uint32_t  alphabet_size      = argc > 2? atoi(argv[2]) : 256;
    printf("zipf_factor = %f, alphabet_size = %d\n", zipf_factor, alphabet_size); 

    double *lut;              /**< return value */
    lut = gen_zipf_lut(zipf_factor, alphabet_size);//(double *) malloc (alphabet_size * sizeof (*lut));
    assert (lut);
    for (int i = 0; i < 20; i++)
        printf("%d: %f\n", i, lut[i]);

    free(lut); 
}
#endif

