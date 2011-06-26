#ifndef __INCLUDED_APK_FILE_H
#define __INCLUDED_APK_FILE_H

#include <stdlib.h>

#if defined(__cplusplus)
extern "C"
{
#endif

/** @file apk_file.h
  <b>APK is now considered internal - File in file.h should be used
  by applications.</b>

  This library supports FILE*-like access to assets in the APK file.  These
  APIs should not be used directly.  Use the File APIs in file.h, which
  include all of this library's functionalities and support files from APK
  and from /data.
  @see nv_file.h
*/

typedef void APKFile;



/**
  Initializes the library.  This function MUST be called from the application's
  JNI_OnLoad, from a function known to be called by JNI_OnLoad, or from a function
  in a Java-called thread.  nv_thread-created native threads cannot call this
  initialization function.
  */
void        APKInit();

APKFile*    APKOpen(char const* path);
void        APKClose(APKFile* file);
int         APKGetc(APKFile *stream);
char*       APKGets(char* s, int size, APKFile* stream);
size_t      APKSize(APKFile* stream);
long        APKSeek(APKFile* stream, long offset, int type);
long        APKTell(APKFile* stream);
size_t      APKRead(void* ptr, size_t size, size_t nmemb, APKFile* stream);
int         APKEOF(APKFile *stream);

#if defined(__cplusplus)
}
#endif

#endif
