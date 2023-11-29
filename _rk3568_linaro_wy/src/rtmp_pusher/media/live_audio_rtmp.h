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

#ifndef LIVE_AUDIO_H
#define LIVE_AUDIO_H

/***************************************************************************************/

#include "linked_list.h"
#include "rtmp_rmua.h"
/***************************************************************************************/


/*
 * After getting the encoded data from the hardware encoder, 
 * you can call this interface to send the data to the audio sending queue 
 *
 * If you use this interface to send audio data, you do not need to 
 * implement the CLiveAudio_rtmp::captureThread function 
 */
BOOL media_live_put_audio(int idx, uint8 * data, int size, int nbsamples);

#endif

