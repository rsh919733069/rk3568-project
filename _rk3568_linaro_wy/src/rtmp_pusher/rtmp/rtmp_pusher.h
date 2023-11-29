/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2014-2021, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifndef RTMP_PUSHER_H
#define RTMP_PUSHER_H

#include "sys_inc.h"
#include "hqueue.h"
#include "rtmp_cfg.h"
#include "rtmp_rmua.h"

/***************************************************************************************/

#define RTMP_RECONN	    1
#define RTMP_EXIT       4

#define VERSION_STRING  "V4.2"

#ifdef ANDROID
#define RTMP_PUSHER_LOG "/sdcard/rtmppusher.log"
#else
#define RTMP_PUSHER_LOG "rtmppusher.log"
#endif

/***************************************************************************************/

typedef struct
{
	HQUEUE *    msg_queue;		// message receive queue
	pthread_t   tid_main;       // main task thread
} RTMP_CLASS;

typedef struct 
{
	uint32      msg_src;        // message type
	uint32      msg_dua;        // message destination unit
} RIMSG_RTMP;

/***************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

BOOL rtmp_pusher_start1();
BOOL rtmp_pusher_start(const char * cfg_file);
void rtmp_pusher_stop();
BOOL rtmp_pusher_init(RTMP_PUSHER * p_pusher);
void rtmp_pusher_uninit(RTMP_PUSHER * p_pusher);
void rtmp_commit_reconn(RMPUA * p_rua);

int run_rtmp_pusher(void);

#ifdef __cplusplus
}
#endif

#endif


