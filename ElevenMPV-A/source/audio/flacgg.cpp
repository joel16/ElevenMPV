#include <kernel.h>

#include "vitaaudiolib.h"
#include "audio.h"
#include "config.h"
#include "dr_flac.h"
#include "menu_audioplayer.h"

static audio::FlacDecoder *s_currentDecoderInstance = SCE_NULL;

SceVoid audio::FlacDecoder::MetadataCbEntry(ScePVoid pUserData, ScePVoid pMeta)
{
	String *text8 = new String();
	drflac_metadata *pMetadata = (drflac_metadata *)pMeta;
	s_currentDecoderInstance->metadata->hasMeta = SCE_TRUE;
	if (pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_PICTURE) {
		if (pMetadata->data.picture.type == DRFLAC_PICTURE_TYPE_COVER_FRONT) {
			if (!sce_paf_strncasecmp("image/jpg", pMetadata->data.picture.mime, 9) ||
				!sce_paf_strncasecmp("image/jpeg", pMetadata->data.picture.mime, 10) ||
				!sce_paf_strncasecmp("image/png", pMetadata->data.picture.mime, 9)) {

				s_currentDecoderInstance->coverLoader = new audio::PlayerCoverLoaderThread(SCE_KERNEL_COMMON_QUEUE_HIGHEST_PRIORITY, 0x1000, "EMPVA::PlayerCoverLoader");
				s_currentDecoderInstance->coverLoader->workptr = sce_paf_malloc(pMetadata->data.picture.pictureDataSize);
				if (s_currentDecoderInstance->coverLoader->workptr != SCE_NULL) {
					s_currentDecoderInstance->coverLoader->isExtMem = SCE_TRUE;
					sce_paf_memcpy(s_currentDecoderInstance->coverLoader->workptr, pMetadata->data.picture.pPictureData, pMetadata->data.picture.pictureDataSize);
					s_currentDecoderInstance->coverLoader->size = pMetadata->data.picture.pictureDataSize;
					s_currentDecoderInstance->coverLoader->Start();

					s_currentDecoderInstance->metadata->hasMeta = SCE_TRUE;
					s_currentDecoderInstance->metadata->hasCover = SCE_TRUE;
				}
			}
		}
	}
	else if (pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT) {
		unsigned int len;
		drflac_vorbis_comment_iterator iter;
		drflac_init_vorbis_comment_iterator(&iter, pMetadata->data.vorbis_comment.commentCount, pMetadata->data.vorbis_comment.pComments);
		for (int i = 0; i < pMetadata->data.vorbis_comment.commentCount; i++) {
			const char* tag = drflac_next_vorbis_comment(&iter, &len);

			if (!sce_paf_strncasecmp("TITLE=", tag, 6)) {
				s_currentDecoderInstance->metadata->hasMeta = SCE_TRUE;
				text8->Set(tag + 6, len - 6);
				text8->ToWString(&s_currentDecoderInstance->metadata->title);
				text8->Clear();
			}

			if (!sce_paf_strncasecmp("ALBUM=", tag, 6)) {
				s_currentDecoderInstance->metadata->hasMeta = SCE_TRUE;
				text8->Set(tag + 6, len - 6);
				text8->ToWString(&s_currentDecoderInstance->metadata->album);
				text8->Clear();
			}

			if (!sce_paf_strncasecmp("ARTIST=", tag, 7)) {
				s_currentDecoderInstance->metadata->hasMeta = SCE_TRUE;
				text8->Set(tag + 7, len - 7);
				text8->ToWString(&s_currentDecoderInstance->metadata->artist);
				text8->Clear();
			}
		}
	}

	delete text8;
}

audio::FlacDecoder::FlacDecoder(const char *path, SceBool isSwDecoderUsed) : GenericDecoder::GenericDecoder(path, isSwDecoderUsed)
{
	framesRead = 0;
	metadata = new audio::FlacDecoder::Metadata();
	s_currentDecoderInstance = this;

	//SceLibc is the fastest sync IO available on Vita, so we just use stdio here
	flac = (ScePVoid)drflac_open_file_with_metadata(path, (drflac_meta_proc)MetadataCbEntry, SCE_NULL, SCE_NULL);
	audio::DecoderCore::Init(GetSampleRate(), GetChannels() == 2 ? SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO : SCE_AUDIO_OUT_PARAM_FORMAT_S16_MONO);
	audio::DecoderCore::SetDecoder(this, SCE_NULL);
}

SceUInt32 audio::FlacDecoder::GetSampleRate()
{
	drflac *flacHandle = (drflac *)flac;
	return flacHandle->sampleRate;
}

SceUInt8 audio::FlacDecoder::GetChannels()
{
	drflac *flacHandle = (drflac *)flac;
	return flacHandle->channels;
}

SceVoid audio::FlacDecoder::Decode(ScePVoid stream, SceUInt32 length, ScePVoid userdata)
{
	drflac *flacHandle = (drflac *)flac;

	if (isPlaying == SCE_FALSE || isPaused == SCE_TRUE) {
		short *bufShort = (short *)stream;
		SceUInt32 count;
		for (count = 0; count < length * 2; count++)
			*(bufShort + count) = 0;
	}
	else {
		framesRead += drflac_read_pcm_frames_s16(flacHandle, (drflac_uint64)length, (drflac_int16 *)stream);

		if (framesRead >= flacHandle->totalPCMFrameCount)
			isPlaying = SCE_FALSE;
	}
}

SceUInt64 audio::FlacDecoder::GetPosition()
{
	return framesRead;
}

SceUInt64 audio::FlacDecoder::GetLength()
{
	drflac *flacHandle = (drflac *)flac;

	return flacHandle->totalPCMFrameCount;
}

SceUInt64 audio::FlacDecoder::Seek(SceFloat32 percent)
{
	drflac *flacHandle = (drflac *)flac;

	drflac_uint64 seekFrame = (drflac_uint64)((SceFloat32)flacHandle->totalPCMFrameCount * percent / 100.0f);

	if (drflac_seek_to_pcm_frame(flacHandle, seekFrame) == DRFLAC_TRUE) {
		framesRead = seekFrame;
		return framesRead;
	}

	return -1;
}

audio::FlacDecoder::~FlacDecoder()
{
	/*if (metadata.has_meta)
		metadata.has_meta = SCE_FALSE;*/ // TODO

	drflac_close((drflac *)flac);
}
