#include <psp2/kernel/iofilemgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/sysmodule.h>
#include <psp2/audiocodec.h>
#include <stdlib.h>
#include <math.h>

#include "audio.h"
#include "config.h"
#include "touch.h"
#include "id3.h"
#include "utils.h"
#include "vitaaudiolib.h"
#include "menu_audioplayer.h"

static SceAudiocodecCtrl codecCtrl;
static void* streamBuffer[2];
static SceUInt32 codecType;
static SceUInt32 bufindex = 0;
static SceUInt32 callCount = 0;

static SceUID file_fd;
static SceUID opHandle;
static SceUID adecWorkMem;

static SceIoAsyncParam asyncParam;

static SceUInt64 totalEsSamples = 0;
static SceUInt64 totalEsPlayed = 0;

static SceSize dataBeginOffset = 0;
static SceSize readOffset = 0;
static SceSize current_offset;

#define EA3_HEADER_SIZE 96
#define GROUP_SIZE 64
#define AT3P_ES_GRAIN 2048
#define AT3_ES_GRAIN 1024

typedef enum At3Types {
	OMA_CODECID_ATRAC3 = 0,
	OMA_CODECID_ATRAC3P = 1,
} At3Types;

SceInt32 sceAudiocodecGetAt3ConfigPSP2(SceUInt32 cmode, SceUInt32 nbytes)
{
	if ((cmode == 0) && (nbytes == 0xc0)) {
		return 4;
	}
	if ((cmode == 0) && (nbytes == 0x98)) {
		return 6;
	}
	if ((cmode == 1) && (nbytes == 0x60)) {
		return 0xb;
	}
	if ((cmode == 2) && (nbytes == 0xc0)) {
		return 0xe;
	}
	if ((cmode == 2) && (nbytes == 0x98)) {
		return 0xf;
	}
	return -1;
}

static int AT3_Init_common(const char *path, SceUInt8 type, SceUInt8 param1, SceUInt8 param2, SceSize dataOffset) {

	int ret;

	//Initialize SceAudiocodec
	sceClibMemset(&asyncParam, 0, sizeof(SceIoAsyncParam));
	sceClibMemset(&codecCtrl, 0, sizeof(SceAudiocodecCtrl));
	switch (type) {
	case OMA_CODECID_ATRAC3P:
		codecType = SCE_AUDIOCODEC_ATX;
		codecCtrl.atx.codecParam1 = param1;
		codecCtrl.atx.codecParam2 = param2;
		break;
	case OMA_CODECID_ATRAC3:
		codecType = SCE_AUDIOCODEC_AT3;
		codecCtrl.at3.codecParam = sceAudiocodecGetAt3ConfigPSP2(param1, param2);
		break;
	}

	ret = sceAudiocodecQueryMemSize(&codecCtrl, codecType);
	if (-1 < ret) {

		SceSize memsize = 0x8000;
		if (codecCtrl.neededWorkMem > memsize)
			memsize = 0x10000;
		adecWorkMem = sceKernelAllocMemBlock("ElevenMPV_adec-work", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, memsize, NULL);
		sceKernelGetMemBlockBase(adecWorkMem, &codecCtrl.pWorkMem);

		if (codecType == SCE_AUDIOCODEC_ATX)
			codecCtrl.unk_atx_0C = 1;

		ret = sceAudiocodecInit(&codecCtrl, codecType);
		if ((ret < 0) && (codecCtrl.pWorkMem != NULL)) {
			free(codecCtrl.pWorkMem);
			codecCtrl.pWorkMem = NULL;
			return -1;
		}
		else if (ret < 0) {
			return -1;
		}
	}
	else {
		return -1;
	}

	//Allocate stream buffer, read first es streams to buffer, set output grain
	file_fd = sceIoOpen(path, SCE_O_RDONLY, 0);
	SceIoStat stat;
	sceIoGetstatByFd(file_fd, &stat);
	SceSize atDataSize;

	switch (type) {
	case OMA_CODECID_ATRAC3P:
		streamBuffer[0] = malloc(codecCtrl.atx.inputEsSize * GROUP_SIZE);
		streamBuffer[1] = malloc(codecCtrl.atx.inputEsSize * GROUP_SIZE);
		current_offset += sceIoPread(file_fd, streamBuffer[0], codecCtrl.atx.inputEsSize * GROUP_SIZE, (SceOff)dataOffset);
		//Calculate total samples
		atDataSize = (SceSize)stat.st_size - dataOffset;
		totalEsSamples = atDataSize / codecCtrl.atx.inputEsSize;
		vitaAudioPreSetGrain(AT3P_ES_GRAIN);
		break;
	case OMA_CODECID_ATRAC3:
		switch (codecCtrl.at3.codecParam) {
		case 4:
			codecCtrl.inputEsSize = 384;
			break;
		case 6:
			codecCtrl.inputEsSize = 304;
			break;
		case 0xB:
			codecCtrl.inputEsSize = 192;
			break;
		case 0xE:
			return -1; //unsupported
			break;
		case 0xF:
			return -1; //unsupported
			break;
		}
		streamBuffer[0] = malloc(codecCtrl.inputEsSize * GROUP_SIZE);
		streamBuffer[1] = malloc(codecCtrl.inputEsSize * GROUP_SIZE);
		current_offset += sceIoPread(file_fd, streamBuffer[0], codecCtrl.inputEsSize * GROUP_SIZE, (SceOff)dataOffset);
		//Calculate total samples
		atDataSize = (SceSize)stat.st_size - dataOffset;
		totalEsSamples = atDataSize / codecCtrl.inputEsSize;
		vitaAudioPreSetGrain(AT3_ES_GRAIN);
		break;
	}

	current_offset += dataOffset;
	dataBeginOffset = dataOffset;
	bufindex = 0;

	return 0;
}

