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

#ifndef RTMP_CFG_H
#define RTMP_CFG_H

#include "media_info.h"
#ifdef MEDIA_PROXY
#include "media_proxy.h"
#endif

typedef struct _RTMP_PUSHER
{
    struct _RTMP_PUSHER * next;
    
    char            srcurl[256];
    char            dsturl[256];        // the match url path
    char            user[100];          // the username for rtsp connection
    char            pass[100];          // the password for rtsp connection

    MEDIA_INFO      output;         // output information
} RTMP_PUSHER;

typedef struct _RTMP_CFG
{
    BOOL            log_enable;         // log enable 
    int             log_level;          // log level
    uint32          loop_nums;          // file loop-play numbers
    int             reconn_interval;    // re-connect interval, unit is second
    
    RTMP_PUSHER   * pusher;             // the audio & video output configuration
} RTMP_CFG;

#ifdef __cplusplus
extern "C" {
#endif

extern RTMP_CFG g_rtmp_cfg;

/**
 * read configuration file
 *
 * @param config the configuration file name
 * @return TRUE on success, FALSE on error
 */
BOOL         rtmp_read_config(const char * config);
RTMP_PUSHER* rtmp_add_pusher(RTMP_PUSHER ** p_output);
void         rtmp_free_pushers(RTMP_PUSHER ** p_pusher);

#ifdef __cplusplus
}
#endif


#endif



