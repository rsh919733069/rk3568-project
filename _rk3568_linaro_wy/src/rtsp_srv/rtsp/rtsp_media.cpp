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

#include "sys_inc.h"
#include "rtsp_media.h"
#include "rtsp_rsua.h"
#include "rtp_tx.h"
#include "hqueue.h"
#include "media_format.h"
#include "rtsp_cfg.h"
#include "rtsp_srv.h"
#include "rtsp_util.h"
#include "log/log.h"
#include "video_common.h"
#include "logger.h"

BOOL rtsp_get_url_path(const char * url, char * path, int path_size)
{
    char host[128];
    
    url_split(url, NULL, 0, NULL, 0, NULL, 0, host, sizeof(host), NULL, path, path_size);

    if (host[0] == '\0')
    {
        return FALSE;
    }
    
	return TRUE;
}

char * rtsp_media_get_video_sdp_line(void * rua)
{
	RSUA * p_rua = (RSUA *)rua;

#ifdef MEDIA_LIVE
	if (p_rua->media_info.is_live)
	{
		if (p_rua->media_info.live_video)
		{
			return p_rua->media_info.live_video->getAuxSDPLine(p_rua->channels[AV_VIDEO_CH].cap[0]);
		}
	}
#endif

	return NULL;
}


char * rtsp_media_get_audio_sdp_line(void * rua)
{
	RSUA * p_rua = (RSUA *)rua;

#ifdef MEDIA_LIVE
	if (p_rua->media_info.is_live)
	{
		if (p_rua->media_info.live_audio)
		{
			return p_rua->media_info.live_audio->getAuxSDPLine(p_rua->channels[AV_AUDIO_CH].cap[0]);
		}
	}
#endif

	return NULL;
}

void rtsp_media_fix_audio_param(RSUA * p_rua)
{
	if (AUDIO_CODEC_G726 == p_rua->media_info.a_codec)
	{
		p_rua->media_info.a_channels = 1; // G726 only support mono
		p_rua->media_info.a_samplerate = 8000;
		return;
	}

	if (AUDIO_CODEC_G722 == p_rua->media_info.a_codec)
	{
		p_rua->media_info.a_channels = 1; // G722 only support mono
		p_rua->media_info.a_samplerate = 16000;
		return;
	}

	if (AUDIO_CODEC_OPUS == p_rua->media_info.a_codec)
	{
		p_rua->media_info.a_channels = 2;
		p_rua->media_info.a_samplerate = 48000;
		return;
	}

	const int sample_rates[] = 
	{
		8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
	};

	int i;
	int sample_rate_num = sizeof(sample_rates) / sizeof(int);

	for (i = 0; i < sample_rate_num; i++)
	{
		if (p_rua->media_info.a_samplerate <= sample_rates[i])
		{
			p_rua->media_info.a_samplerate = sample_rates[i];
			break;
		}
	}

	if (i == sample_rate_num)
	{
		p_rua->media_info.a_samplerate = 48000;
	}
}

BOOL rtsp_parse_url_transfer_parameters(RSUA * p_rua)
{
	char value[32] = {'\0'};
	char * p = strchr(p_rua->media_info.filename, '&');	
	if (NULL == p)
	{
		return FALSE;
	}

	p++; // skip '&' char
	
	if (GetNameValuePair(p, strlen(p), "t", value, sizeof(value)))
	{
		if (strcasecmp(value, "unicast") == 0)
		{
			p_rua->rtp_unicast = 1;
		}
		else if (strcasecmp(value, "multicase") == 0)
		{
			p_rua->rtp_unicast = 0;
		}
	}

	if (GetNameValuePair(p, strlen(p), "p", value, sizeof(value)))
	{
		if (strcasecmp(value, "udp") == 0)
		{
			p_rua->rtp_tcp = 0;
		}
		else if (strcasecmp(value, "tcp") == 0)
		{
			p_rua->rtp_tcp = 1;
		}
		else if (strcasecmp(value, "rtsp") == 0)
		{
			p_rua->rtp_tcp = 1;
		}
		else if (strcasecmp(value, "http") == 0)
		{

		}
	}

	return TRUE;
}

