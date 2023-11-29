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

#include <rtmp_log.h>
#include "sys_inc.h"
#include "rtmp_pusher.h"
#include "media_format.h"
#include "media_info.h"
#include "hqueue.h"
#include "rtmp_media.h"
#include "rtsp_client.h"

#ifdef MEDIA_PROXY
#include "rtsp_parse.h"
#include "sys_buf.h"
#endif

#include "common.h"
#include "log/log.h"

/***************************************************************************************/
extern "C" void RTMP_TLS_Init();

RTMP_CLASS	hrtmp;

/***************************************************************************************/

void rtmp_log_callback(int level, const char *format, va_list vl)
{
	int lvl = HT_LOG_DBG;
	int offset = 0;
	char str[2048] = "";

	offset += vsnprintf(str, 2048-1, format, vl);

	offset += snprintf(str+offset, 2048-1-offset, "\r\n");
	
	if (level == RTMP_LOGCRIT)
	{
		lvl = HT_LOG_FATAL;
	}
	else if (level == RTMP_LOGERROR)
	{
		lvl = HT_LOG_ERR;
	}
	else if (level == RTMP_LOGWARNING)
	{
		lvl = HT_LOG_WARN;
	}
	else if (level == RTMP_LOGINFO)
	{
		lvl = HT_LOG_INFO;
	}
	else if (level == RTMP_LOGDEBUG)
	{
		lvl = HT_LOG_DBG;
	}

	log_print(lvl, str);
}

void rtmp_set_rtmp_log()
{
    RTMP_LogLevel lvl;
    
    RTMP_LogSetCallback(rtmp_log_callback);

    switch (g_rtmp_cfg.log_level)
    {
    case HT_LOG_TRC:
        lvl = RTMP_LOGALL;
        break;
        
    case HT_LOG_DBG:
        lvl = RTMP_LOGDEBUG;
        break;

    case HT_LOG_INFO:
        lvl = RTMP_LOGINFO;
        break;

    case HT_LOG_WARN:
        lvl = RTMP_LOGWARNING;
        break;

    case HT_LOG_ERR:
        lvl = RTMP_LOGERROR;
        break;

    case HT_LOG_FATAL:
        lvl = RTMP_LOGCRIT;
        break;

    default:
        lvl = RTMP_LOGERROR;
        break;
    }

    RTMP_LogSetLevel(lvl);
}

void rtmp_pusher_reconn(RMPUA * p_rua)
{
	log_print(HT_LOG_INFO, "%s, enter\r\n", __FUNCTION__);

	RTMP_PUSHER * p_pusher = (RTMP_PUSHER *)p_rua->pusher;
	
	rtmp_rua_stop(p_rua);

	rmpua_set_idle_rua(p_rua);
	
	sleep(g_rtmp_cfg.reconn_interval); // sleep reconn_interval second to try
	
	rtmp_pusher_init(p_pusher);

	log_print(HT_LOG_INFO, "%s, exit\r\n", __FUNCTION__);
}

void rtmp_commit_reconn(RMPUA * p_rua)
{
	RIMSG_RTMP msg;
	memset(&msg,0,sizeof(RIMSG_RTMP));
	msg.msg_src = RTMP_RECONN;
	msg.msg_dua = rmpua_get_index(p_rua);
	
	if (hqBufPut(hrtmp.msg_queue, (char *)&msg) == FALSE)
	{
		log_print(HT_LOG_ERR, "%s, hqBufPut failed\r\n");
	}
}

void * rtmp_puher_task(void * argv)
{
	RIMSG_RTMP stm;

	log_print(HT_LOG_INFO, "%s, enter\r\n", __FUNCTION__);
	
	while (1)
	{
		if (hqBufGet(hrtmp.msg_queue,(char *)&stm))
		{
			switch(stm.msg_src)
			{
			case RTMP_RECONN:
				rtmp_pusher_reconn(rmpua_get_by_index(stm.msg_dua));
				break;
				
			case RTMP_EXIT:
			    goto EXIT;
			}
		}
	}

EXIT:

	hrtmp.tid_main = 0;

	log_print(HT_LOG_INFO, "%s, exit\r\n", __FUNCTION__);
	
	return NULL;
}

