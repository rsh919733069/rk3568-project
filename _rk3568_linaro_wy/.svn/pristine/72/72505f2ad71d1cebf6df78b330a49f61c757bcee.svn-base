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
#include "rtmp_tx.h"
#include "media_util.h"
#include "media_format.h"
#include "h264.h"
#include "h265.h"
#include "rtmp.h"
#include "rtmp_pusher.h"

/***************************************************************************************/

#define RTMP_HEAD_SIZE   (sizeof(RTMPPacket)+RTMP_MAX_HEADER_SIZE)


/***************************************************************************************/

char * rtmp_put_byte(char *output, uint8 val)    
{    
    output[0] = val;  
    
    return output + 1;    
}

char * rtmp_put_be16(char *output, uint16 val)    
{    
    output[1] = val & 0xff;    
    output[0] = val >> 8;
    
    return output + 2;    
}

char * rtmp_put_be24(char *output, uint32 val)    
{    
    output[2] = val & 0xff;    
    output[1] = val >> 8;    
    output[0] = val >> 16; 
    
    return output + 3;    
}

char * rtmp_put_be32(char *output, uint32 val)    
{    
    output[3] = val & 0xff;    
    output[2] = val >> 8;    
    output[1] = val >> 16;    
    output[0] = val >> 24;
    
    return output + 4;    
}

char * rtmp_put_be64(char *output, ulint64 val)
{    
    output = rtmp_put_be32(output, val >> 32);    
    output = rtmp_put_be32(output, val);
    
    return output;    
}

char * rtmp_put_amf_string(char *c, const char *str)    
{    
    uint16 len = (uint16)strlen(str);
    c = rtmp_put_be16(c, len);    
    memcpy(c, str,len); 
    
    return c + len;    
}

char * rtmp_put_amf_bool(char *c, int b)    
{    
    *c++ = AMF_BOOLEAN; 
    return rtmp_put_byte(c, !!b);  
}

char * rtmp_put_amf_double(char *c, double d)    
{    
    *c++ = AMF_NUMBER;

    uint8 *ci, *co;    
    ci = (uint8 *)&d;    
    co = (uint8 *)c;    
    co[0] = ci[7];    
    co[1] = ci[6];    
    co[2] = ci[5];    
    co[3] = ci[4];    
    co[4] = ci[3];    
    co[5] = ci[2];    
    co[6] = ci[1];    
    co[7] = ci[0];    
    
    return c + 8;    
}

