//----------------------------------------------------------------------------------
// File:            libs\jni\nv_apk_file\nv_apk_file.h
// Samples Version: Android NVIDIA samples 2 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2009-2010 NVIDIA� Corporation 
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------
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
