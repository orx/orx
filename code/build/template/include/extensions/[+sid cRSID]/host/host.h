#ifndef LIBCRSID_HEADER__HOST
#define LIBCRSID_HEADER__HOST //used  to prevent double inclusion of this header-file


#include <stdint.h>


enum cRSID_Specifications__File { CRSID_FILEVERSION_WEBSID=0x4E, CRSID_SECONDS_PER_MINUTE = 60 };

enum cRSID_File_Limitations {
 CRSID_FILESIZE_MAX = 100000,
};


// host/file.c, md5.c
#ifdef CRSID_PLATFORM_PC
char*              cRSID_fileNameOnly      (char* path);
char*              cRSID_folderNameOnly    (char* path);
char               cRSID_compareFileExtension (char *filename, char *extension);
char               cRSID_playableExtension (char *name);
int                cRSID_loadSIDfile (unsigned char* SIDfileData, char* filename, int maxlen); //load SID-file to a memory location (and return size)

void cRSID_MD5_calculateSum (uint8_t* input, int size, uint8_t* result);
#endif

// host/audio.c
#ifdef CRSID_PLATFORM_PC
void*              cRSID_initSound     (unsigned short samplerate, unsigned short buflen);
void               cRSID_startSound    (void);
void               cRSID_stopSound     (void);
void               cRSID_closeSound    (void);
void               cRSID_generateSound (unsigned char* buf, unsigned short len);
#endif


#endif //LIBCRSID_HEADER__HOST