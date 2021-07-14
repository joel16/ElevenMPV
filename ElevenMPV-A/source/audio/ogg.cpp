#include <kernel.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "audio.h"
#include "common.h"
#include "vitaaudiolib.h"

audio::OggDecoder::OggDecoder(const char *path, SceBool isSwDecoderUsed) : GenericDecoder::GenericDecoder(path, isSwDecoderUsed)
{
	String *text8 = new String();

	if (ov_fopen(path, &ogg) < 0) {
		return;
	}

	if ((oggInfo = ov_info(&ogg, -1)) == SCE_NULL)
		return;

	maxLength = ov_pcm_total(&ogg, -1);

	vorbis_comment *comment = ov_comment(&ogg, -1);
	if (comment != SCE_NULL) {

		char *value = SCE_NULL;

		if ((value = vorbis_comment_query(comment, "title", 0)) != SCE_NULL) {
			text8->Set(value);
			text8->ToWString(&metadata->title);
			text8->Clear();
			metadata->hasMeta = SCE_TRUE;
		}

		if ((value = vorbis_comment_query(comment, "album", 0)) != SCE_NULL) {
			text8->Set(value);
			text8->ToWString(&metadata->album);
			text8->Clear();
			metadata->hasMeta = SCE_TRUE;
		}

		if ((value = vorbis_comment_query(comment, "artist", 0)) != SCE_NULL) {
			text8->Set(value);
			text8->ToWString(&metadata->artist);
			text8->Clear();
			metadata->hasMeta = SCE_TRUE;
		}
	}

	delete text8;

	audio::DecoderCore::SetDecoder(this, SCE_NULL);
	audio::DecoderCore::Init(GetSampleRate(), GetChannels() == 2 ? SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO : SCE_AUDIO_OUT_PARAM_FORMAT_S16_MONO);
}

SceUInt32 audio::OggDecoder::GetSampleRate()
{
	return oggInfo->rate;
}

SceUInt8 audio::OggDecoder::GetChannels()
{
	return oggInfo->channels;
}

SceUInt64 audio::OggDecoder::FillBuffer(char *out)
{
	SceUInt32 grain = audio::DecoderCore::GetGrain();
	SceInt32 samplesToRead = (sizeof(SceInt16) * oggInfo->channels) * grain;

	SceInt32 currentSection, samplesJustRead;

	while(samplesToRead > 0) {
		samplesJustRead = ov_read(&ogg, out, samplesToRead > grain ? grain : samplesToRead, 0, 2, 1, &currentSection);

		if (samplesJustRead < 0)
			return samplesJustRead;
		else if (samplesJustRead == 0)
			break;

		samplesToRead -= samplesJustRead;
		out += samplesJustRead;
	}

	return samplesJustRead;
}

SceVoid audio::OggDecoder::Decode(ScePVoid stream, SceUInt32 length, ScePVoid userdata)
{
	if (isPlaying == SCE_FALSE || isPaused == SCE_TRUE) {
		short *bufShort = (short *)stream;
		SceUInt32 count;
		for (count = 0; count < length * 2; count++)
			*(bufShort + count) = 0;
	}
	else {
		FillBuffer((char *)stream);

		samplesRead = ov_pcm_tell(&ogg);

		if (samplesRead >= maxLength)
			isPlaying = SCE_FALSE;
	}
}

SceUInt64 audio::OggDecoder::GetPosition()
{
	return samplesRead;
}

SceUInt64 audio::OggDecoder::GetLength()
{
	return maxLength;
}

SceUInt64 audio::OggDecoder::Seek(SceFloat32 percent)
{
	ogg_int64_t seekSample = (ogg_int64_t)((SceFloat32)maxLength * percent / 100.0f);
	
	if (ov_pcm_seek(&ogg, seekSample) >= 0) {
		samplesRead = seekSample;
		return samplesRead;
	}

	return -1;
}

audio::OggDecoder::~OggDecoder()
{
	isPlaying = SCE_TRUE;
	isPaused = SCE_FALSE;

	audio::DecoderCore::EndPre();
	thread::Thread::Sleep(100);
	audio::DecoderCore::End();

	ov_clear(&ogg);
}
