#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>

#include "apk_file.h"
#include "thread.h"

static jobject s_globalThiz;
static jclass APKFileClass;
static jclass fileHelper;
static jmethodID s_openFile;
static jmethodID s_closeFile;
static jfieldID s_lengthId;
static jmethodID s_seekFile;
static jfieldID s_positionId;
static jmethodID s_readFile;
static jfieldID s_dataId;

void APKInit()
{
    __android_log_print(ANDROID_LOG_DEBUG, "apk",  "apk init\n");
	JNIEnv *env = ThreadGetCurrentJNIEnv();
    __android_log_print(ANDROID_LOG_DEBUG, "apk",  "env = %p\n", env);
	fileHelper = (*env)->FindClass(env, "org/orx/android/APKFileHelper");
    __android_log_print(ANDROID_LOG_DEBUG, "apk",  "class = %d\n", fileHelper);
	jmethodID getInstance = (*env)->GetStaticMethodID(env, fileHelper, "getInstance", "()Lorg/orx/android/APKFileHelper;");
    __android_log_print(ANDROID_LOG_DEBUG, "apk",  "inst = %d\n", getInstance);
	APKFileClass = (*env)->FindClass(env, "org/orx/android/APKFileHelper$APKFile");
	
    s_openFile = (*env)->GetMethodID(env, fileHelper, "openFileAndroid", "(Ljava/lang/String;)Lorg/orx/android/APKFileHelper$APKFile;");
    s_closeFile = (*env)->GetMethodID(env, fileHelper, "closeFileAndroid", "(Lorg/orx/android/APKFileHelper$APKFile;)V");
    s_lengthId = (*env)->GetFieldID(env, APKFileClass, "length", "I");
    s_seekFile = (*env)->GetMethodID(env, fileHelper, "seekFileAndroid", "(Lorg/orx/android/APKFileHelper$APKFile;I)J");
    s_positionId = (*env)->GetFieldID(env, APKFileClass, "position", "I");
    s_readFile = (*env)->GetMethodID(env, fileHelper, "readFileAndroid", "(Lorg/orx/android/APKFileHelper$APKFile;I)V");
    s_dataId = (*env)->GetFieldID(env, APKFileClass, "data", "[B");

	jobject thiz = (*env)->CallStaticObjectMethod(env, fileHelper, getInstance);
	s_globalThiz = (*env)->NewGlobalRef(env, thiz);
}

APKFile* APKOpen(char const* path)
{
	JNIEnv *env = ThreadGetCurrentJNIEnv();
    jstring test = (*env)->NewStringUTF(env, path);
    jobject fileHandle = (*env)->CallObjectMethod(env, s_globalThiz, s_openFile, test);
    (*env)->DeleteLocalRef(env, (jobject)test);

    return (APKFile *) fileHandle;
}

void APKClose(APKFile* file)
{
	JNIEnv *env = ThreadGetCurrentJNIEnv();
    (*env)->CallVoidMethod(env, s_globalThiz, s_closeFile, (jobject) file);
    (*env)->DeleteLocalRef(env, (jobject)file);
}

int APKGetc(APKFile *stream)
{
    char buff;
    APKRead(&buff,1,1,stream);
    return buff;
}

char* APKGets(char* s, int size, APKFile* stream)
{
    int i;
    char *d=s;
    for(i = 0; (size > 1) && (!APKEOF(stream)); i++, size--, d++)
    {
        APKRead(d,1,1,stream);
        if(*d==10)
        {
            size=1;
        }
    }
    *d=0;

    return s;
}

size_t APKSize(APKFile* stream)
{
	JNIEnv *env = ThreadGetCurrentJNIEnv();
    jint len = (*env)->GetIntField(env, (jobject) stream, s_lengthId);

    return len;
}

long APKSeek(APKFile* stream, long offset, int type)
{
	JNIEnv *env = ThreadGetCurrentJNIEnv();
	switch (type)
	{
	    case SEEK_CUR:
	        offset += APKTell(stream);
	        break;
	    case SEEK_END:
	        offset += APKSize(stream);
            break;
	    case SEEK_SET:
	        // No need to change the offset..
	        break;
	}

    jlong ret = (*env)->CallLongMethod(env, s_globalThiz, s_seekFile, (jobject) stream, (jint) offset);

    return ret;
}

long APKTell(APKFile* stream)
{
	JNIEnv *env = ThreadGetCurrentJNIEnv();
    jint pos = (*env)->GetIntField(env, (jobject) stream, s_positionId);

    return pos;
}

size_t APKRead(void* ptr, size_t size, size_t nmemb, APKFile* stream)
{
	JNIEnv *env = ThreadGetCurrentJNIEnv();
    jint readLength = size*nmemb;

    int avail = APKSize(stream)-APKTell(stream);
    if(readLength>avail)
    {
        readLength = avail;
        nmemb = readLength/size;
    }

    (*env)->CallVoidMethod(env, s_globalThiz, s_readFile, (jobject) stream, (jint) readLength);

    jbyteArray data = (jbyteArray) (*env)->GetObjectField(env, (jobject) stream, s_dataId);
    char *data2 = (char *) (*env)->GetByteArrayElements(env, data, NULL);
    memcpy(ptr,data2,readLength);
    (*env)->ReleaseByteArrayElements(env, data, (jbyte*) data2, 0);

    (*env)->DeleteLocalRef(env, data);

    return nmemb;
}

int APKEOF(APKFile *stream)
{
    int rv = (APKTell(stream) >= APKSize(stream)) ? 1 : 0;
    return rv;
}
