/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */
#ifndef __INCLUDED_NV_APK_FILE_H
#define __INCLUDED_NV_APK_FILE_H

#include <stdlib.h>

#if defined(__cplusplus)
extern "C"
{
#endif

/** @file nv_apk_file.h
  <b>NvAPK is now considered internal - NvFile in nv_file.h should be used
  by applications.</b>

  This library supports FILE*-like access to assets in the APK file.  These
  APIs should not be used directly.  Use the NvFile APIs in nv_file.h, which
  include all of this library's functionalities and support files from APK
  and from /data.
  @see nv_file.h
*/

typedef void OrxAPKFile;



/**
  Initializes the library.  This function MUST be called from the application's
  JNI_OnLoad, from a function known to be called by JNI_OnLoad, or from a function
  in a Java-called thread.  nv_thread-created native threads cannot call this
  initialization function.
  */
void        OrxAPKInit();

OrxAPKFile*  OrxAPKOpen(char const* path);
void        OrxAPKClose(OrxAPKFile* file);
int         OrxAPKGetc(OrxAPKFile *stream);
char*       OrxAPKGets(char* s, int size, OrxAPKFile* stream);
size_t      OrxAPKSize(OrxAPKFile* stream);
long        OrxAPKSeek(OrxAPKFile* stream, long offset, int type);
long        OrxAPKTell(OrxAPKFile* stream);
size_t      OrxAPKRead(void* ptr, size_t size, size_t nmemb, OrxAPKFile* stream);
int         OrxAPKEOF(OrxAPKFile *stream);

#if defined(__cplusplus)
}
#endif

#endif
