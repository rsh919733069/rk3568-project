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

#ifndef RTMP_MEDIA_H
#define RTMP_MEDIA_H

#include "sys_inc.h"
#include "rtmp_rmua.h"


#ifdef __cplusplus
extern "C" {
#endif

BOOL rtmp_parse_url(RMPUA * p_rua, RTMP_PUSHER * p_pusher);
BOOL rtmp_media_init(RMPUA * p_rua, RTMP_PUSHER * p_pusher);
BOOL rtmp_rua_start(RMPUA * p_rua);
void rtmp_rua_stop(RMPUA * p_rua);

#ifdef __cplusplus
}
#endif

#endif


