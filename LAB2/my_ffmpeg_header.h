#pragma once

#include "return_codes.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>

#include <stdlib.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif 

#ifdef MYFFMMPEG

	int8_t start(char firstFile[], char secondFile[], double* firstResult, double* secondResult, int32_t* sampleRateResult, int32_t* memoryCount);

	int8_t getFrame(double* ArrayResult, int32_t* count, int32_t* memoryCount, Stream* Result);

	int8_t giveSamples(double* ArrayResult, Stream* Result, int32_t* memoryCount, int32_t* count);

	int8_t giveFirstStream(char argv[], Stream* Result);

	int8_t alignment(double* Result, int32_t* resultMemoryCount, int32_t memoryCount, int32_t count);

#endif // MYFFMMPEG
