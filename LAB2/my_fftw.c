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

void cleaningComplex(fftw_complex* in1, fftw_complex* in2)
{
	fftw_free(in1);
	fftw_free(in2);
}

int8_t DFT(double* in1, double* in2, double* bigOut, int32_t* memoryCount)
{
	fftw_complex* firstResult = fftw_alloc_complex(*memoryCount * sizeof(fftw_complex));
	fftw_complex* secondResult = fftw_alloc_complex(*memoryCount * sizeof(fftw_complex));
	if (!firstResult || !secondResult)
	{
		fprintf(stderr, "Failed to allocate memory in fftw_alloc_complex");
		cleaningComplex(firstResult, secondResult);
		return ERROR_NOTENOUGH_MEMORY;
	}
	fftw_execute(fftw_plan_dft_r2c_1d(*memoryCount, in1, firstResult, FFTW_ESTIMATE));
	fftw_execute(fftw_plan_dft_r2c_1d(*memoryCount, in2, secondResult, FFTW_ESTIMATE));
	fftw_execute(fftw_plan_dft_c2r_1d(*memoryCount, multi(*memoryCount, firstResult, secondResult), bigOut, FFTW_ESTIMATE));
	cleaningComplex(firstResult, secondResult);
	return SUCCESS;
}