static int AT3_Init_OMA(const char *path) {

	SceUID fd;

	ID3Tag *ID3 = malloc(sizeof(ID3Tag));
	ParseID3(path, ID3);

	char* meta_ptr = (char *)ID3;

	for (int i = 0; i < 792; i++) {
		if (meta_ptr[i] != '\0') {
			metadata.has_meta = SCE_TRUE;
			break;
		}
	}

	if (metadata.has_meta) {
		sceClibStrncpy(metadata.title, ID3->ID3Title, 260);
		sceClibStrncpy(metadata.artist, ID3->ID3Artist, 260);
		sceClibStrncpy(metadata.album, ID3->ID3Album, 64);
		sceClibStrncpy(metadata.year, ID3->ID3Year, 5);
		sceClibStrncpy(metadata.genre, ID3->ID3GenreText, 64);
	}

	if (ID3->ID3EncapsulatedPictureType == JPEG_IMAGE) {
		Menu_UnloadExternalCover();
		fd = sceIoOpen(path, SCE_O_RDONLY, 0);
		if (fd >= 0) {
			char *buffer = malloc(ID3->ID3EncapsulatedPictureLength);
			if (buffer) {
				sceIoLseek32(fd, ID3->ID3EncapsulatedPictureOffset, SCE_SEEK_SET);
				sceIoRead(fd, buffer, ID3->ID3EncapsulatedPictureLength);
				sceIoClose(fd);

				vita2d_JPEG_ARM_decoder_initialize();
				metadata.cover_image = vita2d_load_JPEG_ARM_buffer(buffer, ID3->ID3EncapsulatedPictureLength, 0, 0, 0);
				vita2d_JPEG_ARM_decoder_finish();

				free(buffer);
			}
		}
	}
	else if (ID3->ID3EncapsulatedPictureType == PNG_IMAGE) {
		Menu_UnloadExternalCover();
		fd = sceIoOpen(path, SCE_O_RDONLY, 0);
		if (fd >= 0) {
			char *buffer = malloc(ID3->ID3EncapsulatedPictureLength);
			if (buffer) {
				sceIoLseek32(fd, ID3->ID3EncapsulatedPictureOffset, SCE_SEEK_SET);
				sceIoRead(fd, buffer, ID3->ID3EncapsulatedPictureLength);
				sceIoClose(fd);

				metadata.cover_image = vita2d_load_PNG_buffer(buffer);

				free(buffer);
			}
		}
	}

	SceSize offset = ID3v2TagSize(path);

	free(ID3);

	//Locate EA3 header
	int index = 0;
	char* ea3Header_init = malloc(1024);
	char* ea3Header = ea3Header_init;
	sceClibMemset(ea3Header, 0, 1024);

	fd = sceIoOpen(path, SCE_O_RDONLY, 0);
	sceIoPread(fd, ea3Header, 1024, (SceOff)offset);
	sceIoClose(fd);

	while (*ea3Header == '\0' && index < 1024) {
		index++;
		ea3Header++;
	}

	if (sceClibMemcmp(ea3Header, "EA3", 3))
		return -1;

	SceUInt8 codecType = ea3Header[32];
	SceUInt8 param1;
	SceUInt8 param2;

	if (codecType == OMA_CODECID_ATRAC3P) {
		param1 = ea3Header[34];
		param2 = ea3Header[35];
	}
	else if (codecType == OMA_CODECID_ATRAC3) {
		//Everything here is hardcoded: no ATRAC3 header documentation anywhere
		param1 = ea3Header[33];
		if (param1 == 2)
			param1 = 1;

		param2 = ea3Header[35];
		switch (param2) {
		case 0x18:
			param2 = 0x60;
			break;
		case 0x26:
			param2 = 0x98;
			break;
		case 0x30:
			param2 = 0xC0;
			break;
		default:
			return -1;
			break;
		}		
	}
	else
		return -1;

	free(ea3Header_init);
	return AT3_Init_common(path, codecType, param1, param2, offset + index + EA3_HEADER_SIZE);
}

