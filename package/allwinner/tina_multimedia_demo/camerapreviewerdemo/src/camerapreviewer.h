#ifdef __cplusplus
extern "C"{
#endif
#ifndef _CAMERAPREVIEWER_H_
#define _CAMERAPREVIEWER_H_

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Trecorder.h>
#include <videoOutPort.h>

#define NONE "\e[0m"
#define RED "\e[0;31m"
#define L_RED "\e[1;31m"
#define YELLOW "\e[1;33m"

typedef struct RecoderTestContext
{
    TrecorderHandle*   mTrecorder;
    int              mRecorderId;
    int              mRecorderFileCount;
    char           mRecorderPath[128];
}RecoderTestContext;

typedef struct RecorderStatusContext
{
	int PreviewEnable;
	int PreviewRatioEnable;
	int AudioMuteEnable;
	int VideoMarkEnable;
	int RecordingEnable;
}RecorderStatusContext;

int CallbackFromTRecorder(void* pUserData, int msg, void* param);


#endif

#ifdef __cplusplus
}
#endif
