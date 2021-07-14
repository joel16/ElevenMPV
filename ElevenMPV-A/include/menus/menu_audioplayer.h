#ifndef _ELEVENMPV_MENU_PLAYAUDIO_H_
#define _ELEVENMPV_MENU_PLAYAUDIO_H_

#include <paf.h>

#include "audio.h"
#include "menu_displayfiles.h"

using namespace paf;

namespace menu {
	namespace audioplayer {

		class PlayerButtonCB : public widget::Widget::EventCallback
		{
		public:

			enum ButtonHash
			{
				ButtonHash_Play = 0x22535800,
				ButtonHash_Rew = 0x8ff51fc8,
				ButtonHash_Ff = 0x373fc526,
				ButtonHash_Repeat = 0x28bfa2c9,
				ButtonHash_Shuffle = 0x46be756f,
				ButtonHash_Progressbar = 0x354adaae
			};

			PlayerButtonCB()
			{
				eventHandler = PlayerButtonCBFun;
			};

			virtual ~PlayerButtonCB()
			{

			};

			static SceVoid PlayerButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};

		class BackButtonCB : public widget::Widget::EventCallback
		{
		public:

			BackButtonCB()
			{
				eventHandler = BackButtonCBFun;
			};

			virtual ~BackButtonCB()
			{

			};

			static SceVoid BackButtonCBFun(SceInt32 eventId, widget::Widget *self, SceInt32 a3, ScePVoid pUserData);

		};

		class AudioplayerCore
		{
		public:

			AudioplayerCore(const char *file);

			~AudioplayerCore();

			audio::GenericDecoder *GetDecoder();

			SceVoid SetInitialParams();

			SceVoid SetMetadata(const char *file);

		private:

			audio::GenericDecoder *decoder;
		};

		class Audioplayer 
		{
		public:

			class Playlist
			{
			public:

				String *path[1024];
				SceUInt8 isConsumed[1024];
			};

			Audioplayer(const char *cwd, menu::displayfiles::File *startFile);

			~Audioplayer();

			static SceVoid RegularTask(ScePVoid pUserData);

			static SceVoid PowerOffTask(ScePVoid pUserData);

			static SceVoid HandleNext(SceBool fromHandlePrev, SceBool fromFfButton);

			static SceVoid HandlePrev();

			static SceVoid ConvertSecondsToString(String *string, SceUInt64 seconds, SceBool needSeparator);
			
			SceVoid GetMusicList(menu::displayfiles::File *startFile);

			AudioplayerCore *GetCore();

		private:

			AudioplayerCore *core;
			Playlist playlist;
			SceInt32 playlistIdx;
			SceUInt32 startIdx;
			SceUInt32 totalIdx;
			SceUInt32 totalConsumedIdx;
		};

	}
}

/*void Menu_PlayAudio(char *path);
void Menu_UnloadExternalCover(void);*/

#endif
