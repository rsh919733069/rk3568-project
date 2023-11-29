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
#include "rtmp_cfg.h"
#include "xml_node.h"
#include "media_format.h"
#include "rtmp_rmua.h"
#include "log/log.h"
/***********************************************************/

// the default configuration file name
#define RTMP_DEF_CFG	"config_rtmp.xml"

// global rtmp configuration variable
RTMP_CFG g_rtmp_cfg;

/***********************************************************/

RTMP_PUSHER * rtmp_add_pusher(RTMP_PUSHER ** p_output)
{
	RTMP_PUSHER * p_tmp;
	RTMP_PUSHER * p_new_pusher = (RTMP_PUSHER *) malloc(sizeof(RTMP_PUSHER));
	if (NULL == p_new_pusher)
	{
		return NULL;
	}

	memset(p_new_pusher, 0, sizeof(RTMP_PUSHER));

	p_tmp = *p_output;
	if (NULL == p_tmp)
	{
		*p_output = p_new_pusher;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_pusher;
	}	

	return p_new_pusher;
}

void rtmp_free_pushers(RTMP_PUSHER ** p_output)
{
	RTMP_PUSHER * p_next;
	RTMP_PUSHER * p_tmp = *p_output;

	while (p_tmp)
	{
		p_next = p_tmp->next;		
		free(p_tmp);
		p_tmp = p_next;
	}

	*p_output = NULL;
}

/***********************************************************/

int rtmp_parse_video_codec(const char * codec)
{
    if (strcasecmp(codec, "h264") == 0)
    {
        return VIDEO_CODEC_H264;
    }
    else if (strcasecmp(codec, "h265") == 0)
    {
        return VIDEO_CODEC_H265;
    }

    return VIDEO_CODEC_H264;
}

int rtmp_parse_audio_codec(const char * codec)
{
    if (strcasecmp(codec, "G711") == 0 || strcasecmp(codec, "G711A") == 0 || strcasecmp(codec, "PCMA") == 0)
	{
		return AUDIO_CODEC_G711A;
	}
	else if (strcasecmp(codec, "G711U") == 0 || strcasecmp(codec, "PCMU") == 0)
	{
		return AUDIO_CODEC_G711U;
	}
    else if (strcasecmp(codec, "aac") == 0)
    {
        return AUDIO_CODEC_AAC;
    }

    return AUDIO_CODEC_AAC;
}

BOOL rtmp_parse_video_info(XMLN * p_node, VIDEO_INFO * p_info)
{
    XMLN * p_codec;
	XMLN * p_width;
	XMLN * p_height;
	XMLN * p_framerate;
	XMLN * p_bitrate;

    p_codec = xml_node_get(p_node, "codec");
	if (p_codec && p_codec->data)
	{
	    p_info->codec = rtmp_parse_video_codec(p_codec->data);
	}
	
	p_width = xml_node_get(p_node, "width");
	if (p_width && p_width->data)
	{
		p_info->width = atoi(p_width->data);

		if (p_info->width < 0)
		{
			p_info->width = 0;
		}
	}

	p_height = xml_node_get(p_node, "height");
	if (p_height && p_height->data)
	{
		p_info->height = atoi(p_height->data);

		if (p_info->height < 0)
		{
			p_info->height = 0;
		}
	}

	p_framerate = xml_node_get(p_node, "framerate");
	if (p_framerate && p_framerate->data)
	{
		p_info->framerate = atoi(p_framerate->data);

		if (p_info->framerate < 0)
		{
			p_info->framerate = 0;
		}
	}

	p_bitrate = xml_node_get(p_node, "bitrate");
	if (p_bitrate && p_bitrate->data)
	{
		p_info->bitrate = atoi(p_bitrate->data);

		if (p_info->bitrate < 0)
		{
			p_info->bitrate = 0;
		}
	}

	return TRUE;
}

BOOL rtmp_parse_audio_info(XMLN * p_node, AUDIO_INFO * p_info)
{
    XMLN * p_codec;
	XMLN * p_samplerate;
	XMLN * p_channels;
	XMLN * p_bitrate;

    p_codec = xml_node_get(p_node, "codec");
	if (p_codec && p_codec->data)
	{
	    p_info->codec = rtmp_parse_audio_codec(p_codec->data);
	}
	
	p_samplerate = xml_node_get(p_node, "samplerate");
	if (p_samplerate && p_samplerate->data)
	{
		p_info->samplerate = atoi(p_samplerate->data);

		if (p_info->samplerate < 0)
		{
			p_info->samplerate = 0;
		}
	}

	p_channels = xml_node_get(p_node, "channels");
	if (p_channels && p_channels->data)
	{
		p_info->channels = atoi(p_channels->data);

		if (p_info->channels < 0)
		{
			p_info->channels = 0;
		}
		else if (p_info->channels > 2)
		{
			p_info->channels = 2;
		}
	}

	p_bitrate = xml_node_get(p_node, "bitrate");
	if (p_bitrate && p_bitrate->data)
	{
		p_info->bitrate = atoi(p_bitrate->data);

		if (p_info->bitrate < 0)
		{
			p_info->bitrate = 0;
		}
	}

	return TRUE;
}