int rtmp_send_metadata(void * info)
{
    int ret;
    uint8 * body;
    RTMPPacket * packet;
    RMPUA * p_rua = (RMPUA *)info;
    RTMP * p_rtmp = p_rua->rtmp;
	
	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + 1024);
	memset(packet, 0, RTMP_HEAD_SIZE);

	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (uint8 *)packet->m_body;
	
	char * p = (char *)body;

    p = rtmp_put_byte(p, AMF_STRING);  
    p = rtmp_put_amf_string(p, "@setDataFrame"); 
    
    p = rtmp_put_byte(p, AMF_STRING);  
    p = rtmp_put_amf_string(p, "onMetaData");

    p = rtmp_put_byte(p, AMF_OBJECT);

    if (p_rua->media_info.has_video)
    {
        p = rtmp_put_amf_string(p, "hasVideo");  
        p = rtmp_put_amf_bool(p, 1);
        
        p = rtmp_put_amf_string(p, "width");  
        p = rtmp_put_amf_double(p, p_rua->media_info.v_info.width);
      
        p = rtmp_put_amf_string(p, "height");  
        p = rtmp_put_amf_double(p, p_rua->media_info.v_info.height);
      
        p = rtmp_put_amf_string(p, "framerate");  
        p = rtmp_put_amf_double(p, p_rua->media_info.v_info.framerate);

        if (VIDEO_CODEC_H264 == p_rua->media_info.v_info.codec)
        {
            p = rtmp_put_amf_string(p, "videocodecid");
            p = rtmp_put_amf_double(p, 7);
        }
        else if (VIDEO_CODEC_H265 == p_rua->media_info.v_info.codec)
        {
            p = rtmp_put_amf_string(p, "videocodecid");
            p = rtmp_put_amf_double(p, 12);
        }
    }

    if (p_rua->media_info.has_audio)
    {
        p = rtmp_put_amf_string(p, "hasAudio");  
        p = rtmp_put_amf_bool(p, 1);
        
        p = rtmp_put_amf_string(p, "stereo");  
        p = rtmp_put_amf_bool(p, p_rua->media_info.a_info.channels > 1 ? 1 : 0);
        
        p = rtmp_put_amf_string(p, "audiosamplerate");  
        p = rtmp_put_amf_double(p, p_rua->media_info.a_info.samplerate);

        p = rtmp_put_amf_string(p, "audiosamplesize");  
        p = rtmp_put_amf_double(p, 16);

        if (AUDIO_CODEC_AAC == p_rua->media_info.a_info.codec)
        {
            p = rtmp_put_amf_string(p, "audiocodecid");  
            p = rtmp_put_amf_double(p, 10);
        }
        else if (AUDIO_CODEC_G711A == p_rua->media_info.a_info.codec)
        {
            p = rtmp_put_amf_string(p, "audiocodecid");  
            p = rtmp_put_amf_double(p, 7);
        }
        else if (AUDIO_CODEC_G711U == p_rua->media_info.a_info.codec)
        {
            p = rtmp_put_amf_string(p, "audiocodecid");  
            p = rtmp_put_amf_double(p, 8);
        }
    }

    *p++ = 0;
    *p++ = 0;
    *p++ = AMF_OBJECT_END;
  
    int len = p - (char *)body;
    
    packet->m_packetType = RTMP_PACKET_TYPE_INFO;
	packet->m_nBodySize = len;
	packet->m_nChannel = 5;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet->m_nInfoField2 = p_rtmp->m_stream_id;
	
    ret = RTMP_SendPacket(p_rtmp, packet, 0);
    
	free(packet);

	return (ret == FALSE) ? -1 : 0;
}

int rtmp_send_avcc(RTMP * rtmp, uint8 * sps, int sps_len, uint8 * pps, int pps_len, uint32 ts)
{
	int i;
	uint8 * body;
	RTMPPacket * packet;

	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + 1024);
	if (NULL == packet)
	{
		return -1;
	}
	
	memset(packet, 0, RTMP_HEAD_SIZE);

	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (uint8 *)packet->m_body;

	i = 0;
	body[i++] = 0x17;
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	/* AVCDecoderConfigurationRecord */
	body[i++] = 0x01;
	body[i++] = sps[1];
	body[i++] = sps[2];
	body[i++] = sps[3];
	body[i++] = 0xff;

	/* sps */
	body[i++] = 0xe1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i], sps, sps_len);
	i += sps_len;

	/* pps */
	body[i++] = 0x01;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i], pps, pps_len);
	i += pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 6;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet->m_nInfoField2 = rtmp->m_stream_id;

	RTMP_SendPacket(rtmp, packet, 0);
	
	free(packet);

	return 0;
}

int rtmp_send_avcc_ex(RTMP * rtmp, uint8 * vps, int vps_len, uint8 * sps, int sps_len, uint8 * pps, int pps_len, uint32 ts)
{
	int i;
	uint8 * body;
	RTMPPacket * packet;

	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + 1024);
	if (NULL == packet)
	{
		return -1;
	}
	
	memset(packet, 0, RTMP_HEAD_SIZE);

	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (uint8 *)packet->m_body;

	i = 0;
	body[i++] = 0x1c;
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

    body[i++] = 3; // numOfArrays
    
    /* vps */
    body[i++] = HEVC_NAL_VPS;
    body[i++] = 0;
    body[i++] = 1;
    body[i++] = (vps_len >> 8) & 0xff;
    body[i++] = vps_len & 0xff;
    memcpy(&body[i], vps, vps_len);
	i += vps_len;

	/* sps */
	body[i++] = HEVC_NAL_SPS;
	body[i++] = 0;
    body[i++] = 1;
	body[i++] = (sps_len >> 8) & 0xff;
	body[i++] = sps_len & 0xff;
	memcpy(&body[i], sps, sps_len);
	i += sps_len;

	/* pps */
	body[i++] = HEVC_NAL_PPS;
	body[i++] = 0;
    body[i++] = 1;
	body[i++] = (pps_len >> 8) & 0xff;
	body[i++] = (pps_len) & 0xff;
	memcpy(&body[i], pps, pps_len);
	i += pps_len;

	packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = i;
	packet->m_nChannel = 6;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet->m_nInfoField2 = rtmp->m_stream_id;

	RTMP_SendPacket(rtmp, packet, 0);
	
	free(packet);

	return 0;
}