BOOL rtsp_parse_url_video_parameters(RSUA * p_rua)
{
	char value[32] = {'\0'};
	char * p = strchr(p_rua->media_info.filename, '&');	
	if (NULL == p)
	{
		return FALSE;
	}

	p++; // skip '&' char

	if (GetNameValuePair(p, strlen(p), "ve", value, sizeof(value)))
	{
		if (strcasecmp(value, "JPEG") == 0)
		{
			p_rua->media_info.v_codec = VIDEO_CODEC_JPEG;
		}
		else if (strcasecmp(value, "H264") == 0)
		{
			p_rua->media_info.v_codec = VIDEO_CODEC_H264;
		}
		else if (strcasecmp(value, "H265") == 0)
		{
			p_rua->media_info.v_codec = VIDEO_CODEC_H265;
		}
		else if (strcasecmp(value, "MP4V-ES") == 0 || strcasecmp(value, "MP4") == 0)
		{
			p_rua->media_info.v_codec = VIDEO_CODEC_MP4;
		}
	}

    if (GetNameValuePair(p, strlen(p), "fps", value, sizeof(value)))
	{
		p_rua->media_info.v_framerate = atoi(value);
	}
	
	if (GetNameValuePair(p, strlen(p), "w", value, sizeof(value)))
	{
		p_rua->media_info.v_width = atoi(value);
	}

	if (GetNameValuePair(p, strlen(p), "h", value, sizeof(value)))
	{
		p_rua->media_info.v_height = atoi(value);
	}

	return TRUE;
}

BOOL rtsp_parse_url_audio_parameters(RSUA * p_rua)
{
	char value[32] = {'\0'};
	char * p = strchr(p_rua->media_info.filename, '&');	
	if (NULL == p)
	{
		return FALSE;
	}

	p++; // skip '&' char

	if (GetNameValuePair(p, strlen(p), "ae", value, sizeof(value)))
	{
		if (strcasecmp(value, "PCMU") == 0 || strcasecmp(value, "G711U") == 0)
		{
			p_rua->media_info.a_codec = AUDIO_CODEC_G711U;
		}
	    else if (strcasecmp(value, "PCMA") == 0 || strcasecmp(value, "G711A") == 0)
		{
			p_rua->media_info.a_codec = AUDIO_CODEC_G711A;
		}
		else if (strcasecmp(value, "G726") == 0)
		{
			p_rua->media_info.a_codec = AUDIO_CODEC_G726;
		}
		else if (strcasecmp(value, "G722") == 0)
		{
			p_rua->media_info.a_codec = AUDIO_CODEC_G722;
		}
		else if (strcasecmp(value, "OPUS") == 0)
		{
			p_rua->media_info.a_codec = AUDIO_CODEC_OPUS;
		}
		else if (strcasecmp(value, "MP4A-LATM") == 0 || strcasecmp(value, "AAC") == 0)
		{
			p_rua->media_info.a_codec = AUDIO_CODEC_AAC;
		}
	}

	if (GetNameValuePair(p, strlen(p), "sr", value, sizeof(value)))
	{
		p_rua->media_info.a_samplerate = atoi(value);
	}

	if (GetNameValuePair(p, strlen(p), "ch", value, sizeof(value)))
	{
		p_rua->media_info.a_channels = atoi(value);
	}

	return TRUE;
}

