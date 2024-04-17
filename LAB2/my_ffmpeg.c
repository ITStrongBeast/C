#include "my_ffmpeg_header.h"

typedef struct Streams
{
	int8_t numberСhannel;
	int32_t numberStream;
	int32_t sampleRate;
	AVFormatContext* formatContext;
	AVCodecContext* codecContext;
	AVCodecParameters* codecParameters;
	AVCodec* codec;
	AVPacket* packet;
	AVFrame* frame;
} Stream;

int8_t getFrame(double** ArrayResult, int32_t* count, int32_t* memoryCount, Stream* Result)
{
	int32_t cnt = avcodec_send_packet(Result->codecContext, Result->packet);
	int32_t countSamp;
	while (cnt >= 0) 
	{
		cnt = avcodec_receive_frame(Result->codecContext, Result->frame);
		countSamp = Result->frame->nb_samples;
		if (*count + countSamp >= *memoryCount - 2)
		{
			*memoryCount += (13 * countSamp > *memoryCount) ? 13 * countSamp : *memoryCount;
			if (!realloc(*ArrayResult, *memoryCount * sizeof(double)))
			{
				fprintf(stderr, "Incorrect number of arguments");
				return ERROR_NOTENOUGH_MEMORY;
			}
		}
		for (int32_t i = 0; i < countSamp; i++) 
		{
			(*ArrayResult)[*count] = (double) Result->frame->data[Result->numberСhannel][i];
			*count += 1;
		}
		av_frame_unref(Result->frame);
	}
	return SUCCESS;
}

int8_t giveSamples(double** ArrayResult, Stream* Result, int32_t* memoryCount, int32_t* count) 
{
	Result->codecContext = avcodec_alloc_context3(Result->codec);
	Result->packet = av_packet_alloc();
	Result->frame = av_frame_alloc();
	if (!Result->codecContext || !Result->packet || !Result->frame) 
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_NOTENOUGH_MEMORY;
	}
	if (avcodec_parameters_to_context(Result->codecContext, Result->codecParameters) < 0 ||
		avcodec_open2(Result->codecContext, Result->codec, NULL) < 0) 
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_UNKNOWN;
	}
	int8_t check = SUCCESS;
	while (av_read_frame(Result->formatContext, Result->packet) >= 0)
	{
		if (Result->packet->stream_index == Result->numberStream)
		{
			check = getFrame(ArrayResult, count, memoryCount, Result);
			if (check != SUCCESS) 
			{
				return check;
			}
		}
		av_packet_unref(Result->packet);
	}
	return SUCCESS;
}

int8_t giveFirstStream(char argv[], Stream* Result) 
{
	Result->formatContext = avformat_alloc_context();
	if (!Result->formatContext) 
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_ARGUMENTS_INVALID;
	}
	if (avformat_open_input(&Result->formatContext, argv, NULL, NULL) != 0 ||
		avformat_find_stream_info(Result->formatContext, NULL) < 0) 
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_CANNOT_OPEN_FILE;
	}
	Result->numberStream = av_find_best_stream(Result->formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (Result->numberStream < 0) 
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_CANNOT_OPEN_FILE;
	}
	Result->codecParameters = Result->formatContext->streams[0]->codecpar;
	Result->codec = avcodec_find_decoder(Result->codecParameters->codec_id);
	if (!Result->codec) 
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_NOTENOUGH_MEMORY;
	}
	Result->sampleRate = Result->codecParameters->sample_rate;
	return SUCCESS;
}

int8_t alignment(double** Result, int32_t* resultMemoryCount, int32_t memoryCount, int32_t count) 
{
	if (memoryCount < *resultMemoryCount) 
	{
		if (!realloc(*Result, *resultMemoryCount * sizeof(double)))
		{
			fprintf(stderr, "Incorrect number of arguments");
			return ERROR_NOTENOUGH_MEMORY;
		}
	}
	for (int32_t i = count - 1; i < *resultMemoryCount; i++) 
	{
		(*Result)[i] = 0;
	}
	return SUCCESS;
}

int8_t start(char firstFile[], char secondFile[], double** firstResult, double** secondResult, int32_t* sampleRateResult, int32_t* memoryCount) 
{
	Stream FirstResult;
	int8_t check1 = giveFirstStream(firstFile, &FirstResult);
	FirstResult.numberСhannel = 0;
	if (check1) 
	{
		return check1;
	}
	Stream SecondResult;
	int8_t check2 = 0;
	if (!secondFile) 
	{
		if (FirstResult.formatContext->nb_streams != 2) 
		{
			fprintf(stderr, "Incorrect number of arguments");
			return ERROR_ARGUMENTS_INVALID;
		}
		SecondResult = FirstResult;
		SecondResult.numberСhannel = 1;
	}
	else 
	{
		check2 = giveFirstStream(secondFile, &SecondResult);
		SecondResult.numberСhannel = 0;
	}
	if (check2) 
	{
		return check2;
	}
	if (FirstResult.sampleRate > SecondResult.sampleRate) 
	{
		// передискретизация
	}
	else if (FirstResult.sampleRate < SecondResult.sampleRate) 
	{
		// передискретизация
	}
	*sampleRateResult = FirstResult.sampleRate;
	if (!*firstResult || !*secondResult) 
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_NOTENOUGH_MEMORY;
	}
	int32_t startMemoryCount = *memoryCount;
	int32_t a = 0;
	int32_t* startCount = &a;
	int8_t check3 = giveSamples(firstResult, &FirstResult, memoryCount, startCount);
	if (check3) 
	{
		return check3;
	} 
	int32_t firstMemoryCount = *memoryCount;
	int32_t firstCount = *startCount;
	*startCount = 0;
	*memoryCount = startMemoryCount;
	int8_t check4 = giveSamples(secondResult, &SecondResult, memoryCount, startCount);
	if (check4)
	{
		return check4;
	}
	int32_t secondMemoryCount = *memoryCount;
	int32_t secondCount = *startCount;
	*memoryCount = (firstCount + secondCount - 1) + 13 - (firstCount + secondCount - 1) % 13;
	int8_t check5 = alignment(firstResult, memoryCount, firstMemoryCount, firstCount);
	int8_t check6 = alignment(secondResult, memoryCount, secondMemoryCount, secondCount);
	if (check5 != SUCCESS || check6 != SUCCESS) 
	{
		return check5;
	}
	return SUCCESS;
}

//		SwrContext *resameContext = NUll;
//		swr_alloc_set_opts2(&resameContext,
//							pLocalCodecParameters2->ch_layout,
//							pLocalCodecParameters2->format,
//							sr1,
//							pLocalCodecParameters2->ch_layout,
//							pLocalCodecParameters2->format,
//							sr2,
//							0,
//							NULL,
//							NULL);

// 1) NULL 2) stream[i] -> codecpar -> ch_layout 3) stream[i] -> format 4) больший sample rate 5)
// stream[i] -> codecpar -> ch_layout 6) stream[i] -> format 5) исходный sample rate 6) 0 7) NULL