BOOL rtmp_pusher_init(RTMP_PUSHER * p_pusher)
{
	RMPUA * p_rua = rmpua_get_idle_rua();
	if (NULL == p_rua)
	{
		SLOGI("%s, rmpua_get_idle_rua return null\r\n", __FUNCTION__);
		return FALSE;
	}

	if (!rtmp_parse_url(p_rua, p_pusher))
	{
		rmpua_set_idle_rua(p_rua);

		SLOGI("%s, invalid src %s\r\n", __FUNCTION__, p_pusher->srcurl);
		return FALSE;
	}

	if(strcmp(p_pusher->srcurl,"live") == 0)
	{
		strcpy(p_rua->src_url, p_pusher->srcurl);
		strcpy(p_rua->dst_url, gs_rtsp_info.rtmp_ip);
	}
	else
	{
		strcpy(p_rua->src_url, p_pusher->srcurl);
		strcpy(p_rua->dst_url, p_pusher->dsturl);
	}

	strcpy(p_rua->user, p_pusher->user);
	strcpy(p_rua->pass, p_pusher->pass);
    p_rua->pusher = p_pusher;
    
	if (p_pusher->output.has_video && p_rua->media_info.has_video)
	{
		memcpy(&p_rua->media_info.v_info, &p_pusher->output.video, sizeof(VIDEO_INFO));
	}
	else
	{
		p_rua->media_info.has_video = 0;
	}

	if (p_pusher->output.has_audio && p_rua->media_info.has_audio)
	{
		memcpy(&p_rua->media_info.a_info, &p_pusher->output.audio, sizeof(AUDIO_INFO));
	}
	else
	{
		p_rua->media_info.has_audio = 0;
	}

    if (!rtmp_media_init(p_rua, p_pusher))
    {
        rmpua_set_idle_rua(p_rua);

		SLOGI("%s, %s, rtmp_media_init failed\r\n", __FUNCTION__, p_pusher->srcurl);
		return FALSE;
    }

	printf("gs_rtsp_info.rtmp_ip:%s\n",gs_rtsp_info.rtmp_ip);
	//SLOGI("p_rua->media_info.has_video %d",p_rua->media_info.has_video);
	if (p_rua->media_info.has_video || p_rua->media_info.has_audio)
	{
		printf("begin rtmp gongneng\n");
		rtmp_rua_start(p_rua);
		rmpua_set_online_rua(p_rua);
		return TRUE;
	}
	
	return FALSE;
}

void rtmp_pusher_uninit(RTMP_PUSHER * p_pusher)
{
    log_print(HT_LOG_DBG, "%s, enter...\r\n", __FUNCTION__);

    for (int i = 0; i < MAX_NUM_RUA; i++)
	{
		RMPUA * p_rua = rmpua_get_by_index(i);

		if (p_rua->used_flag && p_rua->pusher == p_pusher)
		{
			rtmp_rua_stop(p_rua);
			rmpua_set_idle_rua(p_rua);
			break;
		}
	}
}

BOOL rtmp_pusher_start1()
{
    if (g_rtmp_cfg.reconn_interval <= 0)
	{
		g_rtmp_cfg.reconn_interval = 5;
	}

	memset(&hrtmp, 0, sizeof(hrtmp));

	hrtmp.msg_queue = hqCreate(2*MAX_NUM_RUA, sizeof(RIMSG_RTMP), HQ_GET_WAIT);
	if (hrtmp.msg_queue == NULL)
	{
		log_print(HT_LOG_ERR, "%s, create task queue failed!!!\r\n", __FUNCTION__);
		return FALSE;
	}

	hrtmp.tid_main = sys_os_create_thread((void *)rtmp_puher_task, NULL);
	
	rmpua_proxy_init();

	RTMP_TLS_Init();

    rtmp_set_rtmp_log();

#ifdef MEDIA_PROXY
	sys_buf_init(4*MAX_NUM_RUA);
	rtsp_parse_buf_init(4*MAX_NUM_RUA);
	http_msg_buf_init(4*MAX_NUM_RUA);
#endif

	srand(time(NULL));
	//logger_info("rtmp","rtmp_pusher_init");
	RTMP_PUSHER * p_pusher = g_rtmp_cfg.pusher;
	while (p_pusher)
	{
		printf("song jin lai le\n");
		//logger_info("rtmp","p_pusher ");
		rtmp_pusher_init(p_pusher);
		
		p_pusher = p_pusher->next;
	}

	return TRUE;
}

BOOL rtmp_pusher_start(const char * cfg_file)
{
	printf("Happytime RTMP Pusher %s -----------------\r\n", VERSION_STRING);

	rtmp_read_config(cfg_file);

	if (g_rtmp_cfg.log_enable)
	{
		log_init(RTMP_PUSHER_LOG);
		log_set_level(g_rtmp_cfg.log_level);
	}

	return rtmp_pusher_start1();
}

void rtmp_pusher_stop()
{
    RIMSG_RTMP stm;
    memset(&stm, 0, sizeof(stm));

    stm.msg_src = RTMP_EXIT;

    hqBufPut(hrtmp.msg_queue, (char *)&stm);

    while (hrtmp.tid_main)
    {
        usleep(10*1000);
    }
    
	RMPUA * p_rua = rmpua_lookup_start();
	while (p_rua)
	{
		rtmp_rua_stop(p_rua);

		p_rua = rmpua_lookup_next(p_rua);
	}
	rmpua_lookup_stop();

	rtmp_free_pushers(&g_rtmp_cfg.pusher);

	rmpua_proxy_deinit();

    hqDelete(hrtmp.msg_queue);
    
#ifdef MEDIA_PROXY
	sys_buf_deinit();
	rtsp_parse_buf_deinit();
	http_msg_buf_deinit();
#endif

	log_close();
}

int run_rtmp_pusher(void)
{
	int ret;
	ret=rtmp_pusher_start(NULL);

	return ret;
}




