/*
 * Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : trecordertest.c
 * Description : trecorder functional test
 * History :
 *
 */
#include "camerapreviewer.h"
#define CAMERA_DEV_INDEX T_CAMERA_CVBS

typedef struct Command
{
	const char* strCommand;
	int nCommandId;
	const char* strHelpMsg;
}Command;

int CallbackFromTRecorder(void* pUserData, int msg, void* param){
	printf(" CallbackFromTRecorder msg = %d \n", msg);
	return 0;
}
static bool isCapture = false;
static int oneChannelTest(RecoderTestContext *recorderTestContext, RecorderStatusContext *RecorderStatus, int number)
{
	int i;
    int ret = 0;
	TdispRect rect;
    char num[][6] = {"front", "rear"};
    char outputchar[64];
	int heigh;

	if(number==0){
		heigh = 576;
	}else{
		heigh = 480;
	}
	i = CAMERA_DEV_INDEX;
	/* Create TRecorder handle */
	memset(&recorderTestContext[i], 0, sizeof(RecoderTestContext));
	recorderTestContext[i].mTrecorder = CreateTRecorder();
	if (recorderTestContext[i].mTrecorder == NULL) {
		printf("CreateTRecorder[%d] err\n", i);
		return -1;
	}
	/* Reset Trecorder*/
	recorderTestContext[i].mRecorderId = i;
	TRreset(recorderTestContext[i].mTrecorder);

	TRsetCamera(recorderTestContext[i].mTrecorder, i);
	TRsetCameraCaptureSize(recorderTestContext[i].mTrecorder, 720, heigh);
    RecorderStatus[i].VideoMarkEnable = 0;
	TRsetPreview(recorderTestContext[i].mTrecorder,T_DISP_LAYER0);
    TRsetRecorderCallback(recorderTestContext[i].mTrecorder,CallbackFromTRecorder,(void*)&recorderTestContext[i]);
	TRsetPreviewCapture(recorderTestContext[i].mTrecorder, isCapture);
	printf("CVBS HEIGH is %d\n", heigh);

	ret = TRprepare(recorderTestContext[i].mTrecorder);
    if(ret < 0){
        printf("trecorder %d prepare error\n", i);
        return -1;
    }
	ret = TRstart(recorderTestContext[i].mTrecorder,T_PREVIEW);
    if(ret < 0){
        printf("trecorder %d start error\n", i);
        return -1;
    }
    RecorderStatus[i].PreviewEnable= 1;
    RecorderStatus[i].RecordingEnable = 0;

    return 0;
}

bool isNumber(char* str){
	int len = strlen(str);
    for (int i = 0; i < len; i++) {
	   if (!isdigit(str[i])) {
			return false;
		}
	}
	return true;
}

int main(int argc, char** argv)
{
	int number, i;
	RecoderTestContext recorderTestContext[2];
	RecorderStatusContext RecorderStatus[2];
	char strCommandLine[256];
	int  nCommandParam;
	int  nCommandId;
	int  bQuit = 0;
	TCaptureConfig captureConfig;
	char jpegPath[128];
	int frontjpgNumber = 0;
	int rearjpgNumber = 0;
	int switchTimes = 0;
	TdispRect rect;
	int try_times = 10;
	printf("****************************************************************************\n");
	printf("* This program shows how to test tvin CVBS\n");
	printf("* exampe: arg1: CVBS or PAL\n arg2: preview or bmp\n arg3: hold secondes\n");
	printf("****************************************************************************\n");
	if(argc==3||argc==4){
		if(strcmp(argv[1],"PAL")==0){
			printf("use PAL\n");
			number = 0;
		}else if((strcmp(argv[1],"NTSC")==0)){
			printf("use NTSC\n");
			number = 1;
		}else{
			printf("default use PAL\n");
			number = 0;
		}
		if(strcmp(argv[2],"preview")==0){
			isCapture = false;
		}else if(strcmp(argv[2],"bmp")==0){
			isCapture = true;
		}else{
			printf("default use preview\n");
			isCapture = false;
		}
		if(argc==4&&isNumber(argv[3])){
			try_times = atoi(argv[3]);
		}
		printf("holding %d secondes\n", try_times);
	}else{
		printf("parameter error\n");
		return -1;
	}
	printf("Camera is %s hold %d secondes\n", argv[1], try_times);
	oneChannelTest(recorderTestContext, RecorderStatus, number);
	while(--try_times){
		usleep(1000000);
	}
	TRstop(recorderTestContext[CAMERA_DEV_INDEX].mTrecorder, T_PREVIEW);
	TRrelease(recorderTestContext[CAMERA_DEV_INDEX].mTrecorder);
	printf("\n");
	printf("*************************************************************************\n");
	printf("* Quit the program, goodbye!\n");
	printf("********************************************************************\n");
	printf("\n");
	return 0;
}