#ifdef MEDIA_LIVE
BOOL rtsp_parse_live_suffix(RSUA * p_rua, const char * suffix)
{
	// todo : parse the URL suffix, set the p_rua->media_info struct

	/*if (strncasecmp(suffix, "live", strlen("live")) != 0)
	{
		return FALSE;
	}*/
	
	p_rua->media_info.is_live = 1;
	p_rua->media_info.has_video = 1;
	p_rua->media_info.has_audio = 0;
	p_rua->media_info.v_width = 1920;
	p_rua->media_info.v_height = 1080;
	p_rua->media_info.v_framerate = 30;
	p_rua->media_info.a_samplerate = 8000;
	p_rua->media_info.a_channels = 1;
	p_rua->media_info.v_index = 0;
	p_rua->media_info.a_index = 0;

	if(p_rua->media_info.has_video)
	{
		if(VIDEO_CODEC_NONE == p_rua->media_info.v_codec)
		{
			if(gs_vedio_info.output_type==TYPE_H264){
				p_rua->media_info.v_codec = VIDEO_CODEC_H264;
			}else{
				p_rua->media_info.v_codec = VIDEO_CODEC_H265;
			}
		}

		// get the live video instance
		p_rua->media_info.live_video = CLiveVideo::getInstance(p_rua->media_info.v_index);

		if(NULL == p_rua->media_info.live_video)
		{
			return FALSE;
		}
		else if(p_rua->media_info.live_video->initCapture(p_rua->media_info.v_codec,
														p_rua->media_info.v_width,
														p_rua->media_info.v_height,
														p_rua->media_info.v_framerate,
														p_rua->media_info.v_bitrate) == FALSE)
		{
			log_print(HT_LOG_ERR, "%s, init live video capture failed\r\n", __FUNCTION__);
			p_rua->media_info.live_video->freeInstance(p_rua->media_info.v_index);
			p_rua->media_info.live_video = NULL;
			return FALSE;
		}
	}



	if(p_rua->media_info.has_audio)
	{
		if(AUDIO_CODEC_NONE == p_rua->media_info.a_codec)
		{
			p_rua->media_info.a_codec = AUDIO_CODEC_G711A;
		}

		// get the live video instance
		p_rua->media_info.live_audio = CLiveAudio::getInstance(p_rua->media_info.a_index);

		if(NULL == p_rua->media_info.live_audio)
		{
			return FALSE;
		}
		else if(p_rua->media_info.live_audio->initCapture(p_rua->media_info.a_codec,
														p_rua->media_info.a_samplerate,
														p_rua->media_info.a_channels,
														p_rua->media_info.a_bitrate) == FALSE)
		{
			log_print(HT_LOG_ERR, "%s, init live audio capture failed\r\n", __FUNCTION__);
			p_rua->media_info.live_audio->freeInstance(p_rua->media_info.a_index);
			p_rua->media_info.live_audio = NULL;
			return FALSE;
		}
	}

	// todo : if have audio, uncomment the statements

	// p_rua->media_info.has_audio = 1;
	// p_rua->media_info.a_codec = AUDIO_CODEC_G711U;
	// p_rua->media_info.a_samplerate = 8000;
	// p_rua->media_info.a_channels = 1;
	// p_rua->media_info.a_index = 0;

	return TRUE;
}