int rtmp_push_h26x_avcc(RMPUA * p_rua, uint32 ts)
{
    int ret = -1;
    
	if (p_rua->avcc_flag == 1)
	{
		return 0;
	}

    if (VIDEO_CODEC_H264 == p_rua->media_info.v_info.codec && p_rua->sps_len > 0 && p_rua->pps_len > 0)
    {
        ret = rtmp_send_avcc(p_rua->rtmp, p_rua->sps_buf, p_rua->sps_len, p_rua->pps_buf, p_rua->pps_len, ts);
    }
    else if (VIDEO_CODEC_H265 == p_rua->media_info.v_info.codec && p_rua->vps_len > 0 && p_rua->sps_len > 0 && p_rua->pps_len > 0)
    {
        ret = rtmp_send_avcc_ex(p_rua->rtmp, p_rua->vps_buf, p_rua->vps_len, p_rua->sps_buf, p_rua->sps_len, p_rua->pps_buf, p_rua->pps_len, ts);
    }

    if (ret >= 0)
    {
	    p_rua->avcc_flag = 1;
	}

	return 0;
}

int rtmp_send_h26x_data(RMPUA * p_rua, uint8 * buf, int len, uint32 ts, uint8 key)
{
    int ret;
	uint8 * body;
	RTMPPacket * packet;
	RTMP * rtmp = p_rua->rtmp;
	
	packet = (RTMPPacket *)(buf - RESV_HEADER_SIZE);
	if (NULL == packet)
	{
		return -1;
	}
	
	memset(packet, 0, RTMP_HEAD_SIZE);

	packet->m_body = (char *)(buf - 9);

	/* send video packet */
	body = (uint8 *)packet->m_body;
	
	if (VIDEO_CODEC_H264 == p_rua->media_info.v_info.codec)
	{
	    /* key frame */
    	if (key)
    	{
    		body[0] = 0x17;
    	}
    	else
    	{
    	    body[0] = 0x27;
    	}
	}
	else if (VIDEO_CODEC_H265 == p_rua->media_info.v_info.codec)
	{
    	if (key)
    	{
    		body[0] = 0x1c;
    	}
    	else
    	{
    	    body[0] = 0x2c;
    	}
	}
	
	body[1] = 0x01;   /* nal unit */
	body[2] = 0x00;
	body[3] = 0x00;
	body[4] = 0x00;

	body[5] = (len >> 24) & 0xff;
	body[6] = (len >> 16) & 0xff;
	body[7] = (len >> 8) & 0xff;
	body[8] = (len) & 0xff;

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
	packet->m_nBodySize = len + 9;
	packet->m_nChannel = 6;
	packet->m_nTimeStamp = ts;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet->m_nInfoField2 = rtmp->m_stream_id;
    
	ret = RTMP_SendPacket(rtmp, packet, 0);
    
	return (ret == FALSE) ? -1 : 0;
}

int rtmp_push_h26x(RMPUA * p_rua, uint8 * p_data, int len, uint32 ts, uint8 key)
{
	if (p_rua == NULL || p_rua->avcc_flag != 1)
	{
		return -1;
	}

	return rtmp_send_h26x_data(p_rua, p_data, len, ts, key);
}

