#ifndef _ELEVENMPV_AUDIO_H_
#define _ELEVENMPV_AUDIO_H_

#include <paf.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <shellaudio.h>

#include "xmp.h"
#include "audiocodec.h"

using namespace paf;

typedef SceUInt64 drflac_uint64;
typedef SceInt64 ogg_int64_t;

namespace audio {

	class PlayerCoverLoaderThread : public thread::Thread
	{
	public:

		using thread::Thread::Thread;

		SceVoid EntryFunction();

		ScePVoid workptr;
		SceSize size;
		SceBool isExtMem;
		graphics::Texture coverTex;
	};

	class Utils
	{
	public:

		static SceVoid ResetBgmMode();
	};

	class GenericDecoder
	{
	public:

		class Metadata
		{
		public:

			SceBool hasMeta;
			SceBool hasCover;
			WString title;
			WString album;
			WString artist;
		};

		GenericDecoder(const char *path, SceBool isSwDecoderUsed);

		virtual ~GenericDecoder();

		virtual SceUInt64 Seek(SceFloat32 percent);

		virtual SceVoid Decode(ScePVoid stream, SceUInt32 length, ScePVoid userdata);

		virtual SceUInt32 GetSampleRate();

		virtual SceUInt8 GetChannels();

		virtual SceUInt64 GetPosition();

		virtual SceUInt64 GetLength();

		virtual SceBool IsPaused();

		virtual SceVoid Pause();

		virtual SceVoid Stop();

		virtual Metadata *GetMetadataLocation();

		SceBool isPlaying;
		SceBool isPaused;
		Metadata *metadata;
		PlayerCoverLoaderThread *coverLoader;
	};

	class FlacDecoder : public GenericDecoder
	{
	public:

		FlacDecoder(const char *path, SceBool isSwDecoderUsed);

		~FlacDecoder();

		static SceVoid MetadataCbEntry(ScePVoid pUserData, ScePVoid pMetadata);

		static SceSize drflacReadCB(ScePVoid pUserData, ScePVoid pBufferOut, SceSize bytesToRead);

		static SceUInt32 drflacSeekCB(ScePVoid pUserData, SceInt32 offset, SceInt32 origin);

		SceUInt64 Seek(SceFloat32 percent);

		SceVoid Decode(ScePVoid stream, SceUInt32 length, ScePVoid userdata);

		SceUInt32 GetSampleRate();

		SceUInt8 GetChannels();

		SceUInt64 GetPosition();

		SceUInt64 GetLength();

	private:

		ScePVoid flac;
		drflac_uint64 framesRead;
	};

	class OpusDecoder : public GenericDecoder
	{
	public:

		OpusDecoder(const char *path, SceBool isSwDecoderUsed);

		~OpusDecoder();

		SceUInt64 Seek(SceFloat32 percent);

		SceVoid Decode(ScePVoid stream, SceUInt32 length, ScePVoid userdata);

		SceUInt32 GetSampleRate();

		SceUInt8 GetChannels();

		SceUInt64 GetPosition();

		SceUInt64 GetLength();

	private:

		ScePVoid opus;
		ogg_int64_t samplesRead;
		ogg_int64_t	maxSamples;
	};

	class OggDecoder : public GenericDecoder
	{
	public:

		OggDecoder(const char *path, SceBool isSwDecoderUsed);

		~OggDecoder();

		SceUInt64 Seek(SceFloat32 percent);

		SceVoid Decode(ScePVoid stream, SceUInt32 length, ScePVoid userdata);

		SceUInt32 GetSampleRate();

		SceUInt8 GetChannels();

		SceUInt64 GetPosition();

		SceUInt64 GetLength();

	private:

		SceUInt64 FillBuffer(char *out);

		OggVorbis_File ogg;
		vorbis_info *oggInfo;
		ogg_int64_t samplesRead;
		ogg_int64_t oldSamplesRead;
		ogg_int64_t	maxLength;
	};

	class XmDecoder : public GenericDecoder
	{
	public:

		XmDecoder(const char *path, SceBool isSwDecoderUsed);

		~XmDecoder();

		SceUInt64 Seek(SceFloat32 percent);

		SceVoid Decode(ScePVoid stream, SceUInt32 length, ScePVoid userdata);

		SceUInt32 GetSampleRate();

		SceUInt8 GetChannels();

		SceUInt64 GetPosition();

		SceUInt64 GetLength();

	private:

		SceUInt64 samplesRead;
		SceUInt64 totalSamples;
		xmp_context xmp;
		xmp_frame_info frame_info;
		xmp_module_info module_info;
	};

	class At3Decoder : public GenericDecoder
	{
	public:

		enum Type
		{
			Type_AT3,
			Type_AT3P
		};

		At3Decoder(const char *path, SceBool isSwDecoderUsed);

		~At3Decoder();

		SceUInt64 Seek(SceFloat32 percent);

		SceVoid Decode(ScePVoid stream, SceUInt32 length, ScePVoid userdata);

		SceUInt32 GetSampleRate();

		SceUInt8 GetChannels();

		SceUInt64 GetPosition();

		SceUInt64 GetLength();

	private:

		static SceInt32 sceAudiocodecGetAt3ConfigPSP2(SceUInt32 cmode, SceUInt32 nbytes);

		SceVoid InitOMA(const char *path);

		SceVoid InitRIFF(const char *path);

		SceVoid InitCommon(const char *path, SceUInt8 type, SceUInt8 param1, SceUInt8 param2, SceSize dataOffset);

		SceAudiocodecCtrl codecCtrl;
		ScePVoid streamBuffer[2];
		io::File *at3File;
		SceSize dataBeginOffset;
		SceSize readOffset;
		SceSize currentOffset;
		SceUInt64 totalEsSamples;
		SceUInt64 totalEsPlayed;
		SceUInt32 codecType;
		SceUInt32 bufindex;
		SceUInt32 callCount;
	};

	class ShellCommonDecoder : public GenericDecoder
	{
	public:

		ShellCommonDecoder(const char *path, SceBool isSwDecoderUsed);

		~ShellCommonDecoder();

		SceUInt64 Seek(SceFloat32 percent);

		SceUInt32 GetSampleRate();

		SceUInt64 GetPosition();

		SceUInt64 GetLength();

	private:

		SceBool isSeeking;
		SceUInt32 timeRead;
		SceUInt32 totalTime;
		SceUInt32 seekFrame;
		SceMusicPlayerServicePlayStatusExtension pbStat;
	};

	class Mp3Decoder : public ShellCommonDecoder
	{
	public:

		Mp3Decoder(const char *path, SceBool isSwDecoderUsed);

	};
}

#endif