static int AT3_Init_RIFF(const char *path) {

	//Locate AT3 info header
	SceSize index = 0;
	char* ea3Header = malloc(1024);
	char* dataStart = ea3Header;
	sceClibMemset(ea3Header, 0, 1024);

	SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
	sceIoRead(fd, ea3Header, 1024);
	sceIoClose(fd);

	//"data"
	while (*(int *)dataStart != 0x61746164 && index < 1024) {
		index++;
		dataStart++;
	}
	index += 8;

	SceUInt8 codecType = ea3Header[60];
	SceUInt8 param1;
	SceUInt8 param2;

	if (codecType == OMA_CODECID_ATRAC3P) {
		param1 = ea3Header[62];
		param2 = ea3Header[63];
	}
	else if (codecType == OMA_CODECID_ATRAC3) {
		//Everything here is hardcoded: no ATRAC3 header documentation anywhere
		param1 = ea3Header[61];
		if (param1 == 2)
			param1 = 1;

		param2 = ea3Header[63];
		switch (param2) {
		case 0x18:
			param2 = 0x60;
			break;
		case 0x26:
			param2 = 0x98;
			break;
		case 0x30:
			param2 = 0xC0;
			break;
		default:
			return -1;
			break;
		}
	}
	else
		return -1;

	free(ea3Header);
	return AT3_Init_common(path, codecType, param1, param2, index);
}

int AT3_Init(const char *path) {

	//Check ATRAC3 file header type
	char* ext = sceClibStrrchr(path, '.');
	if (!sceClibStrncasecmp(ext, ".oma", 4) || !sceClibStrncasecmp(ext, ".aa3", 4))
		return AT3_Init_OMA(path);
	else if (!sceClibStrncasecmp(ext, ".at3", 4))
		return AT3_Init_RIFF(path);
	else
		return -1;
}

SceUInt32 AT3_GetSampleRate(void) {
	return 44100;
}

SceUInt8 AT3_GetChannels(void) {
	//TODO: actually check from header
	return 2;
}

