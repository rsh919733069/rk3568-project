/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2014-2020, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifndef MEDIA_UTIL_H
#define MEDIA_UTIL_H

#include "sys_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32  remove_emulation_bytes(uint8* to, uint32 toMaxSize, uint8* from, uint32 fromSize);
uint8 * avc_find_startcode(uint8 *p, uint8 *end);
uint8 * avc_split_nalu(uint8 * e_buf, int e_len, int * s_len, int * d_len);
uint8   avc_h264_nalu_type(uint8 * e_buf, int len);
uint8   avc_h265_nalu_type(uint8 * e_buf, int len);

#ifdef __cplusplus
}
#endif

#endif