int rtmp_h26x_data_tx(void * info, uint8 * p_data, int len, uint32 ts)
{
    int ret = 0;
	RMPUA * p_rua = (RMPUA *) info;
	if (NULL == p_rua->rtmp)
	{
		return -1;
	}

	uint8 key;
	uint8 nalu_t;

    if (VIDEO_CODEC_H264 == p_rua->media_info.v_info.codec)
    {
        nalu_t = p_data[0] & 0x1F;
	    key = (nalu_t == H264_NAL_IDR);

	    if (nalu_t == H264_NAL_SPS)
    	{
    		if (p_rua->sps_len == 0 && len <= sizeof(p_rua->sps_buf))
    		{
    			memcpy(p_rua->sps_buf, p_data, len);
    			p_rua->sps_len = len;

    			rtmp_push_h26x_avcc(p_rua, ts);
    		}
    	}
    	else if (nalu_t == H264_NAL_PPS)
    	{
    		if (p_rua->pps_len == 0 && len <= sizeof(p_rua->pps_buf))
    		{
    			memcpy(p_rua->pps_buf, p_data, len);
    			p_rua->pps_len = len;

    			rtmp_push_h26x_avcc(p_rua, ts);
    		}
    	}
    	else if (nalu_t == H264_NAL_SEI)
    	{
    	}
    	else if (p_rua->avcc_flag && p_rua->iframe_tx)
    	{
    		ret = rtmp_push_h26x(p_rua, p_data, len, ts, key);
    	}
    	else if (p_rua->avcc_flag && key)
    	{
    	    p_rua->iframe_tx = 1;
    	    
    	    ret = rtmp_push_h26x(p_rua, p_data, len, ts, key);
    	}
    }
    else if (VIDEO_CODEC_H265 == p_rua->media_info.v_info.codec)
    {
        nalu_t = (p_data[0] >> 1) & 0x3F;
        key = (nalu_t >= HEVC_NAL_BLA_W_LP && nalu_t <= HEVC_NAL_CRA_NUT);

        if (nalu_t == HEVC_NAL_VPS)
    	{
    	    if (p_rua->vps_len == 0 && len <= sizeof(p_rua->vps_buf))
    		{
    			memcpy(p_rua->vps_buf, p_data, len);
    			p_rua->vps_len = len;

    			rtmp_push_h26x_avcc(p_rua, ts);
    		}
    	}
    	else if (nalu_t == HEVC_NAL_SPS)
    	{
    		if (p_rua->sps_len == 0 && len <= sizeof(p_rua->sps_buf))
    		{
    			memcpy(p_rua->sps_buf, p_data, len);
    			p_rua->sps_len = len;

    			rtmp_push_h26x_avcc(p_rua, ts);
    		}
    	}
    	else if (nalu_t == HEVC_NAL_PPS)
    	{
    		if (p_rua->pps_len == 0 && len <= sizeof(p_rua->pps_buf))
    		{
    			memcpy(p_rua->pps_buf, p_data, len);
    			p_rua->pps_len = len;

    			rtmp_push_h26x_avcc(p_rua, ts);
    		}
    	}
    	else if (nalu_t == HEVC_NAL_SEI_PREFIX || nalu_t == HEVC_NAL_SEI_SUFFIX)
    	{
    	}
    	else if (p_rua->avcc_flag && p_rua->iframe_tx)
    	{
    		ret = rtmp_push_h26x(p_rua, p_data, len, ts, key);
    	}
    	else if (p_rua->avcc_flag && key)
    	{
    	    p_rua->iframe_tx = 1;
    	    
    	    ret = rtmp_push_h26x(p_rua, p_data, len, ts, key);
    	}
    }
	
	return ret;
}

