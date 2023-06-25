/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : cdc_version.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/

#ifndef CDC_VERSION_H
#define CDC_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define REPO_TAG "CedarC-v1.3.0"
#define REPO_PATCH ""
#define REPO_BRANCH "master"
#define REPO_COMMIT "df00b3625ddf928a5d99bdb9b8e1be80e41cfc27"
#define REPO_DATE "Sat Mar 19 14:28:48 2022 +0800"
#define RELEASE_AUTHOR "wuhuating"

static inline void LogVersionInfo(void)
{
    logd("\n"
         ">>>>>>>>>>>>>>>>>>>>>>>>>>>>> Cedar Codec <<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"
         "tag   : %s\n"
         "branch: %s\n"
         "commit: %s\n"
         "date  : %s\n"
         "author: %s\n"
         "patch : %s\n"
         "----------------------------------------------------------------------\n",
         REPO_TAG, REPO_BRANCH, REPO_COMMIT, REPO_DATE, RELEASE_AUTHOR, REPO_PATCH);
}

/* usage: TagVersionInfo(myLibTag) */
#define TagVersionInfo(tag) \
    static void VersionInfo_##tag(void) __attribute__((constructor));\
    void VersionInfo_##tag(void) \
    { \
        logd("-------library tag: %s-------", #tag);\
        LogVersionInfo(); \
    }


#ifdef __cplusplus
}
#endif

#endif

