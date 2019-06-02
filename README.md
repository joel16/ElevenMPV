# ElevenMPV - Eleven Music Player VITA

A homebrew music player for Playstation VITA that aims to support many different audio formats compared to the offical PS VITA music application.


# Currently supported formats: (16 bit signed samples)
- FLAC
- IT
- MOD
- MP3 
- OGG
- OPUS
- S3M
- WAV (A-law and u-law, Microsoft ADPCM, IMA ADPCM)
- XM


# Features:
- Browse ux0:/, ur0:/ and uma0:/ to play the above audio formats.
- Pause/Play audio.
- Shuffle/Repeat audio.
- Next/Previous track in current working directory.
- Display ID3v1 and ID3v2 metadata for MP3 files. Other tags are displayed for OGG, FLAC, OPUS and XM.
- Basic touch support.
- Seeking support using touch screen. (No support for OPUS)


# Controls:
**In file manager:**

- Enter button (cross/circle): enter folder/play supported audio file.
- Cancel button (cross/circle): go up parent folder.
- DPAD Up/Down: Navigate files.
- DPAD Left/Right: Top/Bottom of list.

**In audio player: (Note: you can use touch controls here or the following buttons below)**

- Enter button (cross/circle): Play/Pause.
- Cancel button (cross/circle): Return to file manager.
- L trigger: Previous audio file in current directory.
- R trigger: Next audio file in current directory.
- Triangle: Shuffle audio files in current directory.
- Square: Repeat audio files in current directory.
- Start: Turn off display and keep playing audio in background.
- Touch: Touch anywhere on the progress bar to seek to that location.


# Credits:
- MPG123 contributors.
- dr_libs by mackron.
- libvorbis, libogg and libopus contributors.
- libxmp-lite contributors.
- Preetisketch for startup.png (banner).
- LineageOS's Eleven Music Player contributors for design elements.