int rtmp_h26x_tx(void * p_rua, uint8 * p_data, int len, uint32 ts)
{
    int ret = 0;
	int s_len;
	int n_len = 0;	
	uint8 * p_next;
	uint8 * p_cur = p_data;
	uint8 * p_end = p_data + len;
	
	while (p_cur && p_cur < p_end && len > 0)
	{
		p_next = avc_split_nalu(p_cur, len, &s_len, &n_len);

		ret = rtmp_h26x_data_tx(p_rua, p_cur+s_len, n_len-s_len, ts);
		if (ret < 0)
		{
		    break;
		}

		len -= n_len;
		p_cur = p_next;
	}
 
	return ret;
}

int rtmp_aac_spec_tx(RMPUA * p_rua, uint8 * spec_buf, int spec_len, uint32 ts)
{
	RTMP * rtmp = p_rua->rtmp;
	if (NULL == rtmp)
	{
		return -1;
	}

	int len;
	uint8 * body;
	RTMPPacket * packet;

	len = spec_len;  /* spec data length,commonly is 2 */

	packet = (RTMPPacket *)malloc(RTMP_HEAD_SIZE + len + 2);
	if (NULL == packet)
	{
		return -1;
	}
	
	memset(packet, 0, RTMP_HEAD_SIZE);

	packet->m_body = (char *)packet + RTMP_HEAD_SIZE;
	body = (uint8 *)packet->m_body;

    uint8 soundrate;
    uint8 soundsize = 1;
    uint8 soundtype = (p_rua->media_info.a_info.channels > 1 ? 1 : 0);

    if (p_rua->media_info.a_info.samplerate <= 5500)
    {
        soundrate = 0;
    }
    else if (p_rua->media_info.a_info.samplerate <= 11000)
    {
        soundrate = 1;
    }
    else if (p_rua->media_info.a_info.samplerate <= 22000)
    {
        soundrate = 2;
    }
    else
    {
        soundrate = 3;
    }

	/*********************************************************************************
	 * Audio tag header
	 *    b7 b6 b5 b4 b3 b2 b1 b0
      *
      *    b4-b7 SoundFormat, 10 = AAC
      *    b2-b3 SoundRate, 0 = 5.5 kHz, 1 = 11 kHz, 2 = 22 kHz, 3 = 44 kHz
      *    b1    SoundSize, 0 = 8-bit samples, 1 = 16-bit samples
      *    b0    SoundType, 0 = Mono sound, 1 = Stereo sound
	 ********************************************************************************/
	body[0] = ((0xA << 4) | (soundrate << 2) | (soundsize << 1) | (soundtype));
	body[1] = 0x00; // AAC sequence header
	memcpy(&body[2], spec_buf, len); /* spec_buf is AAC sequence header data */

    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
	packet->m_nBodySize = len + 2;
	packet->m_nChannel = 7;
	packet->m_nTimeStamp = 0;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet->m_nInfoField2 = rtmp->m_stream_id;

	RTMP_SendPacket(rtmp, packet, 0);
	
	free(packet);

	return 0;
}