BOOL rtsp_media_live_init(RSUA * p_rua)
{
	// todo : init the live stream object

	if (p_rua->media_info.has_video)
	{
		rtsp_parse_url_transfer_parameters(p_rua);

		if (p_rua->media_info.v_index < 0 || 
			p_rua->media_info.v_index >= CLiveVideo::getStreamNums())
		{
			log_print(HT_LOG_ERR, "%s, v_index=%d, getStreamNums=%d\r\n", __FUNCTION__, 
			p_rua->media_info.v_index, CLiveVideo::getStreamNums());
			return FALSE;
		}

		if (VIDEO_CODEC_NONE == p_rua->media_info.v_codec)
		{
			p_rua->media_info.v_codec = VIDEO_CODEC_H264;		
		}
		
		if (0 == p_rua->media_info.v_framerate)
		{
			p_rua->media_info.v_framerate = 25;
		}

		rtsp_parse_url_video_parameters(p_rua);

		// get the live video instance
		p_rua->media_info.live_video = CLiveVideo::getInstance(p_rua->media_info.v_index);
		if (p_rua->media_info.live_video->initCapture(p_rua->media_info.v_codec,
													  p_rua->media_info.v_width, 
													  p_rua->media_info.v_height, 
													  p_rua->media_info.v_framerate,
													  p_rua->media_info.v_bitrate) == FALSE)
		{
			log_print(HT_LOG_ERR, "%s, init live video capture failed\r\n", __FUNCTION__);
			p_rua->media_info.live_video->freeInstance(p_rua->media_info.v_index);
			p_rua->media_info.live_video = NULL;
			return FALSE;
		}
	}

	if (p_rua->media_info.has_audio)
	{
		if (p_rua->media_info.a_index < 0 || 
			p_rua->media_info.a_index >= CLiveAudio::getDeviceNums())
		{
			log_print(HT_LOG_ERR, "%s, aindex=%d, getDeviceNums=%d\r\n", __FUNCTION__, 
				p_rua->media_info.a_index, CLiveAudio::getDeviceNums());
			return FALSE;
		}

		if (AUDIO_CODEC_NONE == p_rua->media_info.a_codec)
		{
			p_rua->media_info.a_codec = AUDIO_CODEC_G711U;		
		}

		if (0 == p_rua->media_info.a_samplerate)
		{
			p_rua->media_info.a_samplerate = 8000;
		}

		if (0 == p_rua->media_info.a_channels)
		{
			p_rua->media_info.a_channels = 1;
		}

		rtsp_parse_url_audio_parameters(p_rua);

		rtsp_media_fix_audio_param(p_rua);

		// get audio capture instance
		p_rua->media_info.live_audio = CLiveAudio::getInstance(p_rua->media_info.a_index);
		if (p_rua->media_info.live_audio->initCapture(p_rua->media_info.a_codec, 
		                                              p_rua->media_info.a_samplerate, 
		                                              p_rua->media_info.a_channels,
		                                              p_rua->media_info.a_bitrate) == FALSE)
		{
			log_print(HT_LOG_ERR, "%s, init live audio capture failed\r\n", __FUNCTION__);
			p_rua->media_info.live_audio->freeInstance(p_rua->media_info.a_index);
			p_rua->media_info.live_audio = NULL;
			return FALSE;
		}
	}
	
	return TRUE;
}
#endif	// end of MEDIA_LIVE


BOOL rtsp_media_init(void * rua)
{
	RSUA * p_rua = (RSUA *)rua;
	char * p_suffix;
	char path[256];
	
	if (!rtsp_get_url_path(p_rua->uri, path, sizeof(path)))
	{
		return FALSE;
	}

	if (path[0] == '/')
	{
	    p_suffix = path + 1;
	}
	else
	{
	    p_suffix = path;
	}

	strncpy(p_rua->media_info.filename, p_suffix, sizeof(p_rua->media_info.filename)-1);

#ifdef MEDIA_LIVE
	if (rtsp_parse_live_suffix(p_rua, p_suffix))
	{
		return rtsp_media_live_init(p_rua);
	}
#endif

    return TRUE;
}

void rtsp_media_clear_queue(HQUEUE * queue)
{
	UA_PACKET packet;
	
	while (!hqBufIsEmpty(queue))
	{
		if (hqBufGet(queue, (char *)&packet))
		{
			if (packet.data != NULL && packet.size != 0)
			{
				free(packet.buff);
			}
		}
		else
		{
		    // should be not to here
		    log_print(HT_LOG_ERR, "%s, hqBufGet failed\r\n", __FUNCTION__);
		    break;
		}
	}
}

