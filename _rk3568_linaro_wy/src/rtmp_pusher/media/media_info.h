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

#ifndef MEDIA_INFO_H
#define MEDIA_INFO_H


typedef struct
{
    int     codec;                  // video codec, refer media_format.h
    int     width;                  // video width
    int     height;                 // video height
    double  framerate;              // frame rate
    int     bitrate;                // bitrate, unit is kb/s
} VIDEO_INFO;

typedef struct
{
    int     codec;                  // audio codec, refer media_format.h
    int     samplerate;             // sample rate
    int     channels;               // channels
    int     bitrate;                // bitrate, unit is kb/s
    int     bitpersample;           // bit per sample
} AUDIO_INFO;

typedef struct
{
	uint32  has_video : 1;          // has video?
	uint32  has_audio : 1;          // has audio?
	uint32  reserved  : 30;
	
    VIDEO_INFO  video;             // video information
    AUDIO_INFO  audio;             // audio information
} MEDIA_INFO;

#endif


