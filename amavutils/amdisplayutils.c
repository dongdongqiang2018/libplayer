/*
 * Copyright (C) 2010 Amlogic Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */




#define LOG_TAG "amavutils"

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#ifdef ANDROID
#include <cutils/log.h>
#endif
#include <sys/ioctl.h>
#include "include/Amdisplayutils.h"


#define FB_DEVICE_PATH   "/sys/class/graphics/fb0/virtual_size"
#define SCALE_PATH       "/sys/class/graphics/fb0/scale"
#define SCALE_REQUEST    "/sys/class/graphics/fb0/request2XScale"
#define OSD_ROTATION_PATH "/sys/class/graphics/fb0/prot_angle"
#define OSD_ROTATION_ON "/sys/class/graphics/fb0/prot_on"
#define SYSCMD_BUFSIZE 40


//Temp solution, finally we will move out all the functions about display,
//it should be not part of player.
#ifndef FB_BUFFER_NUM
#define FB_BUFFER_NUM (2)
#endif
#ifdef ANDROID
#ifndef LOGD
#define LOGV ALOGV
#define LOGD ALOGD
#define LOGI ALOGI
#define LOGW ALOGW
#define LOGE ALOGE
#endif
#else
#define LOGV printf
#define LOGD printf
#define LOGI printf
#define LOGW printf
#define LOGE printf
#endif

//#define LOG_FUNCTION_NAME LOGI("%s-%d\n",__FUNCTION__,__LINE__);
#define LOG_FUNCTION_NAME

static void get_display_mode(char *mode)
{
    int fd;
    char *path = "/sys/class/display/mode";
    if (!mode) {
        LOGE("[get_display_mode]Invalide parameter!");
        return;
    }
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        memset(mode, 0, 16); // clean buffer and read 15 byte to avoid strlen > 15
        read(fd, mode, 15);
        LOGI("[get_display_mode]mode=%s strlen=%d\n", mode, strlen(mode));
        mode[strlen(mode)] = '\0';
        close(fd);
    } else {
        sprintf(mode, "%s", "fail");
    };
    LOGI("[get_display_mode]display_mode=%s\n", mode);
    return ;
}
int amdisplay_utils_get_size(int *width, int *height)
{
    LOG_FUNCTION_NAME
    char buf[SYSCMD_BUFSIZE];
    int disp_w = 0;
    int disp_h = 0;
    int ret;
    ret = amsysfs_get_sysfs_str(FB_DEVICE_PATH, buf, SYSCMD_BUFSIZE);
    if (ret < 0) {
        return ret;
    }
    if (sscanf(buf, "%d,%d", &disp_w, &disp_h) == 2) {
        LOGI("disp resolution %dx%d\n", disp_w, disp_h);
        disp_h = disp_h / FB_BUFFER_NUM;
    } else {
        return -2;/*format unknow*/
    }
    *width = disp_w;
    *height = disp_h;
    return 0;
}

#define FB_DEVICE_PATH_FB2   "/sys/class/graphics/fb2/virtual_size"
int amdisplay_utils_get_size_fb2(int *width, int *height)
{
    LOG_FUNCTION_NAME
    char buf[SYSCMD_BUFSIZE];
    int disp_w = 0;
    int disp_h = 0;
    int ret;
    ret = amsysfs_get_sysfs_str(FB_DEVICE_PATH_FB2, buf, SYSCMD_BUFSIZE);
    if (ret < 0) {
        return ret;
    }
    if (sscanf(buf, "%d,%d", &disp_w, &disp_h) == 2) {
        LOGI("disp resolution %dx%d\n", disp_w, disp_h);
        disp_h = disp_h / FB_BUFFER_NUM;
    } else {
        return -2;/*format unknow*/
    }
    *width = disp_w;
    *height = disp_h;
    return 0;
}

int amdisplay_utils_set_scale_mode(int scale_wx, int scale_hx)
{
    int width, height;
    int ret;
    int neww, newh;
    char buf[40];

    /*scale mode only support x2,x1*/
    if ((scale_wx != 1 && scale_wx != 2) || (scale_hx != 1 && scale_hx != 2)) {
        LOGI("unsupport scaling mode,x1,x2 only\n", scale_wx, scale_hx);
        return -1;
    }

    if (scale_wx == 2) {
        ret = amsysfs_set_sysfs_str(SCALE_REQUEST, "1");
    } else if (scale_wx == 1) {
        ret = amsysfs_set_sysfs_str(SCALE_REQUEST, "2");
    }

    if (ret < 0) {
        LOGI("set [%s]=[%s] failed\n", SCALE_REQUEST, buf);
        return -2;
    }

    return ret;
}


int amdisplay_utils_get_osd_rotation()
{
    char buf[40];
    int ret;

    ret = amsysfs_get_sysfs_str(OSD_ROTATION_ON, buf, SYSCMD_BUFSIZE);
    if ((ret < 0) || strstr(buf, "OFF")) {
        return 0;//no rotation+
    }
    memset(buf, 0 , 40);


    ret = amsysfs_get_sysfs_str(OSD_ROTATION_PATH, buf, SYSCMD_BUFSIZE);
    if (ret < 0) {
        return 0;    //no rotation
    }

    int rotation = 0;
    if (sscanf(buf, "osd_rotate:%d", &rotation) == 1) {
        LOGI("get osd rotation  %d\n", rotation);
    }

    switch (rotation) {
    case 0:
        rotation = 0;
        break;
    case 1:
        rotation = 90;
        break;
    case 2:
        rotation = 270;
        break;
    default:
        break;
    }

    LOGD("amdisplay_utils_get_osd_rotation return %d", rotation);
    return rotation;
}