void * rtsp_media_video_thread(void * argv)
{
	RSUA * p_rua = (RSUA *)argv;
	UA_PACKET packet;
    int sret = -1;
	int cur_delay = 0;
	int pre_delay = 0;
	int timeout = 1000000.0 / p_rua->media_info.v_framerate;
	uint32 cur_time = 0;
	uint32 pre_time = 0;

	while (p_rua->rtp_tx)
	{
	    if (p_rua->rtp_pause)
	    {
	        usleep(10*1000);
	        continue;
	    }
	    
		if (hqBufGet(p_rua->media_info.v_queue, (char *)&packet))
		{
			if (packet.data == NULL || packet.size == 0)
			{
				break;
			}

#ifdef RTSP_REPLAY
            if (p_rua->replay)
            {
                rtsp_update_replay_ntp_time(p_rua, AV_VIDEO_CH);
            }
#endif

			if (p_rua->media_info.v_codec == VIDEO_CODEC_H264)
			{
				sret = rtp_h264_video_tx(p_rua, packet.data, packet.size, rtsp_get_timestamp(90000));
			}
			else if (p_rua->media_info.v_codec == VIDEO_CODEC_H265)
			{
				sret = rtp_h265_video_tx(p_rua, packet.data, packet.size, rtsp_get_timestamp(90000));
			}
			else if (p_rua->media_info.v_codec == VIDEO_CODEC_MP4)
			{
				sret = rtp_video_tx(p_rua, packet.data, packet.size, rtsp_get_timestamp(90000));
			}
			else if (p_rua->media_info.v_codec == VIDEO_CODEC_JPEG)
			{
				sret = rtp_jpeg_video_tx(p_rua, packet.data, packet.size, rtsp_get_timestamp(90000));
			}

			free(packet.buff);

#ifdef RTSP_REPLAY
            if (p_rua->replay && !p_rua->rate_control)
            {
                usleep(10*1000);
                continue;
            }
#endif
			if (packet.waitnext)
			{
			    cur_time = sys_os_get_ms();
			    cur_delay = timeout;

			    if (pre_time > 0)
			    {
			        cur_delay += pre_delay - (cur_time - pre_time) * 1000;
			        if (cur_delay < 1000)
			        {
			            cur_delay = 0;
			        }
			    }

			    pre_time = cur_time;
			    pre_delay = cur_delay;
			    
			    if (cur_delay > 0)
			    {
				    usleep(cur_delay);
				}
			}
		}
		else
		{
		    // should be not to here
		    log_print(HT_LOG_ERR, "%s, hqBufGet failed\r\n", __FUNCTION__);
		    break;
		}
	}

	rtsp_media_clear_queue(p_rua->media_info.v_queue);
	
	p_rua->media_info.v_thread = 0;
	
	return NULL;
}

void * rtsp_media_audio_thread(void * argv)
{
	RSUA * p_rua = (RSUA *)argv;
	UA_PACKET packet;
	int sret = -1;
	int samplerate = p_rua->media_info.a_samplerate;
	int cur_delay = 0;
	int pre_delay = 0;
	uint32 cur_time = 0;
	uint32 pre_time = 0;

	while (p_rua->rtp_tx)
	{
	    if (p_rua->rtp_pause)
	    {
	        usleep(10*1000);
	        continue;
	    }
	    
		if (hqBufGet(p_rua->media_info.a_queue, (char *)&packet))
		{
			if (packet.data == NULL || packet.size == 0)
			{
				break;
			}

#ifdef RTSP_REPLAY
            if (p_rua->replay)
            {
                rtsp_update_replay_ntp_time(p_rua, AV_AUDIO_CH);
            }
#endif

			if (p_rua->media_info.a_codec == AUDIO_CODEC_AAC)
			{
				sret = rtp_aac_audio_tx(p_rua, packet.data, packet.size, rtsp_get_timestamp(samplerate));
			}
			else 
			{
				sret = rtp_audio_tx(p_rua, packet.data, packet.size, rtsp_get_timestamp(samplerate));
			}

			free(packet.buff);

#ifdef RTSP_REPLAY
            if (p_rua->replay && !p_rua->rate_control)
            {
                usleep(10*1000);
                continue;
            }
#endif

			if (packet.waitnext)
			{
			    cur_time = sys_os_get_ms();
			    cur_delay = 1000000.0 / samplerate * packet.nbsamples;

			    if (pre_time > 0)
			    {
			        cur_delay += pre_delay - (cur_time - pre_time) * 1000;
			        if (cur_delay < 1000)
			        {
			            cur_delay = 0;
			        }
			    }

			    pre_time = cur_time;
			    pre_delay = cur_delay;
			    
			    if (cur_delay > 0)
			    {
				    usleep(cur_delay);
				}
			}
		}
		else
		{
		    // should be not to here
		    log_print(HT_LOG_ERR, "%s, hqBufGet failed\r\n", __FUNCTION__);
		    break;
		}
	}

	rtsp_media_clear_queue(p_rua->media_info.a_queue);
	
	p_rua->media_info.a_thread = 0;
	
	return NULL;
}

