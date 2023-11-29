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

#include "sys_inc.h"
#include "media_format.h"
#include "media_info.h"
#include "hqueue.h"
#include "rtmp_media.h"
#include "rtmp_tx.h"
#include "rtmp_pusher.h"
#include "common.h"
typedef struct
{
   uint8 * buff;                   // raw data buffer
   uint8 * data;                   // audio and video data
   int     size;                   // audio and video data size
   int     nbsamples;              // the number of audio samples
   int     waitnext;               // wait for next packet, used to control the rate of audio and video transmission 
} UA_PACKET_RTMP;

/***************************************************************************************/

void rtmp_media_fix_audio_param(RMPUA * p_rua)
{
    if (AUDIO_CODEC_G711A == p_rua->media_info.a_info.codec)
	{
		p_rua->media_info.a_info.samplerate = 8000;
		return;
	}
	else if (AUDIO_CODEC_G711U == p_rua->media_info.a_info.codec)
	{
		p_rua->media_info.a_info.samplerate = 8000;
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
		if (p_rua->media_info.a_info.samplerate <= sample_rates[i])
		{
			p_rua->media_info.a_info.samplerate = sample_rates[i];
			break;
		}
	}

	if (i == sample_rate_num)
	{
		p_rua->media_info.a_info.samplerate = 48000;
	}
}