int rtmp_send_audio(RMPUA * p_rua, uint8 * buf, int len, uint32 ts)
{
    RTMP * rtmp = p_rua->rtmp;
	if (NULL == rtmp)
	{
		return -1;
	}
	
	if (NULL == buf || len <= 0) 
	{
		return -1;
	}

    int ret;
    int taglen;
    uint8 fmt;
	uint8 * body;
	RTMPPacket * packet;

	packet = (RTMPPacket *)(buf - RESV_HEADER_SIZE);
	if (NULL == packet)
	{
	    return -1;
	}
	
	memset(packet, 0, RTMP_HEAD_SIZE);

    if (AUDIO_CODEC_AAC == p_rua->media_info.a_info.codec)
    {
        fmt = 0xA;
        taglen = 2;
	}
	else if (AUDIO_CODEC_G711A == p_rua->media_info.a_info.codec)
	{
	    fmt = 0x7;
	    taglen = 1;
	}
	else if (AUDIO_CODEC_G711U == p_rua->media_info.a_info.codec)
	{
	    fmt = 0x8;
	    taglen = 1;
	}
	else
	{
	    log_print(HT_LOG_ERR, "%s, unsupport codec %d\r\n", __FUNCTION__, p_rua->media_info.a_info.codec);
	    return -1;
	}
	
	packet->m_body = (char *)(buf - taglen);
	body = (uint8 *)packet->m_body;

    uint8 soundrate;
    uint8 soundsize = 1;
    uint8 soundtype = (p_rua->media_info.a_info.channels > 1 ? 1 : 0);

    if (p_rua->media_info.a_info.samplerate <= 5500)
    {
        soundrate = 0;
    }
    else if (p_rua->media_info.a_info.samplerate <= 11000)
    {
        soundrate = 1;
    }
    else if (p_rua->media_info.a_info.samplerate <= 22000)
    {
        soundrate = 2;
    }
    else
    {
        soundrate = 3;
    }
    
	/*********************************************************************************
	 * Audio tag header
	 *    b7 b6 b5 b4 b3 b2 b1 b0
      *
      *    b4-b7 SoundFormat, 10 = AAC, 7 = G711A, 8 = G711U
      *    b2-b3 SoundRate, 0 = 5.5 kHz, 1 = 11 kHz, 2 = 22 kHz, 3 = 44 kHz
      *    b1    SoundSize, 0 = 8-bit samples, 1 = 16-bit samples
      *    b0    SoundType, 0 = Mono sound, 1 = Stereo sound
	 ********************************************************************************/
	body[0] = ((fmt << 4) | (soundrate << 2) | (soundsize << 1) | (soundtype));

	if (AUDIO_CODEC_AAC == p_rua->media_info.a_info.codec)
	{
	    body[1] = 0x01; // AAC raw data
	}

    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
	packet->m_nBodySize = len + taglen;
	packet->m_nChannel = 7;
	packet->m_nTimeStamp = ts;
	packet->m_hasAbsTimestamp = 0;
	packet->m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet->m_nInfoField2 = rtmp->m_stream_id;

	ret = RTMP_SendPacket(rtmp, packet, 0);

	return (ret == FALSE) ? -1 : 0;
}

int rtmp_aac_tx(void * info, uint8 * p_data, int len, uint32 ts)
{
    int ret = 0;
	RMPUA * p_rua = (RMPUA *)info;

	// the first packet should be aac spec
	if (p_rua->aac_flag == 0)
	{
	    int rate_idx = 0;
    
        switch (p_rua->media_info.a_info.samplerate)
        {
        case 96000:
            rate_idx = 0;
            break;

        case 88200:
            rate_idx = 1;
            break;
            
        case 64000:
            rate_idx = 2;
            break;
            
        case 48000:
            rate_idx = 3;
            break;
            
        case 44100:
            rate_idx = 4;
            break;
            
        case 32000:
            rate_idx = 5;
            break;
            
        case 24000:
            rate_idx = 6;
            break;

        case 22050:
            rate_idx = 7;
            break;

        case 16000:
            rate_idx = 8;
            break;     

        case 12000:
            rate_idx = 9;
            break;

        case 11025:
            rate_idx = 10;
            break;

        case 8000:
            rate_idx = 11;
            break;

        case 7350:
            rate_idx = 12;
            break;
            
        default:
            rate_idx = 4;
            break;
        }

        uint8 config[2];
        uint8 audioObjectType = 2; // AAC-LC
        int   configSize = 2;
        
        config[0] = (audioObjectType << 3) | (rate_idx >> 1);
        config[1] = (rate_idx << 7) | (p_rua->media_info.a_info.channels << 3);
        
		p_rua->aac_flag = 1;
		
		return rtmp_aac_spec_tx(p_rua, config, configSize, ts);
	}

    ret = rtmp_send_audio(p_rua, p_data, len, ts);

	return ret;
}

int rtmp_g711a_tx(void * info, uint8 * p_data, int len, uint32 ts)
{
    return rtmp_send_audio((RMPUA *)info, p_data, len, ts);
}

int rtmp_g711u_tx(void * info, uint8 * p_data, int len, uint32 ts)
{
    return rtmp_send_audio((RMPUA *)info, p_data, len, ts);
}