#ifdef RTSP_METADATA

BOOL rtsp_media_get_metadata(UA_PACKET * p_packet)
{
	int offset;
	
	// todo : here just generate test metadata
	
	p_packet->buff = (uint8*) malloc(1024+40);
	p_packet->data = p_packet->buff + 40; // skip rtp header
	p_packet->waitnext = 1;

	offset = sprintf((char *)p_packet->data, 
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<tt:MetadataStream xmlns:tt=\"http://www.onvif.org/ver10/schema\">"
			"<tt:Event>"
				"<wsnt:NotificationMessage>"
					"<wsnt:Message>"
						"<tt:Message UtcTime=\"2008-10-10T12:24:57.628\">"
						"<tt:Source>"
							"<tt:SimpleItem Name=\"InputToken\" Value=\"DIGIT_INPUT_000\" />"
						"</tt:Source>"
						"<tt:Data>"
							"<tt:SimpleItem Name=\"LogicalState\" Value=\"true\" />"
						"</tt:Data>"
						"</tt:Message>"
					"</wsnt:Message>"
				"</wsnt:NotificationMessage>"
			"</tt:Event>"
		"</tt:MetadataStream>");

	p_packet->size = offset;
	
	return TRUE;
}

void * rtsp_media_metadata_thread(void * argv)
{
	UA_PACKET packet;
	RSUA * p_rua = (RSUA *)argv;
	
	while (p_rua->rtp_tx)
	{
	    if (p_rua->rtp_pause)
	    {
	        usleep(10*1000);
	        continue;
	    }

		memset(&packet, 0, sizeof(packet));
		
		if (rtsp_media_get_metadata(&packet))
		{
#ifdef RTSP_REPLAY
            if (p_rua->replay)
            {
                rtsp_update_replay_ntp_time(p_rua, AV_METADATA_CH);
            }
#endif

	    	rtp_metadata_tx(p_rua, packet.data, packet.size, rtsp_get_timestamp(90000));
		}

		if (packet.buff)
		{
			free(packet.buff);
		}

#ifdef RTSP_REPLAY
        if (p_rua->replay && !p_rua->rate_control)
        {
            usleep(10*1000);
            continue;
        }
#endif

		if (packet.waitnext)
		{
	    	usleep(1000000 / 25); // 25fps
	    }
	}

	p_rua->media_info.m_thread = 0;
	
	return NULL;
}
#endif

void rtsp_media_put_video(RSUA * p_rua, uint8 *data, int size, int waitnext = 1)
{
	UA_PACKET packet;

	if (!p_rua->rtp_tx)
	{
		return;
	}
	
	if (data && size > 0)
	{
		packet.buff = (uint8*)malloc(size+64);
		packet.data = packet.buff + 64;
		
		if (packet.buff)
		{
			memcpy(packet.data, data, size);
			packet.size = size;
			packet.waitnext = waitnext;
		
			if (hqBufPut(p_rua->media_info.v_queue, (char *)&packet) == FALSE)
			{
				free(packet.buff);
			}
		}
	}
	else
	{
		packet.data = NULL;
		packet.size = 0;
		packet.waitnext = waitnext;

		hqBufPut(p_rua->media_info.v_queue, (char *)&packet);
	}
}

