#pragma once

#include "return_codes.h"

#include <fftw3.h>

#include <stdlib.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif 

#ifdef MYFFTW

	int8_t DFT(double* in1, double* in2, double* bigOut, int32_t* memoryCount);

	fftw_complex* multi(int32_t len, fftw_complex* in1, fftw_complex* in2);

#endif // MYFFTW
