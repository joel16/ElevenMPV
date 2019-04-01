#include "audio.h"
#include "common.h"
#include "fs.h"
#define JAR_XM_IMPLEMENTATION
#include "jar_xm.h"

static jar_xm_context_t *xm;
static SceUInt64 samples_read = 0, total_samples = 0;
static char *data = NULL;

int XM_Init(const char *path) {
	int ret = 0;
	char *xm_data = NULL;
	SceUInt64 xm_size_bytes = 0;

    if (R_FAILED(ret = FS_GetFileSize(path, (SceOff *)&xm_size_bytes)))
		return ret;

	xm_data = malloc(xm_size_bytes);
	if (R_FAILED(ret = FS_ReadFile(path, xm_data, xm_size_bytes)))
		return ret;

	jar_xm_create_context_safe(&xm, xm_data, (size_t)xm_size_bytes, (SceUInt32)48000);
	total_samples = jar_xm_get_remaining_samples(xm);

    return 0;
}

SceUInt32 XM_GetSampleRate(void) {
	return 48000;
}

SceUInt8 XM_GetChannels(void) {
	return 2;
}

void XM_Decode(void *buf, unsigned int length, void *userdata) {
    jar_xm_generate_samples_16bit(xm, (short *)buf, (size_t)length);
    jar_xm_get_position(xm, NULL, NULL, NULL, &samples_read);

    if (samples_read == total_samples)
        playing = SCE_FALSE;
}

SceUInt64 XM_GetPosition(void) {
    return samples_read;
}

SceUInt64 XM_GetLength(void) {
    return total_samples;
}

void XM_Term(void) {
    samples_read = 0;
    jar_xm_free_context(xm);
    free(data);
}
