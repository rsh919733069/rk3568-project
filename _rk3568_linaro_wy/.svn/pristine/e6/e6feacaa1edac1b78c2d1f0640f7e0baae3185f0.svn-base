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

#ifndef RTMP_TX_H
#define RTMP_TX_H

#include "sys_inc.h"


#define RESV_HEADER_SIZE    128


#ifdef __cplusplus
extern "C" {
#endif

char  * rtmp_put_byte(char *output, uint8 val);
char  * rtmp_put_be16(char *output, uint16 val);
char  * rtmp_put_be24(char *output, uint32 val);
char  * rtmp_put_be32(char *output, uint32 val);
char  * rtmp_put_be64(char *output, ulint64 val);
char  * rtmp_put_amf_string(char *c, const char *str);
char  * rtmp_put_amf_bool(char *c, int b);
char  * rtmp_put_amf_double(char *c, double d); 

int     rtmp_send_metadata(void * info);
int     rtmp_h26x_tx(void * p_rua, uint8 * p_data, int len, uint32 ts);
int     rtmp_aac_tx(void * p_rua, uint8 * p_data, int len, uint32 ts);
int     rtmp_g711a_tx(void * info, uint8 * p_data, int len, uint32 ts);
int     rtmp_g711u_tx(void * info, uint8 * p_data, int len, uint32 ts);

#ifdef __cplusplus
}
#endif

#endif