void rtsp_media_put_audio(RSUA * p_rua, uint8 *data, int size, int nbsamples, int waitnext = 1)
{
	UA_PACKET packet;

	if (!p_rua->rtp_tx)
	{
		return;
	}

	if (data && size > 0)
	{
		packet.buff = (uint8*)malloc(size+64); // skip forward header
		packet.data = packet.buff+64;
		
		if (packet.buff)
		{
			memcpy(packet.data, data, size);
			packet.size = size;
			packet.nbsamples = nbsamples;
			packet.waitnext = waitnext;
		
			if (hqBufPut(p_rua->media_info.a_queue, (char *)&packet) == FALSE)
			{
				free(packet.buff);
			}
		}
	}
	else
	{
		packet.data = NULL;
		packet.size = 0;
		packet.nbsamples = 0;
		packet.waitnext = waitnext;

		hqBufPut(p_rua->media_info.a_queue, (char *)&packet);
	}
}

void rtsp_send_epmty_packet(HQUEUE * hq)
{
	UA_PACKET packet;
	memset(&packet, 0, sizeof(packet));

	hqBufPut(hq, (char *)&packet);
}

void rtsp_media_free_queue(RSUA * p_rua)
{
	if (p_rua->media_info.has_video && p_rua->media_info.v_queue)
	{
		rtsp_send_epmty_packet(p_rua->media_info.v_queue);
	}

	if (p_rua->media_info.has_audio && p_rua->media_info.a_queue)
	{
		rtsp_send_epmty_packet(p_rua->media_info.a_queue);
	}
	
	while (p_rua->media_info.v_thread)
	{
		usleep(10*1000);
	}

	rtsp_media_clear_queue(p_rua->media_info.v_queue);
	
	hqDelete(p_rua->media_info.v_queue);
	p_rua->media_info.v_queue = NULL;

	while (p_rua->media_info.a_thread)
	{
		usleep(10*1000);
	}

	rtsp_media_clear_queue(p_rua->media_info.a_queue);
	
	hqDelete(p_rua->media_info.a_queue);
	p_rua->media_info.a_queue = NULL;

#ifdef RTSP_METADATA
	while (p_rua->media_info.m_thread)
	{
		usleep(10*1000);
	}
#endif
}


#ifdef MEDIA_LIVE
void rtsp_media_live_video_callback(uint8 * data, int size, void * pUserdata)
{
    RSUA * p_rua = (RSUA *)pUserdata;

    if (p_rua->channels[AV_VIDEO_CH].setup)
    {
	    rtsp_media_put_video(p_rua, data, size, 0);
	}
}

void rtsp_media_live_audio_callback(uint8 * data, int size, int nbsamples, void * pUserdata)
{
    RSUA * p_rua = (RSUA *)pUserdata;
    
    if (p_rua->channels[AV_AUDIO_CH].setup)
    {
	    rtsp_media_put_audio(p_rua, data, size, nbsamples, 0);
	}
}

