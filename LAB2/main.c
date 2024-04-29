#include "my_ffmpeg_header.h"
#include "my_fftw_header.h"

int32_t findDelta(int32_t len, double* in)
{
	int32_t count = 0;
	double max = -1.0;
	for (int32_t i = 0; i < len; i++)
	{
		if (in[i] > max)
		{
			max = in[i];
			count = i;
		}
	}
	if (count <= len / 2)
	{
		return count;
	}
	return count - len;
}

void cleaning(double* in1, double* in2, double* in3)
{
	free(in1);
	free(in2);
	if (in3)
	{
		free(in3);
	}
}

int main(int argc, char* argv[])
{
	av_log_set_level(AV_LOG_QUIET);
	if (4 < argc || argc < 2)
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_ARGUMENTS_INVALID;
	}
	int32_t buffer = 0, size = 1000;
	int32_t *sr = &buffer, *memoryCount = &size;
	double* in1 = malloc(size * sizeof(double));
	double* in2 = malloc(size * sizeof(double));
	if (!in1 || !in2)
	{
		fprintf(stderr, "Failed to allocate memory in malloc");
		return ERROR_NOTENOUGH_MEMORY;
	}
	int8_t check1 = start(argv[1], argv[2], &in1, &in2, sr, memoryCount);
	if (check1 != SUCCESS)
	{
		cleaning(in1, in2, NULL);
		return check1;
	}
	double* bigOut = malloc(*memoryCount * sizeof(double));
	if (!bigOut)
	{
		cleaning(in1, in2, NULL);
		fprintf(stderr, "Failed to allocate memory in malloc");
		return ERROR_NOTENOUGH_MEMORY;
	}
	int8_t check2 = DFT(in1, in2, bigOut, memoryCount);
	if (check2 != SUCCESS)
	{
		cleaning(in1, in2, bigOut);
		return check2;
	}
	int32_t delta = findDelta(*memoryCount, bigOut);
	printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n", delta, *sr, delta * 1000 / *sr);
	cleaning(in1, in2, bigOut);
	return SUCCESS;
}