void AT3_Decode(void *buf, unsigned int length, void *userdata) {

	if (callCount == GROUP_SIZE) {
		bufindex ^= 1;
		readOffset = 0;
		callCount = 0;
	}

	//Wait until we are done with previous call, issue next read call
	if (callCount == 0) {
		if (sceKernelWaitEvent(opHandle, 1, NULL, NULL, NULL) == 0)
			sceIoComplete(opHandle);
		current_offset += asyncParam.result;
		sceClibMemset(&asyncParam, 0, sizeof(SceIoAsyncParam));
		bufindex ^= 1;
		switch (codecType) {
		case SCE_AUDIOCODEC_ATX:
			opHandle = sceIoPreadAsync(file_fd, streamBuffer[bufindex], codecCtrl.atx.inputEsSize * GROUP_SIZE, (SceOff)current_offset, &asyncParam);
			break;
		case SCE_AUDIOCODEC_AT3:
			opHandle = sceIoPreadAsync(file_fd, streamBuffer[bufindex], codecCtrl.inputEsSize * GROUP_SIZE, (SceOff)current_offset, &asyncParam);
			break;
		}
		bufindex ^= 1;
	}

	//Decode current ES samples

	codecCtrl.pEs = streamBuffer[bufindex] + readOffset;
	codecCtrl.pPcm = buf;
	if (sceAudiocodecDecode(&codecCtrl, codecType) < 0) {}
		//playing = SCE_FALSE;
	readOffset += codecCtrl.inputEsSize;

	callCount++;
	totalEsPlayed++;

	if (totalEsPlayed >= totalEsSamples)
		playing = SCE_FALSE;
}

SceUInt64 AT3_GetPosition(void) {
	switch (codecType) {
	case SCE_AUDIOCODEC_ATX:
		return totalEsPlayed * AT3P_ES_GRAIN;
		break;
	case SCE_AUDIOCODEC_AT3:
		return totalEsPlayed * AT3_ES_GRAIN;
		break;
	}

	return 0;
}

SceUInt64 AT3_GetLength(void) {
	switch (codecType) {
	case SCE_AUDIOCODEC_ATX:
		return totalEsSamples * AT3P_ES_GRAIN;
		break;
	case SCE_AUDIOCODEC_AT3:
		return totalEsSamples * AT3_ES_GRAIN;
		break;
	}

	return 0;
}

SceUInt64 AT3_Seek(SceUInt64 index) {
	
	SceSize esGrain = AT3P_ES_GRAIN;
	if (codecType == SCE_AUDIOCODEC_AT3)
		esGrain = AT3_ES_GRAIN;

	SceUInt64 seekSamples = ((totalEsSamples * esGrain) * (index / SEEK_WIDTH_FLOAT));
	SceUInt32 seekEsNum = (SceUInt32)floorf(seekSamples / esGrain);
	seekSamples = (SceUInt64)(seekEsNum * esGrain);

	totalEsPlayed = seekEsNum;
	bufindex = 0;
	readOffset = 0;
	callCount = 0;
	sceClibMemset(&asyncParam, 0, sizeof(SceIoAsyncParam));

	SceSize newOffset = dataBeginOffset + (seekEsNum * codecCtrl.inputEsSize);
	current_offset = newOffset;
	current_offset += sceIoPread(file_fd, streamBuffer[0], codecCtrl.inputEsSize * GROUP_SIZE, (SceOff)(newOffset));

	return seekSamples;
}

void AT3_Term(void) {

	if (sceKernelWaitEvent(opHandle, 1, NULL, NULL, NULL) == 0)
		sceIoComplete(opHandle);

	bufindex = 0;
	callCount = 0;
	readOffset = 0;
	current_offset = 0;
	totalEsSamples = 0;
	totalEsPlayed = 0;
	dataBeginOffset = 0;
	sceKernelFreeMemBlock(adecWorkMem);
	free(streamBuffer[0]);
	free(streamBuffer[1]);
	sceClibMemset(&codecCtrl, 0, sizeof(SceAudiocodecCtrl));
	sceClibMemset(&asyncParam, 0, sizeof(SceIoAsyncParam));

	if (metadata.has_meta)
		metadata.has_meta = SCE_FALSE;

	vitaAudioPreSetGrain(VITA_NUM_AUDIO_SAMPLES);
	sceIoClose(file_fd);
}
