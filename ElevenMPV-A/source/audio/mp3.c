#include <kernel.h>
#include <appmgr.h>
#include <shellaudio.h>
#include <stdlib.h>

#include "audio.h"
#include "config.h"
#include "touch.h"
#include "id3.h"
#include "utils.h"
#include "menu_audioplayer.h"

extern SceMusicPlayerServicePlayStatusExtension pb_stats;
extern SceBool ext_cover_loaded;
extern unsigned int total_time;

int MP3_Init(char *path) {

	sceClibMemset(&pb_stats, 0, sizeof(SceMusicPlayerServicePlayStatusExtension));

	int error = sceMusicPlayerServiceInitialize(0);
	if (error < 0)
		return error;

	error = sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_STOP, 0);
	if (error < 0)
		return error;

	error = sceMusicPlayerServiceOpen(path, NULL);
	if (error < 0)
		return error;



	//Wait until SceShell is ready, but use that time to load metadata
	sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_PLAY, 0);
	pb_stats.currentState = SCE_MUSICCORE_EVENTID_STOP;

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
		SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
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
		SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
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

	free(ID3);

	//Wait until SceShell is ready
	while (pb_stats.currentState == SCE_MUSICCORE_EVENTID_STOP) {
		sceMusicPlayerServiceGetPlayStatusExtension(&pb_stats);
	}

	SceMusicPlayerServiceTrackInfo data;
	sceMusicPlayerServiceGetTrackInfo(&data);
	total_time = data.duration;

	return 0;
}
