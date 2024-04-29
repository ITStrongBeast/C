#include "my_ffmpeg_header.h"

typedef struct Streams
{
	int8_t flagChannel;
	int8_t numberChannel;
	int32_t numberStream;
	int32_t sampleRate;
	SwrContext* swrContext;
	AVFormatContext* formatContext;
	AVCodecContext* codecContext;
	AVCodecParameters* codecParameters;
	AVCodec* codec;
	AVPacket* packet;
	AVFrame* frame;
} Stream;

int8_t getFrame(double** ArrayResult, int32_t* count, int32_t* memoryCount, Stream* Result)
{
	avcodec_send_packet(Result->codecContext, Result->packet);
	avcodec_receive_frame(Result->codecContext, Result->frame);
	;
	int32_t countSamp = Result->frame->nb_samples;
	if (*count + countSamp >= *memoryCount - 1)
	{
		*memoryCount = 2 * (*memoryCount > countSamp ? *memoryCount : countSamp);
		*ArrayResult = realloc(*ArrayResult, *memoryCount * sizeof(double));
		if (!*ArrayResult)
		{
			fprintf(stderr, "Incorrect number of arguments");
			return ERROR_NOTENOUGH_MEMORY;
		}
	}
	for (int32_t i = 0; i < countSamp; i++)
	{
		if (Result->numberChannel == 0)
		{
			(*ArrayResult)[(*count)++] = (double)Result->frame->data[0][i];
		}
		else
		{
			(*ArrayResult)[(*count)++] =
				(double)(av_sample_fmt_is_planar(Result->codecContext->sample_fmt)
							 ? Result->frame->data[1][i]
							 : Result->frame->data[0][countSamp + i]);
		}
	}
	av_frame_unref(Result->frame);
	av_packet_unref(Result->packet);
	return SUCCESS;
}

int8_t getSamples(double** ArrayResult, Stream* Result, int32_t* memoryCount, int32_t* count)
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
	if (Result->flagChannel && Result->codecContext->ch_layout.nb_channels != 2)
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_FORMAT_INVALID;
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
	}
	return SUCCESS;
}

int8_t getFirstStream(char argv[], Stream* Result)
{
	Result->formatContext = avformat_alloc_context();
	if (!Result->formatContext)
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_NOTENOUGH_MEMORY;
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
	int32_t format = Result->codecParameters->codec_id;
	if (format != AV_CODEC_ID_MP2 && format != AV_CODEC_ID_MP3 && format != AV_CODEC_ID_FLAC &&
		format != AV_CODEC_ID_OPUS && format != AV_CODEC_ID_AAC)
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_UNKNOWN;
	}
	Result->sampleRate = Result->codecParameters->sample_rate;
	return SUCCESS;
}

int8_t alignment(double** Result, int32_t* resultMemoryCount, int32_t memoryCount, int32_t count)
{
	if (memoryCount < *resultMemoryCount)
	{
		*Result = realloc(*Result, *resultMemoryCount * sizeof(double));
		if (!*Result)
		{
			fprintf(stderr, "Incorrect number of arguments");
			return ERROR_NOTENOUGH_MEMORY;
		}
	}
	for (int32_t i = count; i < *resultMemoryCount; i++)
	{
		(*Result)[i] = 0;
	}
	return SUCCESS;
}

int8_t start(char firstFile[], char secondFile[], double** firstResult, double** secondResult, int32_t* sampleRateResult, int32_t* memoryCount)
{
	Stream FirstResult;
	int8_t check1 = getFirstStream(firstFile, &FirstResult);
	FirstResult.numberChannel = 0;
	if (check1)
	{
		return check1;
	}
	Stream SecondResult;
	int8_t check2 = 0;
	FirstResult.flagChannel = 0;
	if (!secondFile)
	{
		SecondResult = FirstResult;
		SecondResult.numberChannel = 1;
		FirstResult.flagChannel = 1;
	}
	else
	{
		check2 = getFirstStream(secondFile, &SecondResult);
		SecondResult.numberChannel = 0;
		SecondResult.flagChannel = 0;
	}
	if (check2)
	{
		return check2;
	}
	*sampleRateResult = FirstResult.sampleRate;
	if (!*firstResult || !*secondResult)
	{
		fprintf(stderr, "Incorrect number of arguments");
		return ERROR_NOTENOUGH_MEMORY;
	}
	if (FirstResult.sampleRate != SecondResult.sampleRate)
	{
		return ERROR_FORMAT_INVALID;
	}
	int32_t startMemoryCount = *memoryCount;
	int32_t a = 0;
	int32_t* startCount = &a;
	int8_t check3 = getSamples(firstResult, &FirstResult, memoryCount, startCount);
	if (check3)
	{
		return check3;
	}
	int32_t firstMemoryCount = *memoryCount;
	int32_t firstCount = *startCount;
	*startCount = 0;
	*memoryCount = startMemoryCount;
	int8_t check4 = getSamples(secondResult, &SecondResult, memoryCount, startCount);
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
