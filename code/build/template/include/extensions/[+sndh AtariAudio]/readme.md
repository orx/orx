# Atari Audio Library

AtariAudio/ directory contains Atari Audio library. You can use it to play SNDH files or directly use YM2149 emulator if you want to write your own tracker.
The libray doesn't use any dependency, and should compile on any platform, including embeded systems (it doesn't even use float )

# Playing SNDH file in your own app

AtariAudio library doesn't use any file IO. You should provide data from memory. Entry point is class SndhFile.
Look at SndhFile.h for API details but here is the absolute minimal:

````
bool	Load(const void* rawSndhFile, int sndhFileSize, uint32_t hostReplayRate);
````
Load a raw SNDH file from memory. You should provide the memory buffer, size of the raw file, and host replay rate. ( ex 44100 for 44.1Khz )

````
bool	InitSubSong(int subSongId);
````
Atari SNDH musics could contain several subsongs. You should *always* call InitSubsong before any audio rendering function. By convention, subsongs starts at 1.

````
int		AudioRender(int16_t* buffer, int count, uint32_t* pSampleViewInfo = NULL);
````
This is the main audio rendering function. Render "count" samples into buffer. Buffer is a 16bits, signed, mono, sample buffer. pSampleViewInfo is optional buffer of count * uint32_t buffer. Could be used for oscilloscope viewer. More details in the source code.
Like, let's say your replay rate is 44.1Khz and you want to generate 1 second of music:

````
  int16_t* buffer = buffer of 44100*2 bytes ( one sample is 16bits, mono )
  AudioRender(buffer, 44100);
````

# Credits

- AtariAudio library written by Arnaud Carré aka Leonard/Oxygene.
- MUSASHI 68000 emulation written by Karl Stenerud
- Atari ICE depacker C version written by Hans Wessels