void rtmp_media_clear_queue(HQUEUE * queue)
{
	UA_PACKET_RTMP packet;
	
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

void * rtmp_media_video_thread(void * argv)
{
	RMPUA * p_rua = (RMPUA *)argv;
	UA_PACKET_RTMP packet;
    int sret = -1;
	lint64 cur_delay = 0;
	lint64 pre_delay = 0;
	int timeout = 1000000.0 / p_rua->media_info.v_info.framerate;
	uint32 cur_time = 0;
	uint32 pre_time = 0;

	while (p_rua->rtp_tx)
	{
		if (hqBufGet(p_rua->media_info.v_queue, (char *)&packet))
		{
			if (packet.data == NULL || packet.size == 0)
			{
				break;
			}

			sys_os_mutex_enter(p_rua->mutex);

			if (VIDEO_CODEC_H264 == p_rua->media_info.v_info.codec)
			{
			    sret = rtmp_h26x_tx(p_rua, packet.data, packet.size, sys_os_get_ms() - p_rua->start_ts);
			}
		    else if (VIDEO_CODEC_H265 == p_rua->media_info.v_info.codec)
		    {
		        sret = rtmp_h26x_tx(p_rua, packet.data, packet.size, sys_os_get_ms() - p_rua->start_ts);
		    }
		    
			sys_os_mutex_leave(p_rua->mutex);

			free(packet.buff);

            if (sret < 0)
			{
				rtmp_commit_reconn(p_rua);
				break;
			}
			
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

	rtmp_media_clear_queue(p_rua->media_info.v_queue);

	p_rua->media_info.v_thread = 0;

    log_print(HT_LOG_DBG, "%s, exit\r\n", __FUNCTION__);
    
	return NULL;
}

void * rtmp_media_audio_thread(void * argv)
{
	RMPUA * p_rua = (RMPUA *)argv;
	UA_PACKET_RTMP packet;
	int sret = -1;
	int samplerate = p_rua->media_info.a_info.samplerate;
	lint64 cur_delay = 0;
	lint64 pre_delay = 0;
	uint32 cur_time = 0;
	uint32 pre_time = 0;

	while (p_rua->rtp_tx)
	{
		if (hqBufGet(p_rua->media_info.a_queue, (char *)&packet))
		{
			if (packet.data == NULL || packet.size == 0)
			{
				break;
			}
			
			sys_os_mutex_enter(p_rua->mutex);

            if (packet.nbsamples > 0)
            {
			    p_rua->audio_ts += 1000.0 * packet.nbsamples / samplerate;
			}
			else
			{
			    p_rua->audio_ts = sys_os_get_ms() - p_rua->start_ts;
			}

            if (AUDIO_CODEC_AAC == p_rua->media_info.a_info.codec)
			{
			    sret = rtmp_aac_tx(p_rua, packet.data, packet.size, p_rua->audio_ts);
			}
			else if (AUDIO_CODEC_G711A == p_rua->media_info.a_info.codec)
			{
			    sret = rtmp_g711a_tx(p_rua, packet.data, packet.size, p_rua->audio_ts);
			}
			else if (AUDIO_CODEC_G711U == p_rua->media_info.a_info.codec)
			{
			    sret = rtmp_g711u_tx(p_rua, packet.data, packet.size, p_rua->audio_ts);
			}
			
			sys_os_mutex_leave(p_rua->mutex);

			free(packet.buff);

            if (sret < 0)
			{
			    if (!p_rua->media_info.has_video)
			    {
				    rtmp_commit_reconn(p_rua);
				}
				
				break;
			}
			
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

	rtmp_media_clear_queue(p_rua->media_info.a_queue);

	p_rua->media_info.a_thread = 0;

    log_print(HT_LOG_DBG, "%s, exit\r\n", __FUNCTION__);
    
	return NULL;
}

void rtmp_media_put_video(RMPUA * p_rua, uint8 *data, int size, int waitnext = 1)
{
	UA_PACKET_RTMP packet;

    if (NULL == data || 0 == size)
    {
        memset(&packet, 0, sizeof(packet));

		hqBufPut(p_rua->media_info.v_queue, (char *)&packet);
    }
    else if (p_rua->rtp_tx)
	{
		packet.buff = (uint8*)malloc(size + RESV_HEADER_SIZE);
		packet.data = packet.buff + RESV_HEADER_SIZE;

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
}

void rtmp_media_put_audio(RMPUA * p_rua, uint8 *data, int size, int nbsamples, int waitnext = 1)
{
	UA_PACKET_RTMP packet;

    if (NULL == data || 0 == size)
    {
        memset(&packet, 0, sizeof(packet));

		hqBufPut(p_rua->media_info.a_queue, (char *)&packet);
    }
    else if (p_rua->rtp_tx)
	{
		packet.buff = (uint8*)malloc(size + RESV_HEADER_SIZE); // skip forward header
		packet.data = packet.buff + RESV_HEADER_SIZE;

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
}

BOOL rtmp_channel_open(RMPUA * p_rua)
{
	int err;
	RTMP * p_rtmp = RTMP_Alloc();
	if (NULL == p_rtmp)
	{
		log_print(HT_LOG_ERR, "%s, rtmp alloc failed\r\n", __FUNCTION__);
		return FALSE;
	}

	RTMP_Init(p_rtmp);

	err = RTMP_SetupURL(p_rtmp, p_rua->dst_url);
	if (err <= 0)
	{
		log_print(HT_LOG_ERR, "%s, rtmp setup url failed\r\n", __FUNCTION__);
		goto rtmp_open_err;
	}

	if (p_rtmp->Link.pubUser.av_val && p_rtmp->Link.pubUser.av_len > 0)
	{
	    strncpy(p_rua->user, p_rtmp->Link.pubUser.av_val, p_rtmp->Link.pubUser.av_len);
	}
	
	if (p_rtmp->Link.pubPasswd.av_val && p_rtmp->Link.pubPasswd.av_len > 0)
	{
	    strncpy(p_rua->pass, p_rtmp->Link.pubPasswd.av_val, p_rtmp->Link.pubPasswd.av_len);
	}

	RTMP_EnableWrite(p_rtmp);

    // setup the auth information
    p_rtmp->Link.lFlags |= RTMP_LF_AUTH;
    p_rtmp->Link.flashVer = RTMP_FlashVer;
    p_rtmp->Link.pubUser.av_val = p_rua->user;
    p_rtmp->Link.pubUser.av_len = strlen(p_rua->user);
    p_rtmp->Link.pubPasswd.av_val = p_rua->pass;
    p_rtmp->Link.pubPasswd.av_len = strlen(p_rua->pass);
    
	if (!RTMP_Connect(p_rtmp, NULL))
	{
		log_print(HT_LOG_ERR, "%s, RTMP_Connect failed\r\n", __FUNCTION__);
		goto rtmp_open_err;
	}

	err = RTMP_ConnectStream(p_rtmp, 0);
	if (err <= 0)
	{
		log_print(HT_LOG_ERR, "%s, RTMP_ConnectStream failed\r\n", __FUNCTION__);
		goto rtmp_open_err;
	}

	RTMP_SendChunkSize(p_rtmp, 4096);

	p_rtmp->m_outChunkSize = 4096;

	p_rua->rtmp = p_rtmp;
	p_rua->mutex = sys_os_create_mutex();
    p_rua->start_ts = sys_os_get_ms();
    
    rtmp_send_metadata((void *)p_rua);
    
	return TRUE;

rtmp_open_err:

	if (p_rtmp)
	{
		RTMP_Close(p_rtmp);
		RTMP_Free(p_rtmp);
	}

	return FALSE;
}

void rtmp_channel_close(RMPUA * p_rua)
{
	if (p_rua->rtmp)
	{
		RTMP_Close(p_rua->rtmp);
		RTMP_Free(p_rua->rtmp);
		p_rua->rtmp = NULL;
	}

	if (p_rua->mutex)
	{
		sys_os_destroy_sig_mutex(p_rua->mutex);
		p_rua->mutex = NULL;
	}
}

void rtmp_media_create_video_queue(RMPUA * p_rua)
{
	if (p_rua->media_info.has_video)
	{
		p_rua->media_info.v_queue = hqCreate(10, sizeof(UA_PACKET_RTMP), HQ_GET_WAIT);
		p_rua->media_info.v_thread = sys_os_create_thread((void *)rtmp_media_video_thread, (void*)p_rua);
	}
}

void rtmp_media_create_audio_queue(RMPUA * p_rua)
{
	if (p_rua->media_info.has_audio)
	{
		p_rua->media_info.a_queue = hqCreate(10, sizeof(UA_PACKET_RTMP), HQ_GET_WAIT);
		p_rua->media_info.a_thread = sys_os_create_thread((void *)rtmp_media_audio_thread, (void*)p_rua);
	}
}

void rtmp_media_create_queue(RMPUA * p_rua)
{
	rtmp_media_create_video_queue(p_rua);

	rtmp_media_create_audio_queue(p_rua);
}

void rtmp_media_free_video_queue(RMPUA * p_rua)
{
	if (p_rua->media_info.has_video)
	{
		rtmp_media_put_video(p_rua, NULL, 0, 0);
	}

	while (p_rua->media_info.v_thread)
	{
		usleep(10*1000);
	}

	rtmp_media_clear_queue(p_rua->media_info.v_queue);
	
	hqDelete(p_rua->media_info.v_queue);
	p_rua->media_info.v_queue = NULL;
}

void rtmp_media_free_audio_queue(RMPUA * p_rua)
{
	if (p_rua->media_info.has_audio)
	{
		rtmp_media_put_audio(p_rua, NULL, 0, 0, 0);
	}

	while (p_rua->media_info.a_thread)
	{
		usleep(10*1000);
	}

	rtmp_media_clear_queue(p_rua->media_info.a_queue);
	
	hqDelete(p_rua->media_info.a_queue);
	p_rua->media_info.a_queue = NULL;
}


void rtmp_media_free_queue(RMPUA * p_rua)
{
	rtmp_media_free_video_queue(p_rua);

	rtmp_media_free_audio_queue(p_rua);
}


#ifdef MEDIA_FILE

BOOL rtmp_parse_file_url(RMPUA * p_rua, const char * url)
{
	p_rua->media_info.file_demuxer = new CFileDemux(url, g_rtmp_cfg.loop_nums);
	if (NULL == p_rua->media_info.file_demuxer)
	{
		return FALSE;
	}
	
	p_rua->media_info.is_file = 1;
	p_rua->media_info.has_video = p_rua->media_info.file_demuxer->hasVideo();
	p_rua->media_info.has_audio = p_rua->media_info.file_demuxer->hasAudio();

	if (!p_rua->media_info.has_audio && !p_rua->media_info.has_video)
	{
		printf("%s open failed!!!\r\n", url);
		
		log_print(HT_LOG_ERR, "%s, %s open failed\r\n", __FUNCTION__, url);		
		return FALSE;
	}

	return TRUE;
}

BOOL rtmp_media_file_init(RMPUA * p_rua)
{
	if (p_rua->media_info.has_video)
	{
        if (VIDEO_CODEC_NONE == p_rua->media_info.v_info.codec)
		{
			p_rua->media_info.v_info.codec = VIDEO_CODEC_H264;
		}
		
		if (p_rua->media_info.v_info.width <= 0 || p_rua->media_info.v_info.height <= 0)
		{
			p_rua->media_info.v_info.width = p_rua->media_info.file_demuxer->getWidth();
			p_rua->media_info.v_info.height = p_rua->media_info.file_demuxer->getHeight();
		}

		if (p_rua->media_info.v_info.framerate <= 0)
		{
			p_rua->media_info.v_info.framerate = p_rua->media_info.file_demuxer->getFramerate();
		}

		if (0 == p_rua->media_info.v_info.framerate)
		{
		    p_rua->media_info.v_info.framerate = 25;
		}

		if (p_rua->media_info.file_demuxer->setVideoFormat(p_rua->media_info.v_info.codec,
												p_rua->media_info.v_info.width,
												p_rua->media_info.v_info.height,
                                                p_rua->media_info.v_info.framerate,
                                                p_rua->media_info.v_info.bitrate) == FALSE)
		{
			log_print(HT_LOG_ERR, "%s, setVideoFormat failed\r\n", __FUNCTION__);
			
			delete p_rua->media_info.file_demuxer;
			p_rua->media_info.file_demuxer = NULL;
			return FALSE;
		}
	}

	if (p_rua->media_info.has_audio)
	{
        if (AUDIO_CODEC_NONE == p_rua->media_info.a_info.codec)
		{
			p_rua->media_info.a_info.codec = AUDIO_CODEC_AAC;
		}
		
		if (0 == p_rua->media_info.a_info.samplerate)
		{
			p_rua->media_info.a_info.samplerate = p_rua->media_info.file_demuxer->getSamplerate();
		}

		if (0 == p_rua->media_info.a_info.channels)
		{
			p_rua->media_info.a_info.channels = p_rua->media_info.file_demuxer->getChannels();
		}

		if (p_rua->media_info.a_info.channels > 2)
		{
			p_rua->media_info.a_info.channels = 2;
		}

		rtmp_media_fix_audio_param(p_rua);

		if (p_rua->media_info.file_demuxer->setAudioFormat(p_rua->media_info.a_info.codec,
												p_rua->media_info.a_info.samplerate,
                                                p_rua->media_info.a_info.channels,
                                                p_rua->media_info.a_info.bitrate) == FALSE)
		{
			log_print(HT_LOG_ERR, "%s, setAudioFormat failed\r\n", __FUNCTION__);
			
			delete p_rua->media_info.file_demuxer;
			p_rua->media_info.file_demuxer = NULL;
			return FALSE;
		}
	}

    return (p_rua->media_info.has_video || p_rua->media_info.has_audio);
}

void rtmp_file_demux_callback(uint8 * data, int size, int type, int nbsamples, BOOL waitnext, void * pUserdata)
{
	RMPUA * p_rua = (RMPUA *)pUserdata;

	if (type == DATA_TYPE_VIDEO && p_rua->media_info.has_video)
	{
		rtmp_media_put_video(p_rua, data, size, waitnext);
	}
	else if (type == DATA_TYPE_AUDIO && p_rua->media_info.has_audio)
	{
		rtmp_media_put_audio(p_rua, data, size, nbsamples, waitnext);		
	}
}

void rtmp_media_file_send(RMPUA * p_rua)
{
	CFileDemux * pDemux = p_rua->media_info.file_demuxer;

	pDemux->setCallback(rtmp_file_demux_callback, p_rua);

    if (p_rua->media_info.has_video)
	{
		p_rua->media_info.v_queue = hqCreate(30, sizeof(UA_PACKET_RTMP), HQ_PUT_WAIT | HQ_GET_WAIT);
		p_rua->media_info.v_thread = sys_os_create_thread((void *)rtmp_media_video_thread, (void*)p_rua);
	}

	if (p_rua->media_info.has_audio)
	{
		p_rua->media_info.a_queue = hqCreate(30, sizeof(UA_PACKET_RTMP), HQ_PUT_WAIT | HQ_GET_WAIT);
		p_rua->media_info.a_thread = sys_os_create_thread((void *)rtmp_media_audio_thread, (void*)p_rua);
	}

	while (p_rua->rtp_tx)
	{
		if (pDemux->readFrame() == FALSE)
		{
			break;
		}
	}

	rtmp_media_free_queue(p_rua);
}

void * rtmp_push_file_thread(void * argv)
{
	RMPUA * p_rua = (RMPUA *) argv;

	log_print(HT_LOG_DBG, "%s, enter ... \r\n", __FUNCTION__);

	if (NULL == p_rua->media_info.file_demuxer)
	{
		log_print(HT_LOG_ERR, "%s, open file %s failed\r\n", __FUNCTION__, p_rua->src_url);
		goto thread_exit;
	}

	if (!rtmp_channel_open(p_rua))
	{
		log_print(HT_LOG_ERR, "%s, rtmp_channel_open failed\r\n", __FUNCTION__);

		rtmp_commit_reconn(p_rua);
		goto thread_exit;
	}

	rtmp_media_file_send(p_rua);

thread_exit:

    if (p_rua->media_info.file_demuxer)
    {
    	delete p_rua->media_info.file_demuxer;
    	p_rua->media_info.file_demuxer = NULL;
	}

	rtmp_channel_close(p_rua);

	log_print(HT_LOG_DBG, "%s, exit ... \r\n", __FUNCTION__);

	printf("end pusher : \r\n\tsrc : %s\r\n\tdst : %s\r\n", p_rua->src_url, p_rua->dst_url);

	p_rua->tid_pusher = 0;

	log_print(HT_LOG_DBG, "%s, exit ... \r\n", __FUNCTION__);
		
	return NULL;	
}

#endif // MEDIA_FILE

#ifdef MEDIA_DEVICE

int rtmp_get_device_nums(int type)
{
    int nums = 0;
    
    if (0 == type)  // screen
    {
#if __WINDOWS_OS__		
		nums = CWScreenCapture::getDeviceNums();    
#elif defined(ANDROID)
#elif defined(IOS)
        nums = CMScreenCapture::getDeviceNums();
#elif __LINUX_OS__
		nums = CLScreenCapture::getDeviceNums();
#endif    
    }
    else if (1 == type) // video
    {
#if __WINDOWS_OS__		
		nums = CWVideoCapture::getDeviceNums();    
#elif defined(ANDROID)
        nums = CQVideoCapture::getDeviceNums();
#elif defined(IOS)
        nums = CMVideoCapture::getDeviceNums();
#elif __LINUX_OS__
		nums = CLVideoCapture::getDeviceNums();
#endif    
    }
    else if (2 == type) // audio
    {
#if __WINDOWS_OS__	
		nums = CDSoundAudioCapture::getDeviceNums();    
#elif defined(ANDROID)
        nums = CAAudioCapture::getDeviceNums();
#elif defined(IOS)
        nums = CMAudioCapture::getDeviceNums();
#elif __LINUX_OS__
		nums = CLAudioCapture::getDeviceNums();
#endif    
    }

    return nums;
}

int rtmp_get_video_device_index(char * name)
{
    int index = 0;
    
#if __WINDOWS_OS__
    index = CWVideoCapture::getDeviceIndex(name);
#elif defined(ANDROID)
    index = CQVideoCapture::getDeviceIndex(name);
#elif defined(IOS)
    index = CMVideoCapture::getDeviceIndex(name);
#elif __LINUX_OS__
    index = CLVideoCapture::getDeviceIndex(name);
#endif

    return index;
}

int rtmp_get_audio_device_index(char * name)
{
    int index = 0;
    
#if __WINDOWS_OS__
    index = CDSoundAudioCapture::getDeviceIndex(name);
#elif defined(ANDROID)
    index = CAAudioCapture::getDeviceIndex(name);
#elif defined(IOS)
    index = CMAudioCapture::getDeviceIndex(name);
#elif __LINUX_OS__
    index = CLAudioCapture::getDeviceIndex(name);
#endif

    return index;    
}

int rtmp_parse_device_index(char * buff, int flag)
{
    int i = 0;
    int index = 0;
    char name[256];
    char * p = buff;
		
    if (*p != '\0')
    {
        if (*p == '=')
        {
            p++;
            
            while (*p != '\0')
        	{
        		if (*p != '"' && *p != '\'')
        		{
        			name[i++] = *p;
        		}

        		p++;
        	}

        	name[i] = '\0';

        	if (i > 0)
        	{
        	    url_decode(name, name, strlen(name));
        	    
        	    if (0 == flag)  // video
        	    {
        	        index = rtmp_get_video_device_index(name);
        	    }
        	    else if (1 == flag) // audio
        	    {
        	        index = rtmp_get_audio_device_index(name);
        	    }
        	}
        }
        else
        {
    	    index = atoi(p);
    	}
    }	

    return index;
}

BOOL rtmp_parse_device_url(RMPUA * p_rua, const char * p_url)
{
	int i = 0;
	char buff[512];
	char * p;
	
	while (p_url[i] != '\0')
	{
		if (p_url[i] == '+')
		{
			break;
		}
		else
		{
			buff[i] = p_url[i];
		}

		i++;
	}

	buff[i] = '\0';

	int screen = 0;
	int vdevice = 0;
	int adevice = 0;
	int vindex = 0;
	int aindex = 0;
	
	if (strncasecmp(buff, "videodevice", strlen("videodevice")) == 0)
	{
		vdevice = 1;
		vindex = rtmp_parse_device_index(buff + strlen("videodevice"), 0);
	}
	else if (strncasecmp(buff, "audiodevice", strlen("audiodevice")) == 0)
	{
		adevice = 1;
		aindex = rtmp_parse_device_index(buff + strlen("audiodevice"), 1);
	}
	else if (strncasecmp(buff, "screenlive", strlen("screenlive")) == 0)
	{
		screen = 1;
        p = buff + strlen("screenlive");
		if (*p != '\0')
		{
			vindex = atoi(p);
		}
	}
	else
	{
		return FALSE;
	}

	if (p_url[i] == '+')
	{
		strcpy(buff, p_url+i+1);

		if (strncasecmp(buff, "videodevice", strlen("videodevice")) == 0)
		{
			vdevice = 1;
			vindex = rtmp_parse_device_index(buff + strlen("videodevice"), 0);
		}
		else if (strncasecmp(buff, "audiodevice", strlen("audiodevice")) == 0)
		{
			adevice = 1;
			aindex = rtmp_parse_device_index(buff + strlen("audiodevice"), 1);
		}
		else if (strncasecmp(buff, "screenlive", strlen("screenlive")) == 0)
		{
			screen = 1;
			p = buff + strlen("screenlive");
    		if (*p != '\0')
    		{
    			vindex = atoi(p);
    		}
		}
		else
		{
			return FALSE;
		}
	}

    p_rua->media_info.is_device = 1;
    
	if (vdevice)
	{
		p_rua->media_info.has_video = 1;
		p_rua->media_info.v_index = vindex;
	}

	if (adevice)
	{
		p_rua->media_info.has_audio = 1;
		p_rua->media_info.a_index = aindex;
	}

	if (screen)
	{
		p_rua->media_info.has_video = 1;
		p_rua->media_info.is_screen = 1;
		p_rua->media_info.v_index = vindex;
	}

	return TRUE;
}

void rtmp_media_video_callback(uint8 * data, int size, int waitnext, void * pUserdata)
{
	rtmp_media_put_video((RMPUA *)pUserdata, data, size, 0);
}

void rtmp_media_audio_callback(uint8 * data, int size, int nbsamples, void * pUserdata)
{
	rtmp_media_put_audio((RMPUA *)pUserdata, data, size, nbsamples, 0);
}

BOOL rtmp_media_device_screen_init(RMPUA * p_rua)
{
    if (p_rua->media_info.v_index >= rtmp_get_device_nums(0))
	{
		log_print(HT_LOG_ERR, "%s, index=%d, device nums=%d\r\n", 
		    __FUNCTION__, p_rua->media_info.v_index, rtmp_get_device_nums(0));
		return FALSE;
	}

    if (VIDEO_CODEC_NONE == p_rua->media_info.v_info.codec)
    {
        p_rua->media_info.v_info.codec = VIDEO_CODEC_H264;
    }
	    
	if (0 == p_rua->media_info.v_info.framerate)
	{
		p_rua->media_info.v_info.framerate = 25;
	}

    // get screen capture instance

#if __WINDOWS_OS__
        p_rua->media_info.screen_capture = CWScreenCapture::getInstance(p_rua->media_info.v_index);
#elif defined(ANDROID)
    // android does not yet support
#elif defined(IOS)
    p_rua->media_info.screen_capture = CMScreenCapture::getInstance(p_rua->media_info.v_index);
#elif __LINUX_OS__
	p_rua->media_info.screen_capture = CLScreenCapture::getInstance(p_rua->media_info.v_index);
#endif

    if (NULL == p_rua->media_info.screen_capture)
	{
	    log_print(HT_LOG_ERR, "%s, get screen capture object failed\r\n", __FUNCTION__);
	    return FALSE;
	}
	else if (p_rua->media_info.screen_capture->initCapture(p_rua->media_info.v_info.codec,
                                           p_rua->media_info.v_info.width,
                                           p_rua->media_info.v_info.height,
                                           p_rua->media_info.v_info.framerate,
                                           p_rua->media_info.v_info.bitrate) == FALSE)
	{
		log_print(HT_LOG_ERR, "%s, init screen capture failed\r\n", __FUNCTION__);
		p_rua->media_info.screen_capture->freeInstance(p_rua->media_info.v_index);
		p_rua->media_info.screen_capture = NULL;
		return FALSE;
	}

	// get video size
	if (p_rua->media_info.v_info.width == 0 || p_rua->media_info.v_info.height == 0)
	{
		p_rua->media_info.v_info.width = p_rua->media_info.screen_capture->getWidth();
		p_rua->media_info.v_info.height = p_rua->media_info.screen_capture->getHeight();
	}

	return TRUE;
}

BOOL rtmp_media_device_video_init(RMPUA * p_rua)
{
    if (p_rua->media_info.v_index >= rtmp_get_device_nums(1))
	{
		log_print(HT_LOG_ERR, "%s, index=%d, device nums=%d\r\n", 
		    __FUNCTION__, p_rua->media_info.v_index, rtmp_get_device_nums(1));
		return FALSE;
	}

    if (VIDEO_CODEC_NONE == p_rua->media_info.v_info.codec)
    {
        p_rua->media_info.v_info.codec = VIDEO_CODEC_H264;
    }
    
	if (0 == p_rua->media_info.v_info.framerate)
	{
		p_rua->media_info.v_info.framerate = 25;
	}

#if __WINDOWS_OS__
	p_rua->media_info.video_capture = CWVideoCapture::getInstance(p_rua->media_info.v_index);
#elif defined(ANDROID)
	p_rua->media_info.video_capture = CQVideoCapture::getInstance(p_rua->media_info.v_index);
#elif defined(IOS)
    p_rua->media_info.video_capture = CMVideoCapture::getInstance(p_rua->media_info.v_index);
#elif __LINUX_OS__
	p_rua->media_info.video_capture = CLVideoCapture::getInstance(p_rua->media_info.v_index);
#endif

	if (NULL == p_rua->media_info.video_capture)
	{
	    log_print(HT_LOG_ERR, "%s, get video capture object failed\r\n", __FUNCTION__);
	    return FALSE;
	}
	else if (p_rua->media_info.video_capture->initCapture(p_rua->media_info.v_info.codec,
                                          p_rua->media_info.v_info.width,
                                          p_rua->media_info.v_info.height,
                                          p_rua->media_info.v_info.framerate,
                                          p_rua->media_info.v_info.bitrate) == FALSE)
	{
		log_print(HT_LOG_ERR, "%s, init video capture failed\r\n", __FUNCTION__);
		p_rua->media_info.video_capture->freeInstance(p_rua->media_info.v_index);
		p_rua->media_info.video_capture = NULL;
		return FALSE;
	}

	// get video sizes
	if (p_rua->media_info.v_info.width == 0 || p_rua->media_info.v_info.height == 0)
	{
		p_rua->media_info.v_info.width = p_rua->media_info.video_capture->getWidth();
		p_rua->media_info.v_info.height = p_rua->media_info.video_capture->getHeight();
	}

	return TRUE;
}

BOOL rtmp_media_device_audio_init(RMPUA * p_rua)
{
    if (p_rua->media_info.a_index >= rtmp_get_device_nums(2))
	{
		log_print(HT_LOG_ERR, "%s, index=%d, device nums=%d\r\n", 
		    __FUNCTION__, p_rua->media_info.a_index, rtmp_get_device_nums(2));
		return FALSE;
	}

	if (AUDIO_CODEC_NONE == p_rua->media_info.a_info.codec)
    {
        p_rua->media_info.a_info.codec = AUDIO_CODEC_AAC;
    }
    
	if (0 == p_rua->media_info.a_info.samplerate)
	{
		p_rua->media_info.a_info.samplerate = 44100;
	}

	if (0 == p_rua->media_info.a_info.channels)
	{
		p_rua->media_info.a_info.channels = 2;
	}

	rtmp_media_fix_audio_param(p_rua);

#if __WINDOWS_OS__
	p_rua->media_info.audio_capture = CDSoundAudioCapture::getInstance(p_rua->media_info.a_index);
#elif defined(ANDROID)
	p_rua->media_info.audio_capture = CAAudioCapture::getInstance(p_rua->media_info.a_index);
#elif defined(IOS)
    p_rua->media_info.audio_capture = CMAudioCapture::getInstance(p_rua->media_info.a_index);
#elif __LINUX_OS__
	p_rua->media_info.audio_capture = CLAudioCapture::getInstance(p_rua->media_info.a_index);
#endif

	if (NULL == p_rua->media_info.audio_capture)
	{
	    log_print(HT_LOG_ERR, "%s, get audio capture object failed\r\n", __FUNCTION__);
	    return FALSE;
	}
	else if (p_rua->media_info.audio_capture->initCapture(p_rua->media_info.a_info.codec,
                                          p_rua->media_info.a_info.samplerate,
                                          p_rua->media_info.a_info.channels,
                                          p_rua->media_info.a_info.bitrate) == FALSE)
	{
		log_print(HT_LOG_ERR, "%s, init audio capture failed\r\n", __FUNCTION__);
		p_rua->media_info.audio_capture->freeInstance(p_rua->media_info.a_index);
		p_rua->media_info.audio_capture = NULL;
		return FALSE;
	}

	return TRUE;
}

BOOL rtmp_media_device_init(RMPUA * p_rua)
{
    BOOL vret = FALSE, aret = FALSE;
    
    if (p_rua->media_info.has_video)
    {
        if (p_rua->media_info.is_screen)
        {
            vret = rtmp_media_device_screen_init(p_rua);
        }
        else
        {
            vret = rtmp_media_device_video_init(p_rua);
        }
    }

    if (p_rua->media_info.has_audio)
    {
        aret = rtmp_media_device_audio_init(p_rua);
    }

    return (vret || aret);
}

void rtmp_media_device_screen_send(RMPUA * p_rua)
{
	CScreenCapture * capture = p_rua->media_info.screen_capture;

	rtmp_media_create_video_queue(p_rua);

	capture->addCallback(rtmp_media_video_callback, p_rua);
	capture->startCapture();
	
	while (p_rua->rtp_tx)
	{
		usleep(200*1000);
	}

	capture->delCallback(rtmp_media_video_callback, p_rua);
	capture->freeInstance(p_rua->media_info.v_index);
	p_rua->media_info.screen_capture = NULL;

	rtmp_media_free_video_queue(p_rua);
}

void rtmp_media_device_video_send(RMPUA * p_rua)
{
    CVideoCapture * capture = p_rua->media_info.video_capture;

	rtmp_media_create_video_queue(p_rua);

	capture->addCallback(rtmp_media_video_callback, p_rua);
	capture->startCapture();
	
	while (p_rua->rtp_tx)
	{
		usleep(200*1000);
	}

	capture->delCallback(rtmp_media_video_callback, p_rua);
	capture->freeInstance(p_rua->media_info.v_index);
	p_rua->media_info.video_capture = NULL;

	rtmp_media_free_video_queue(p_rua);
}

void rtmp_media_device_audio_send(RMPUA * p_rua)
{
    CAudioCapture * capture = p_rua->media_info.audio_capture;

	rtmp_media_create_audio_queue(p_rua);

	capture->addCallback(rtmp_media_audio_callback, p_rua);
	capture->startCapture();
	
	while (p_rua->rtp_tx)
	{
		usleep(200*1000);
	}

	capture->delCallback(rtmp_media_audio_callback, p_rua);
	capture->freeInstance(p_rua->media_info.a_index);
	p_rua->media_info.audio_capture = NULL;

	rtmp_media_free_audio_queue(p_rua);
}

void * rtmp_media_device_audio_thread(void * argv)
{
	RMPUA * p_rua = (RMPUA *) argv;

	rtmp_media_device_audio_send(p_rua);
	
	p_rua->tid_audio = 0;

	return NULL;
}

void * rtmp_push_device_thread(void * argv)
{
	RMPUA * p_rua = (RMPUA *) argv;

	log_print(HT_LOG_DBG, "%s, enter ...\r\n", __FUNCTION__);
	
	if (!rtmp_channel_open(p_rua))
	{
	    if (p_rua->media_info.has_video)
    	{
    	    if (p_rua->media_info.screen_capture)
    	    {
            	p_rua->media_info.screen_capture->freeInstance(p_rua->media_info.v_index);
            	p_rua->media_info.screen_capture = NULL;
        	}
        	
    	    if (p_rua->media_info.video_capture)
    	    {
            	p_rua->media_info.video_capture->freeInstance(p_rua->media_info.v_index);
            	p_rua->media_info.video_capture = NULL;
        	}
    	}

    	if (p_rua->media_info.has_audio)
    	{
    	    if (p_rua->media_info.audio_capture)
    	    {
            	p_rua->media_info.audio_capture->freeInstance(p_rua->media_info.a_index);
            	p_rua->media_info.audio_capture = NULL;
        	}
    	}
	
		log_print(HT_LOG_ERR, "%s, rtmp_channel_open failed\r\n", __FUNCTION__);

		rtmp_commit_reconn(p_rua);
		goto thread_exit;
	}
	
	if (p_rua->media_info.has_video)
	{
		if (p_rua->media_info.has_audio)
		{
			p_rua->tid_audio = sys_os_create_thread((void *)rtmp_media_device_audio_thread, (void *)p_rua);
		}

		if (p_rua->media_info.is_screen)
		{
			rtmp_media_device_screen_send(p_rua);
		}
		else
		{
			rtmp_media_device_video_send(p_rua);
		}

		if (p_rua->media_info.has_audio)
		{
			// wait audio send thread exit ...
			while (p_rua->tid_audio)
			{
				usleep(200*1000);
			}
		}
	}
	else if (p_rua->media_info.has_audio)
	{
		rtmp_media_device_audio_send(p_rua);
	}
	
thread_exit:

	rtmp_channel_close(p_rua);

	printf("end pusher : \r\n\tsrc : %s\r\n\tdst : %s\r\n", p_rua->src_url, p_rua->dst_url);

	p_rua->tid_pusher = 0;
	
	log_print(HT_LOG_DBG, "%s, exit ...\r\n", __FUNCTION__);
	
	return NULL;
}

#endif // MEDIA_DEVICE

#ifdef MEDIA_LIVE

BOOL rtmp_parse_live_url(RMPUA * p_rua, const char * url)
{
	// todo : here add your init code ...

	BOOL ret = FALSE;

	if (strcasecmp(url, "live") == 0)
	{
		p_rua->media_info.is_live = 1;
		p_rua->media_info.has_video = 1;
		p_rua->media_info.v_index = 0;
		p_rua->media_info.v_info.framerate = 30;
    	p_rua->media_info.v_info.width = 1920;
    	p_rua->media_info.v_info.height = 1080;

    	// todo : if have audio, uncomment the statements

    	// p_rua->media_info.has_audio = 1;
    	// p_rua->media_info.a_index = 0;
    	// p_rua->media_info.a_info.samplerate = 8000;
    	// p_rua->media_info.a_info.channels = 1;

		ret = TRUE;
	}

	return ret;
}

void rtmp_media_live_video_callback(uint8 * data, int size, void * pUserdata)
{
	rtmp_media_put_video((RMPUA *)pUserdata, data, size, 0);
}

void rtmp_media_live_audio_callback(uint8 * data, int size, int nbsamples, void * pUserdata)
{
	rtmp_media_put_audio((RMPUA *)pUserdata, data, size, nbsamples, 0);
}

BOOL rtmp_media_live_video_init(RMPUA * p_rua)
{
	if (p_rua->media_info.v_index >= CLiveVideo_rtmp::getStreamNums())
	{
		log_print(HT_LOG_ERR, "%s, index=%d, stream nums=%d\r\n", __FUNCTION__, 
    		p_rua->media_info.v_index, CLiveVideo_rtmp::getStreamNums());
        return FALSE;
	}

    if (VIDEO_CODEC_NONE == p_rua->media_info.v_info.codec)
    {
        p_rua->media_info.v_info.codec = VIDEO_CODEC_H264;
    }
    
	if (0 == p_rua->media_info.v_info.framerate)
	{
		p_rua->media_info.v_info.framerate = 25;
	}

	if (0 == p_rua->media_info.v_info.width || 0 == p_rua->media_info.v_info.height)
	{
	    p_rua->media_info.v_info.width = 1280;
	    p_rua->media_info.v_info.height = 720;
	}

    p_rua->media_info.live_video = CLiveVideo_rtmp::getInstance(p_rua->media_info.v_index);
    
	if (NULL == p_rua->media_info.live_video)
	{
	    log_print(HT_LOG_ERR, "%s, get live video capture object failed\r\n", __FUNCTION__);
	    return FALSE;
	}
	else if (p_rua->media_info.live_video->initCapture(p_rua->media_info.v_info.codec,
									   p_rua->media_info.v_info.width,
									   p_rua->media_info.v_info.height,
									   p_rua->media_info.v_info.framerate,
									   p_rua->media_info.v_info.bitrate) == FALSE)
	{
		log_print(HT_LOG_ERR, "%s, init live video capture failed\r\n", __FUNCTION__);
		p_rua->media_info.live_video->freeInstance(p_rua->media_info.v_index);
		p_rua->media_info.live_video = NULL;
		return FALSE;
	}

	return TRUE;
}

BOOL rtmp_media_live_audio_init(RMPUA * p_rua)
{
	if (p_rua->media_info.a_index >= CLiveAudio_rtmp::getStreamNums())
	{
		log_print(HT_LOG_ERR, "%s, index=%d, stream nums=%d\r\n", __FUNCTION__, 
			p_rua->media_info.a_index, CLiveAudio_rtmp::getStreamNums());
		return FALSE;
	}

	if (AUDIO_CODEC_NONE == p_rua->media_info.a_info.codec)
    {
        p_rua->media_info.a_info.codec = AUDIO_CODEC_AAC;
    }
    
	if (0 == p_rua->media_info.a_info.samplerate)
	{
		p_rua->media_info.a_info.samplerate = 8000;
	}

	if (0 == p_rua->media_info.a_info.channels)
	{
		p_rua->media_info.a_info.channels = 2;
	}

	rtmp_media_fix_audio_param(p_rua);

    p_rua->media_info.live_audio = CLiveAudio_rtmp::getInstance(p_rua->media_info.a_index);
    
	if (NULL == p_rua->media_info.live_audio)
	{
	    log_print(HT_LOG_ERR, "%s, get live audio capture object failed\r\n", __FUNCTION__);
	    return FALSE;
	}
	else if (p_rua->media_info.live_audio->initCapture(p_rua->media_info.a_info.codec,
									   p_rua->media_info.a_info.samplerate,
									   p_rua->media_info.a_info.channels,
									   p_rua->media_info.a_info.bitrate) == FALSE)
	{
		log_print(HT_LOG_ERR, "%s, init live audio capture failed\r\n", __FUNCTION__);
		p_rua->media_info.live_audio->freeInstance(p_rua->media_info.a_index);
		p_rua->media_info.live_audio = NULL;
		return FALSE;
	}

	return TRUE;
}

BOOL rtmp_media_live_init(RMPUA * p_rua)
{
    BOOL vret = FALSE, aret = FALSE;
    
    if (p_rua->media_info.has_video)
    {
        vret = rtmp_media_live_video_init(p_rua);
    }

    if (p_rua->media_info.has_audio)
    {
        aret = rtmp_media_live_audio_init(p_rua);
    }

    return (vret || aret);
}

void rtmp_media_live_video_send(RMPUA * p_rua)
{
    CLiveVideo_rtmp * capture = p_rua->media_info.live_video;

	rtmp_media_create_video_queue(p_rua);

	capture->addCallback(rtmp_media_live_video_callback, p_rua);
	capture->startCapture();
	
	while (p_rua->rtp_tx)
	{
		usleep(200*1000);
	}

	capture->delCallback(rtmp_media_live_video_callback, p_rua);
	capture->freeInstance(p_rua->media_info.v_index);
	p_rua->media_info.live_video = NULL;

	rtmp_media_free_video_queue(p_rua);
}

void rtmp_media_live_audio_send(RMPUA * p_rua)
{
    CLiveAudio_rtmp * capture = p_rua->media_info.live_audio;

	rtmp_media_create_audio_queue(p_rua);

	capture->addCallback(rtmp_media_live_audio_callback, p_rua);
	capture->startCapture();
	
	while (p_rua->rtp_tx)
	{
		usleep(200*1000);
	}

	capture->delCallback(rtmp_media_live_audio_callback, p_rua);
	capture->freeInstance(p_rua->media_info.a_index);
	p_rua->media_info.live_audio = NULL;

	rtmp_media_free_audio_queue(p_rua);
}

void * rtmp_media_live_audio_thread(void * argv)
{
	RMPUA * p_rua = (RMPUA *) argv;

	rtmp_media_live_audio_send(p_rua);
	
	p_rua->tid_audio = 0;

	return NULL;
}

void * rtmp_push_live_thread(void * argv)
{
	RMPUA * p_rua = (RMPUA *) argv;

	log_print(HT_LOG_DBG, "%s, enter ...\r\n", __FUNCTION__);
	
	if (!rtmp_channel_open(p_rua))
	{
	    if (p_rua->media_info.has_video)
    	{
    	    if (p_rua->media_info.live_video)
    	    {
            	p_rua->media_info.live_video->freeInstance(p_rua->media_info.v_index);
            	p_rua->media_info.live_video = NULL;
            }
    	}

    	if (p_rua->media_info.has_audio)
    	{
    	    if (p_rua->media_info.live_audio)
    	    {
            	p_rua->media_info.live_audio->freeInstance(p_rua->media_info.a_index);
            	p_rua->media_info.live_audio = NULL;
        	}
    	}
    	
		log_print(HT_LOG_ERR, "%s, rtmp_channel_open failed\r\n", __FUNCTION__);

		rtmp_commit_reconn(p_rua);
		goto thread_exit;
	}
	
	if (p_rua->media_info.has_video)
	{
		if (p_rua->media_info.has_audio)
		{
			p_rua->tid_audio = sys_os_create_thread((void *)rtmp_media_live_audio_thread, (void *)p_rua);
		}
		
		rtmp_media_live_video_send(p_rua);  //rtmp推流

		if (p_rua->media_info.has_audio)
		{
			// wait audio send thread exit ...
			while (p_rua->tid_audio)
			{
				usleep(200*1000);
			}
		}
	}
	else if (p_rua->media_info.has_audio)
	{
		rtmp_media_live_audio_send(p_rua);
	}
	
thread_exit:

	rtmp_channel_close(p_rua);
	
	//printf("end pusher : \r\n\tsrc : %s\r\n\tdst : %s\r\n", p_rua->src_url, p_rua->dst_url);

	p_rua->tid_pusher = 0;
	
	log_print(HT_LOG_DBG, "%s, exit ...\r\n", __FUNCTION__);
	
	return NULL;
}


#endif // MEDIA_LIVE

#ifdef MEDIA_PROXY

BOOL rtmp_parse_proxy_url(RMPUA * p_rua, RTMP_PUSHER * p_pusher)
{
    BOOL ret = FALSE;
    
    if (   memcmp(p_pusher->srcurl, "rtsp://", 7) == 0
        || memcmp(p_pusher->srcurl, "http://", 7) == 0
        || memcmp(p_pusher->srcurl, "https://", 8) == 0
#ifdef RTMP_PROXY
        || memcmp(p_pusher->srcurl, "rtmp://", 7) == 0 
        || memcmp(p_pusher->srcurl, "rtmpt://", 8) == 0
        || memcmp(p_pusher->srcurl, "rtmps://", 8) == 0
        || memcmp(p_pusher->srcurl, "rtmpe://", 8) == 0
        || memcmp(p_pusher->srcurl, "rtmpfp://", 9) == 0
        || memcmp(p_pusher->srcurl, "rtmpte://", 9) == 0
        || memcmp(p_pusher->srcurl, "rtmpts://", 9) == 0
#endif
        )
    {
        p_rua->media_info.is_proxy = 1;
        p_rua->media_info.has_video = 1;
        p_rua->media_info.has_audio = 1;

        ret = TRUE;
    }

	return ret;
}

BOOL rtmp_media_proxy_init(RMPUA * p_rua, RTMP_PUSHER * p_pusher)
{
    BOOL ret = FALSE;

    PROXY_CFG cfg;
    memset(&cfg, 0, sizeof(cfg));

    strcpy(cfg.url, p_pusher->srcurl);

    if (p_pusher->output.has_video || p_pusher->output.has_audio)
    {
        cfg.has_output = 1;
		cfg.output.has_video = p_pusher->output.has_video;
		cfg.output.has_audio = p_pusher->output.has_audio;
    }

    if (p_pusher->output.has_video)
    {
        memcpy(&cfg.output.video, &p_pusher->output.video, sizeof(VIDEO_INFO));
    }

    if (p_pusher->output.has_audio)
    {
        memcpy(&cfg.output.audio, &p_pusher->output.audio, sizeof(AUDIO_INFO));
    }

    p_rua->media_info.proxy = new CMediaProxy(&cfg);
    if (p_rua->media_info.proxy)
    {
        ret = p_rua->media_info.proxy->startConn(cfg.url, cfg.user, cfg.pass);
    }
    else
    {
        log_print(HT_LOG_ERR, "%s, malloc media proxy failed\r\n", __FUNCTION__);
    }

    return ret;
}

void rtmp_media_proxy_callback(uint8 * data, int size, int type, void * pUserdata)
{
	if (type == DATA_TYPE_VIDEO)
	{
		rtmp_media_put_video((RMPUA *) pUserdata, data, size, 0);
	}
	else if (type == DATA_TYPE_AUDIO)
	{
		rtmp_media_put_audio((RMPUA *) pUserdata, data, size, 0, 0);
	}
}

void rtmp_media_proxy_send(RMPUA * p_rua)
{
    CMediaProxy * pProxy = p_rua->media_info.proxy;

	rtmp_media_create_queue(p_rua);

	pProxy->addCallback(rtmp_media_proxy_callback, p_rua);

    while (p_rua->rtp_tx)
	{		
		usleep(200*1000);
	}

    pProxy->delCallback(rtmp_media_proxy_callback, p_rua);

	rtmp_media_free_queue(p_rua);
}

void * rtmp_push_proxy_thread(void * argv)
{
    RMPUA * p_rua = (RMPUA *) argv;

	log_print(HT_LOG_DBG, "%s, enter ...\r\n", __FUNCTION__);

	while (!p_rua->media_info.proxy->inited
#if defined(MEDIA_FILE) || defined(MEDIA_DEVICE)	
	    || (p_rua->media_info.proxy->m_info.has_video && 0 == p_rua->media_info.proxy->m_nVideoRecodec)
	    || (p_rua->media_info.proxy->m_info.has_audio && 0 == p_rua->media_info.proxy->m_nAudioRecodec)
#endif	    
	    )
	{
	    usleep(20*1000);

	    if (!p_rua->rtp_tx)
	    {
	        goto thread_exit;
	    }
	}

#if defined(RTMP_PROXY)
    if (p_rua->media_info.proxy->m_rtmp)
    {
        // Rtmp audio and video READY separately notice, here it need to wait a moment
        sleep(10);
    }

    memcpy(&p_rua->media_info.v_info, &p_rua->media_info.proxy->m_pConfig->output.video, sizeof(VIDEO_INFO));
    memcpy(&p_rua->media_info.a_info, &p_rua->media_info.proxy->m_pConfig->output.audio, sizeof(AUDIO_INFO));
#endif

    p_rua->media_info.has_video = p_rua->media_info.proxy->m_info.has_video;
    p_rua->media_info.has_audio = p_rua->media_info.proxy->m_info.has_audio;

    if (p_rua->media_info.has_video)
    {
#if defined(MEDIA_FILE) || defined(MEDIA_DEVICE)
        p_rua->media_info.v_info.width = p_rua->media_info.proxy->m_pConfig->output.video.width;
        p_rua->media_info.v_info.height = p_rua->media_info.proxy->m_pConfig->output.video.height;
        p_rua->media_info.v_info.framerate = p_rua->media_info.proxy->m_pConfig->output.video.framerate;
#else
        p_rua->media_info.v_info.width = p_rua->media_info.proxy->m_info.video.width;
        p_rua->media_info.v_info.height = p_rua->media_info.proxy->m_info.video.height;
        p_rua->media_info.v_info.framerate = 25; // Unknown frame rate, set to default 25
#endif
    }

    if (p_rua->media_info.has_audio)
    {
#if defined(MEDIA_FILE) || defined(MEDIA_DEVICE)
        p_rua->media_info.a_info.samplerate = p_rua->media_info.proxy->m_pConfig->output.audio.samplerate;
        p_rua->media_info.a_info.channels = p_rua->media_info.proxy->m_pConfig->output.audio.channels;
#else
        p_rua->media_info.a_info.samplerate = p_rua->media_info.proxy->m_info.audio.samplerate;
        p_rua->media_info.a_info.channels = p_rua->media_info.proxy->m_info.audio.channels;
#endif
    }
    
	if (!rtmp_channel_open(p_rua))
	{
		log_print(HT_LOG_ERR, "%s, rtmp_channel_open failed\r\n", __FUNCTION__);

		rtmp_commit_reconn(p_rua);
		goto thread_exit;
	}

	rtmp_media_proxy_send(p_rua);

thread_exit:

    if (p_rua->media_info.proxy)
    {
        delete p_rua->media_info.proxy;
    	p_rua->media_info.proxy = NULL;
	}

	rtmp_channel_close(p_rua);

	printf("end pusher : \r\n\tsrc : %s\r\n\tdst : %s\r\n", p_rua->src_url, p_rua->dst_url);

	p_rua->tid_pusher = 0;
	
	log_print(HT_LOG_DBG, "%s, exit ...\r\n", __FUNCTION__);
	
	return NULL;
}

#endif // end of MEDIA_PROXY

BOOL rtmp_parse_url(RMPUA * p_rua, RTMP_PUSHER * p_pusher)
{
    BOOL ret = FALSE;
    
#ifdef MEDIA_DEVICE
	if (rtmp_parse_device_url(p_rua, p_pusher->srcurl))
	{
		ret = TRUE;
	}
	else 
#endif

#ifdef MEDIA_LIVE
	if (rtmp_parse_live_url(p_rua, p_pusher->srcurl))
	{
		ret = TRUE;
	}
	else
#endif

#ifdef MEDIA_PROXY
	if (rtmp_parse_proxy_url(p_rua, p_pusher))
    {
        ret = TRUE;
    }
    else
#endif

#ifdef MEDIA_FILE
	if (rtmp_parse_file_url(p_rua, p_pusher->srcurl))
	{
		ret = TRUE;
	}
	else 
#endif

	{
	}

	return ret;
}

BOOL rtmp_media_init(RMPUA * p_rua, RTMP_PUSHER * p_pusher)
{
#ifdef MEDIA_FILE
	if (p_rua->media_info.is_file)
	{
		return rtmp_media_file_init(p_rua);
	}
	else 
#endif

#ifdef MEDIA_PROXY
    if (p_rua->media_info.is_proxy)
    {
        return rtmp_media_proxy_init(p_rua, p_pusher);
    }
    else
#endif

#ifdef MEDIA_DEVICE
    if (p_rua->media_info.is_device)
    {
        return rtmp_media_device_init(p_rua);
    }
    else 
#endif

#ifdef MEDIA_LIVE
    if (p_rua->media_info.is_live)
    {
        return rtmp_media_live_init(p_rua);
    }
    else
#endif

    {
    }
    
    return FALSE;
}

BOOL rtmp_rua_start(RMPUA * p_rua)
{
	p_rua->rtp_tx = 1;

	//printf("start pusher : \r\n\tsrc : %s\r\n\tdst : %s\r\n", p_rua->src_url, p_rua->dst_url);

#ifdef MEDIA_FILE
	if (p_rua->media_info.is_file)
	{
		p_rua->tid_pusher = sys_os_create_thread((void *)rtmp_push_file_thread, p_rua);
	}
#endif

#ifdef MEDIA_DEVICE		
	if (p_rua->media_info.is_device)
	{
		p_rua->tid_pusher = sys_os_create_thread((void *)rtmp_push_device_thread, p_rua);
	}
#endif

//#ifdef MEDIA_LIVE
	if (p_rua->media_info.is_live)
	{
		p_rua->tid_pusher = sys_os_create_thread((void *)rtmp_push_live_thread, p_rua);
	}
//#endif

#ifdef MEDIA_PROXY
    if (p_rua->media_info.is_proxy)
    {
		p_rua->tid_pusher = sys_os_create_thread((void *)rtmp_push_proxy_thread, p_rua);
    }
#endif

	return TRUE;
}

void rtmp_rua_stop(RMPUA * p_rua)
{
	p_rua->rtp_tx = 0;

	while (p_rua->tid_pusher)
	{
		usleep(10*1000);		
	}
}