void rtsp_media_live_video_capture(RSUA * p_rua)
{
	CLiveVideo * capture = p_rua->media_info.live_video;
    if (NULL == capture)
    {
        log_print(HT_LOG_ERR, "%s, capture object is null\r\n", __FUNCTION__);
        return;
    }

    p_rua->media_info.v_queue = hqCreate(10, sizeof(UA_PACKET), HQ_GET_WAIT);
	p_rua->media_info.v_thread = sys_os_create_thread((void *)rtsp_media_video_thread, (void*)p_rua);

	capture->addCallback(rtsp_media_live_video_callback, p_rua);
	capture->startCapture();
	struct timeval time;
	while (p_rua->rtp_tx)
	{
		//usleep(200*1000);
		time.tv_sec=0;
		time.tv_usec=(200*1000);
		select(0,NULL,NULL,NULL,&time); 
	}
	logger_info("rtsp","v_index %d ",p_rua->media_info.v_index);
	capture->delCallback(rtsp_media_live_video_callback, p_rua);
	capture->freeInstance(p_rua->media_info.v_index);

    p_rua->media_info.live_video = NULL;
    
	rtsp_send_epmty_packet(p_rua->media_info.v_queue);

	while (p_rua->media_info.v_thread)
	{
		usleep(10*1000);
	}

	rtsp_media_clear_queue(p_rua->media_info.v_queue);
	
	hqDelete(p_rua->media_info.v_queue);
	p_rua->media_info.v_queue = NULL;
}

void rtsp_media_live_audio_capture(RSUA * p_rua)
{
	CLiveAudio * capture = p_rua->media_info.live_audio;
	if (NULL == capture)
	{
		log_print(HT_LOG_ERR, "%s, get audio capture instace (%d) failed\r\n", __FUNCTION__, p_rua->media_info.a_index);
		return;
	}

	p_rua->media_info.a_queue = hqCreate(10, sizeof(UA_PACKET), HQ_GET_WAIT);
	p_rua->media_info.a_thread = sys_os_create_thread((void *)rtsp_media_audio_thread, (void*)p_rua);
	
	capture->addCallback(rtsp_media_live_audio_callback, p_rua);
	capture->startCapture();

	while (p_rua->rtp_tx)
	{
		usleep(200*1000);
	}

	capture->delCallback(rtsp_media_live_audio_callback, p_rua);
	capture->freeInstance(p_rua->media_info.a_index);

    p_rua->media_info.live_audio = NULL;
    
	rtsp_send_epmty_packet(p_rua->media_info.a_queue);

	while (p_rua->media_info.a_thread)
	{
		usleep(10*1000);
	}

	rtsp_media_clear_queue(p_rua->media_info.a_queue);
	
	hqDelete(p_rua->media_info.a_queue);
	p_rua->media_info.a_queue = NULL;
}

void * rtsp_media_live_audio_capture_thread(void * argv)
{
	RSUA * p_rua = (RSUA *)argv;

	rtsp_media_live_audio_capture(p_rua);
	
	p_rua->audio_thread = 0;

	return NULL;
}

void rtsp_media_live_send_thread(RSUA * p_rua)
{
#ifdef RTSP_METADATA
	if (p_rua->media_info.metadata && p_rua->channels[AV_METADATA_CH].setup)
	{
		p_rua->media_info.m_thread = sys_os_create_thread((void *)rtsp_media_metadata_thread, (void*)p_rua);
	}
#endif

	if (p_rua->media_info.has_video)
	{
		if (p_rua->media_info.has_audio)
		{
			p_rua->audio_thread = sys_os_create_thread((void *)rtsp_media_live_audio_capture_thread, (void *)p_rua);
		}

		rtsp_media_live_video_capture(p_rua);

		if (p_rua->media_info.has_audio)
		{
			// wait audio capture thread exit ...
			while (p_rua->audio_thread)
			{
				usleep(20*1000);
			}
		}
	}
	else if (p_rua->media_info.has_audio)
	{
		rtsp_media_live_audio_capture(p_rua);
	}

#ifdef RTSP_METADATA
    if (p_rua->media_info.metadata && p_rua->channels[AV_METADATA_CH].setup)
    {
        while (p_rua->media_info.m_thread)
    	{
    		usleep(20*1000);
    	}
    }
#endif
}
#endif

void rtsp_media_send_thread(void * rua)
{
	RSUA * p_rua = (RSUA *)rua;


#ifdef MEDIA_LIVE
	if (p_rua->media_info.is_live)
	{
		rtsp_media_live_send_thread(p_rua);
	}
	else
#endif


    {
    }

	p_rua->rtp_thread = 0;
}






