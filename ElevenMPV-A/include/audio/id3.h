#ifndef _ELEVENMPV_AUDIO_ID3_H_
#define _ELEVENMPV_AUDIO_ID3_H_

#include <stdio.h>

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif	/* defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus) */

#define JPEG_IMAGE 1
#define PNG_IMAGE 2

#define ID3_JPEG (unsigned char [3]) \
                        { \
                        0xFF,0xD8,0xFF \
                        }

#define ID3_PNG (unsigned char [16]) \
                        { \
                        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52 \
                        }

	typedef struct ID3Tag {
		char   ID3Title[260];
		char   ID3Artist[260];
		char   ID3Album[260];
		char   ID3Year[12];
		char   ID3Comment[260];
		char   ID3GenreCode[12];
		char   ID3GenreText[260];
		char   versionfound[12];
		int    ID3Track;
		char   ID3TrackText[8];
		int    ID3EncapsulatedPictureType;
		int    ID3EncapsulatedPictureOffset; /* Offset address of an attached picture, NULL if no attached picture exists */
		int    ID3EncapsulatedPictureLength;
		int    ID3Length;
	} ID3Tag;

	int ID3v2TagSize(const char *mp3path);
	int ParseID3(const char *mp3path, struct ID3Tag *target);

	//Helper functions (used also by aa3mplayerME to read tags):
	void readTagData(int fp, int tagLength, int maxTagLength, char *tagValue);
	int swapInt32BigToHost(int arg);
	//short int swapInt16BigToHost(short int arg);

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif	/* defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus) */

#endif