BOOL rtmp_parse_pusher(XMLN * p_node, RTMP_PUSHER * p_pusher)
{
	XMLN * p_srcurl;
	XMLN * p_dsturl;
	XMLN * p_user;
	XMLN * p_pass;
	XMLN * p_video;
	XMLN * p_audio;

	p_srcurl = xml_node_get(p_node, "srcurl");
	if (p_srcurl && p_srcurl->data)
	{
		strncpy(p_pusher->srcurl, p_srcurl->data, sizeof(p_pusher->srcurl)-1);
	}
	else
	{
		return FALSE;
	}

	p_dsturl = xml_node_get(p_node, "dsturl");
	if (p_dsturl && p_dsturl->data)
	{
		strncpy(p_pusher->dsturl, p_dsturl->data, sizeof(p_pusher->dsturl)-1);
	}
	else
	{
		return FALSE;
	}

	p_user = xml_node_get(p_node, "user");
	if (p_user && p_user->data)
	{
		strncpy(p_pusher->user, p_user->data, sizeof(p_pusher->user)-1);
	}

	p_pass = xml_node_get(p_node, "pass");
	if (p_pass && p_pass->data)
	{
		strncpy(p_pusher->pass, p_pass->data, sizeof(p_pusher->pass)-1);
	}

	p_video = xml_node_get(p_node, "video");
	if (p_video)
	{
		p_pusher->output.has_video = rtmp_parse_video_info(p_video, &p_pusher->output.video);
	}

	p_audio = xml_node_get(p_node, "audio");
	if (p_audio)
	{
		p_pusher->output.has_audio = rtmp_parse_audio_info(p_audio, &p_pusher->output.audio);
	}
	
	return TRUE;
}

BOOL rtmp_parse_config(char * xml_buff, int rlen)
{
	XMLN * p_node;	
	XMLN * p_log_enable;
	XMLN * p_log_level;
	XMLN * p_loop_nums;
	XMLN * p_reconn_interval;
	XMLN * p_pusher;

	p_node = xxx_hxml_parse(xml_buff, rlen);
	if (NULL == p_node)
	{
		return FALSE;
	}

   	p_log_enable = xml_node_get(p_node, "log_enable");
	if (p_log_enable && p_log_enable->data)
	{
		g_rtmp_cfg.log_enable = atoi(p_log_enable->data);
	}

	p_log_level = xml_node_get(p_node, "log_level");
	if (p_log_level && p_log_level->data)
	{
		g_rtmp_cfg.log_level = atoi(p_log_level->data);
	}

    p_loop_nums = xml_node_get(p_node, "loop_nums");
	if (p_loop_nums && p_loop_nums->data)
	{
		g_rtmp_cfg.loop_nums = (uint32) atoi(p_loop_nums->data);
	}

	p_reconn_interval = xml_node_get(p_node, "reconn_interval");
	if (p_reconn_interval && p_reconn_interval->data)
	{
		g_rtmp_cfg.reconn_interval = atoi(p_reconn_interval->data);
	}

	int cnt = 0;
	
	p_pusher = xml_node_get(p_node, "pusher");
	while (p_pusher && strcasecmp(p_pusher->name, "pusher") == 0)
	{
		RTMP_PUSHER pusher;
		memset(&pusher, 0, sizeof(pusher));
		
		if (rtmp_parse_pusher(p_pusher, &pusher))
		{
			RTMP_PUSHER * p_info = rtmp_add_pusher(&g_rtmp_cfg.pusher);
			//SLOGI("rtmp_add_pusher \n");
			if (p_info)
			{
				cnt++;
				memcpy(p_info, &pusher, sizeof(RTMP_PUSHER));
			}
		}

		if (cnt >= MAX_NUM_RUA)
		{
			break;
		}

		p_pusher = p_pusher->next;
	}

	xml_node_del(p_node);
	
	return TRUE;
}


/**
 * read configuration file
 *
 * @param config the configuration file name
 * @return TRUE on success, FALSE on error
 */
BOOL rtmp_read_config(const char * config)
{
	BOOL ret = FALSE;
	int len, rlen;
	FILE * fp = NULL;
	char * xml_buff = NULL;
	const char * filename = NULL;

	if (NULL == config || config[0] == '\0')
	{
		filename = RTMP_DEF_CFG;
	}
	else
	{
		filename = config;
	}

    // read config file
	
	fp = fopen(filename, "r");
	//SLOGI("rtmp %s\n",filename);
	if (NULL == fp)
	{
		goto FAILED;
	}
	
	fseek(fp, 0, SEEK_END);
	
	len = ftell(fp);
	if (len <= 0)
	{
		goto FAILED;
	}
	fseek(fp, 0, SEEK_SET);
	
	xml_buff = (char *) malloc(len + 1);	
	if (NULL == xml_buff)
	{
		goto FAILED;
	}

	rlen = fread(xml_buff, 1, len, fp);
	if (rlen > 0)
	{
	    xml_buff[rlen] = '\0';
	    ret = rtmp_parse_config(xml_buff, rlen);
		//SLOGI("rtmp_parse_config\n");
	}
	else
	{
	    log_print(HT_LOG_ERR, "%s, rlen = %d, len=%d\r\n", __FUNCTION__, rlen, len);
	}

FAILED:

	if (fp)
	{
		fclose(fp);
	}

	if (xml_buff)
	{
		free(xml_buff);
	}

	return ret;
}


