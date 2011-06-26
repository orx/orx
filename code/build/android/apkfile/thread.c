#include <jni.h>

#include "thread.h"
#include <android/log.h>
#include <pthread.h>

static JavaVM* s_vm = NULL;
static pthread_key_t s_jniEnvKey = 0;

#define MODULE "Thread"

void ThreadInit(JavaVM* vm)
{
	s_vm = vm;
}

JNIEnv* ThreadGetCurrentJNIEnv()
{
	JNIEnv* env = NULL;
    if (s_jniEnvKey)
	{
		env = (JNIEnv*)pthread_getspecific(s_jniEnvKey);
	}
	else
	{
		pthread_key_create(&s_jniEnvKey, NULL);
	}

	if (!env)
	{
		// do we have a VM cached?
		if (!s_vm)
		{
			__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error - could not find JVM!");
			return NULL;
		}

		// Hmm - no env for this thread cached yet
		int error = (*s_vm)->AttachCurrentThread(s_vm, &env, NULL);
		__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "AttachCurrentThread: %d, 0x%p", error, env);
		if (error || !env)
		{
			__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error - could not attach thread to JVM!");
			return NULL;
		}

		pthread_setspecific(s_jniEnvKey, env);
	}

	return env;
}

typedef struct ThreadInitStruct
{
	void* m_arg;
	void *(*m_startRoutine)(void *);
} ThreadInitStruct;

static void* ThreadSpawnProc(void* arg)
{
	ThreadInitStruct* init = (ThreadInitStruct*)arg;
	void *(*start_routine)(void *) = init->m_startRoutine;
	void* data = init->m_arg;
	void* ret;

	free(arg);

	ThreadGetCurrentJNIEnv();

	ret = start_routine(data);

	if (s_vm)
		(*s_vm)->DetachCurrentThread(s_vm);

	return ret;
}

int ThreadSpawnJNIThread(pthread_t *thread, pthread_attr_t const * attr,
    void *(*start_routine)(void *), void * arg)
{
	if (!start_routine)
		return -1;

	ThreadInitStruct* initData = malloc(sizeof(ThreadInitStruct));

	initData->m_startRoutine = start_routine;
	initData->m_arg = arg;

	int err = pthread_create(thread, attr, ThreadSpawnProc, initData);

	// If the thread was not started, then we need to delete the init data ourselves
	if (err)
	{
		free(initData);
	}

	return err;
}

// on linuces, signals can interrupt sleep functions, so you might need to 
// retry to get the full sleep going. I'm not entirely sure this is necessary
// *here* clients could retry themselves when the exposed function returns
// nonzero
inline int __sleep(const struct timespec *req, struct timespec *rem)
{
	int ret = 1;
	int i;
	static const int sleepTries = 2;

	struct timespec req_tmp={0}, rem_tmp={0};

	rem_tmp = *req;
	for(i = 0; i < sleepTries; ++i)
	{
		req_tmp = rem_tmp;
		int ret = nanosleep(&req_tmp, &rem_tmp);
		if(ret == 0)
		{
			ret = 0;
			break;
		}
	}
	if(rem)
		*rem = rem_tmp;
	return ret;
}

int ThreadSleep(unsigned long millisec)
{
    struct timespec req={0},rem={0};
    time_t sec  =(int)(millisec/1000);

    millisec     = millisec-(sec*1000);
    req.tv_sec  = sec;
    req.tv_nsec = millisec*1000000L;
    return __sleep(&req,&rem);
}

