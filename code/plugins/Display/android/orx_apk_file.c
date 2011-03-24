//----------------------------------------------------------------------------------
// File:            libs\jni\nv_apk_file\nv_apk_file.c
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

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>

#include "orx_apk_file.h"
//#include "../nv_thread/nv_thread.h"

static jobject s_globalThiz;
static jclass AnOrxAPKFileClass;
static jclass fileHelper;
static jmethodID s_openFile;
static jmethodID s_closeFile;
static jfieldID s_lengthId;
static jmethodID s_seekFile;
static jfieldID s_positionId;
static jmethodID s_readFile;
static jfieldID s_dataId;

#define ORX_JAVA_PACKAGE_PATH org_orxproject_lib_AnOrxActivity
#define ORX_JAVA_APKFILE_PATH org/orxproject/lib/

#ifndef ORX_JAVA_PACKAGE_PATH
#error You have to define ORX_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_orx_example"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,ORX_JAVA_PACKAGE_PATH)

#define APPENDNAME(suffix, package)     package##suffix
#define APPENDNAME2(preffix,suffix, package)     prefix##package##suffix
#define _STR(s)     #s
#define STR(s)      _STR(s)

extern JNIEnv *mEnv;
extern JavaVM *mVM;

void OrxAPKInit() {
	JNIEnv *env = mEnv;
	fileHelper = (*env)->FindClass(env, "org/orxproject/lib/AnOrxAPKFileHelper");
	jmethodID getInstance = (*env)->GetStaticMethodID(env, fileHelper,
			"getInstance", "()Lorg/orxproject/lib/AnOrxAPKFileHelper;");
	AnOrxAPKFileClass
			= (*env)->FindClass(
					env,"org/orxproject/lib/AnOrxAPKFileHelper$AnOrxAPKFile");

	s_openFile
			= (*env)->GetMethodID(env, fileHelper, "openFileAndroid",
					"(Ljava/lang/String;)Lorg/orxproject/lib/AnOrxAPKFileHelper$AnOrxAPKFile;");
	s_closeFile = (*env)->GetMethodID(env, fileHelper, "closeFileAndroid",
			"(Lorg/orxproject/lib/AnOrxAPKFileHelper$AnOrxAPKFile;)V");
	s_lengthId = (*env)->GetFieldID(env, AnOrxAPKFileClass, "length", "I");
	s_seekFile = (*env)->GetMethodID(env, fileHelper, "seekFileAndroid",
			"(Lorg/orxproject/lib/AnOrxAPKFileHelper$AnOrxAPKFile;I)J");
	s_positionId = (*env)->GetFieldID(env, AnOrxAPKFileClass, "position", "I");
	s_readFile = (*env)->GetMethodID(env, fileHelper, "readFileAndroid",
			"(Lorg/orxproject/lib/AnOrxAPKFileHelper$AnOrxAPKFile;I)V");
	s_dataId = (*env)->GetFieldID(env, AnOrxAPKFileClass, "data", "[B");

	jobject thiz = (*env)->CallStaticObjectMethod(env, fileHelper, getInstance);
	s_globalThiz = (*env)->NewGlobalRef(env, thiz);
}

OrxAPKFile* OrxAPKOpen(char const* path) {
	JNIEnv *env = mEnv;
	jstring test = (*env)->NewStringUTF(env, path);
	jobject fileHandle = (*env)->CallObjectMethod(env, s_globalThiz,
			s_openFile, test);
	(*env)->DeleteLocalRef(env, (jobject) test);

	return (OrxAPKFile *) fileHandle;
}

void OrxAPKClose(OrxAPKFile* file) {
	JNIEnv *env = mEnv;
	(*env)->CallVoidMethod(env, s_globalThiz, s_closeFile, (jobject) file);
	(*env)->DeleteLocalRef(env, (jobject) file);
}

int OrxAPKGetc(OrxAPKFile *stream) {
	char buff;
	OrxAPKRead(&buff, 1, 1, stream);
	return buff;
}

char* OrxAPKGets(char* s, int size, OrxAPKFile* stream) {
	int i;
	char *d = s;
	for (i = 0; (size > 1) && (!OrxAPKEOF(stream)); i++, size--, d++) {
		OrxAPKRead(d, 1, 1, stream);
		if (*d == 10) {
			size = 1;
		}
	}
	*d = 0;

	return s;
}

size_t OrxAPKSize(OrxAPKFile* stream) {
	JNIEnv *env = mEnv;
	jint len = (*env)->GetIntField(env, (jobject) stream, s_lengthId);

	return len;
}

long OrxAPKSeek(OrxAPKFile* stream, long offset, int type) {
	JNIEnv *env = mEnv;
	switch (type) {
	case SEEK_CUR:
		offset += OrxAPKTell(stream);
		break;
	case SEEK_END:
		offset += OrxAPKSize(stream);
		break;
	case SEEK_SET:
		// No need to change the offset..
		break;
	}

	jlong ret = (*env)->CallLongMethod(env, s_globalThiz, s_seekFile,
			(jobject) stream, (jint) offset);

	return ret;
}

long OrxAPKTell(OrxAPKFile* stream) {
	JNIEnv *env = mEnv;
	jint pos = (*env)->GetIntField(env, (jobject) stream, s_positionId);

	return pos;
}

size_t OrxAPKRead(void* ptr, size_t size, size_t nmemb, OrxAPKFile* stream) {
	JNIEnv *env = mEnv;
	jint readLength = size * nmemb;

	int avail = OrxAPKSize(stream) - OrxAPKTell(stream);
	if (readLength > avail) {
		readLength = avail;
		nmemb = readLength / size;
	}

	(*env)->CallVoidMethod(env, s_globalThiz, s_readFile, (jobject) stream,
			(jint) readLength);

	jbyteArray data = (jbyteArray)(*env)->GetObjectField(env, (jobject) stream,
			s_dataId);
	char *data2 = (char *) (*env)->GetByteArrayElements(env, data, NULL);
	memcpy(ptr, data2, readLength);
	(*env)->ReleaseByteArrayElements(env, data, (jbyte*) data2, 0);

	(*env)->DeleteLocalRef(env, data);

	return nmemb;
}

int OrxAPKEOF(OrxAPKFile *stream) {
	int rv = (OrxAPKTell(stream) >= OrxAPKSize(stream)) ? 1 : 0;
	return rv;
}
