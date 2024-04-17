
#include "my_fftw_header.h"

fftw_complex* multi(int32_t len, fftw_complex* in1, fftw_complex* in2)
{
	double a, b, c, d;
	for (int32_t i = 0; i < len; i++)
	{
		a = in1[i][0];
		b = in1[i][1];
		c = in2[i][0];
		d = -1 * in2[i][1];
		in1[i][0] = a * c - b * d;
		in1[i][1] = b * c + a * d;
	}
	return in1;
}

void copy(double* in, double* In, int32_t memoryCoun) 
{
	for (int32_t i = 0; i < memoryCoun; i++) 
	{
		In[i] = in[i];
	}
}

void cleaningComplex(fftw_complex* in1, fftw_complex* in2) 
{
	fftw_free(in1);
	fftw_free(in2);
}

void cleaningReal(double* in1, double* in2, double* in3) 
{
	fftw_free(in1);
	fftw_free(in2);
	fftw_free(in3);
}

int8_t DFT(double* in1, double* in2, double* bigOut, int32_t* memoryCount) 
{
	double* firstIn = fftw_alloc_real(*memoryCount * sizeof(double)); 
	double* secondIn = fftw_alloc_real(*memoryCount * sizeof(double));
	double* Out = fftw_alloc_real(*memoryCount * sizeof(double));
	if (!firstIn || !secondIn)
	{
		fprintf(stderr, "Incorrect number of arguments");
		cleaningReal(firstIn, secondIn, Out);
		return ERROR_NOTENOUGH_MEMORY;
	}
	copy(in1, firstIn, *memoryCount);
	copy(in2, secondIn, *memoryCount);
	fftw_complex* firstResult = fftw_alloc_complex(*memoryCount * sizeof(fftw_complex));
	fftw_complex* secondResult = fftw_alloc_complex(*memoryCount * sizeof(fftw_complex));
	if (!firstResult || !secondResult)
	{
		fprintf(stderr, "Incorrect number of arguments");
		cleaningReal(firstIn, secondIn, Out);
		cleaningComplex(firstResult, secondResult);
		return ERROR_NOTENOUGH_MEMORY;
	}
	fftw_execute(fftw_plan_dft_r2c_1d(*memoryCount, firstIn, firstResult, FFTW_ESTIMATE));
	fftw_execute(fftw_plan_dft_r2c_1d(*memoryCount, secondIn, secondResult, FFTW_ESTIMATE));
	fftw_execute(fftw_plan_dft_c2r_1d(*memoryCount, multi(*memoryCount, firstResult, secondResult), Out, FFTW_ESTIMATE));
	copy(Out, bigOut, *memoryCount);
	cleaningReal(firstIn, secondIn, Out);
	cleaningComplex(firstResult, secondResult);
	return SUCCESS;
}
