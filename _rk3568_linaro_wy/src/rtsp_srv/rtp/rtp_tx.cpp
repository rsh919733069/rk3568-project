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
#include "rtp.h"
#include "rtp_tx.h"
#include "mjpeg_tables.h"
#include "mjpeg.h"
#include "h_util.h"
#include "media_util.h"
#include "rtsp_util.h"
#ifdef RTSP_RTCP
#include "rtcp.h"
#endif
#ifdef RTSP_OVER_HTTP
#include "http.h"
#include "http_srv.h"
#endif
#ifdef RTSP_WEBSOCKET
#include "rtsp_ws.h"
#include "http_srv.h"
#endif


/***********************************************************************/

uint16 rtp_read_uint16(uint8 *input)
{
    return (uint16)((input[0] << 8) | input[1]);
}

uint32 rtp_read_uint32(uint8 *input)
{
    return (uint32)((input[0] << 24) | (input[1] << 16) | (input[2] << 8) | input[3]);
}

uint64 rtp_read_uint64(uint8 *input)
{
    uint32 low = rtp_read_uint32(input);
    uint32 high = rtp_read_uint32(input+4);
    return (uint64)(((uint64)low) << 32 | high);
}

int rtp_write_uint16(uint8 *output, uint16 nVal)
{
    output[1] = nVal & 0xff;
    output[0] = nVal >> 8;

    return 2;
}

int rtp_write_uint32(uint8 *output, uint32 nVal)
{
    output[3] = nVal & 0xff;
    output[2] = nVal >> 8;
    output[1] = nVal >> 16;
    output[0] = nVal >> 24;

    return 4;
}

/***********************************************************************/

/**
 * Send rtp data by TCP socket
 *
 * @param p_rua rtsp user agent
 * @param p_data rtp data
 * @param len rtp data len
 * @return -1 on error, or the data length has been sent
 */
int rtp_tcp_tx(RSUA * p_rua, uint8 * p_data, int len)
{
	int offset = 0;
    SOCKET fd = p_rua->fd;

#ifdef RTSP_OVER_HTTP
    if (p_rua->rtsp_send)
    {
        return http_srv_cln_tx((HTTPCLN *) p_rua->rtsp_send, (char *)p_data, len);
    }
#endif

#ifdef RTSP_WEBSOCKET
    if (p_rua->http_cln)
    {
        int extra = rtsp_ws_encode_data(p_data, len, 0x82);
        
        len += extra;
        p_data -= extra;

        offset = http_srv_cln_tx((HTTPCLN *) p_rua->http_cln, (char *)p_data, len);
        return offset - extra;
    }
#endif

	if (fd <= 0)
	{
	    return -1;
	}
	
	while (p_rua->rtp_tx && offset < len)
	{
		int tlen = send(fd, (const char *)p_data+offset, len-offset, 0);
		if (tlen > 0)
		{
			offset += tlen;
		}
		else
		{
		    int sockerr = sys_os_get_socket_error_num();
            if (sockerr == EINTR || sockerr == EAGAIN)
            {
                usleep(1000);
                continue;
            }
			
			log_print(HT_LOG_ERR, "%s, send failed, fd[%d],tlen[%d,%d],err[%d][%s]!!!\r\n",
				__FUNCTION__, fd, tlen, (len-offset), errno, strerror(errno));			
			return -1;
		}
	}
	
	return offset;
}

/**
 * Send rtp data by UDP socket
 *
 * @param p_rua rtsp user agent
 * @param av_t whether video rtp data
 * @param p_data rtp data
 * @param len rtp data len
 * @return -1 on error, or the data length has been sent
 */
int rtp_udp_tx(RSUA * p_rua, int av_t, uint8 * p_data, int len)
{
	int offset = 0;
	SOCKET fd;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = p_rua->user_real_ip;

    addr.sin_port = htons(p_rua->channels[av_t].r_port);
    fd = p_rua->channels[av_t].udp_fd;

    if (0 == p_rua->rtp_unicast)
    {
        addr.sin_addr.s_addr = inet_addr(p_rua->channels[av_t].destination);
    }

    if (fd <= 0)
	{
	    return -1;
	}

    while (p_rua->rtp_tx && offset < len)
    {
    	int tlen = sendto(fd, (char *)p_data+offset, len-offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
        if (tlen > 0)
        {
            offset += tlen;
        }
    	else
    	{
    	    int sockerr = sys_os_get_socket_error_num();
            if (sockerr == EINTR || sockerr == EAGAIN)
            {
                usleep(1000);
                continue;
            }
            
    		log_print(HT_LOG_ERR, "%s, tlen = %d, len = %d, ip=0x%08x\r\n", 
    			__FUNCTION__, tlen, len, ntohl(p_rua->user_real_ip));
    		return -1;	
    	}
	}

	return offset;
}


/**
 * Build video rtp packet and send
 *
 * @param p_rua rtsp user agent
 * @param p_data payload data
 * @param len payload data length
 * @ts the packet timestamp
 * @mbit rtp marker flag
 * @return the rtp packet length, -1 on error
 */
int rtp_build(RSUA * p_rua, int av_t, uint8 * p_data, int len, uint32 ts, int mbit)
{
	int offset = 0;
	int slen = 0;
	uint8 buff[40];
	uint8 * p_rtp_ptr;

#ifdef RTSP_RTCP
	rtcp_send_packet(p_rua, av_t);
#endif

    p_rua->channels[av_t].rtp_info.rtp_ts = ts;
    
	if (p_rua->rtp_tcp)
	{
		*(buff+offset) = 0x24; // magic
		offset++;
		
		*(buff+offset) = p_rua->channels[av_t].interleaved; // channel
		offset++;
		
#ifdef RTSP_REPLAY
        if (p_rua->replay)
        {
            offset += rtp_write_uint16(buff+offset, 12 + len + 16); // rtp payload length
        }
        else
        {
            offset += rtp_write_uint16(buff+offset, 12 + len);  // rtp payload length
        }
#else
        offset += rtp_write_uint16(buff+offset, 12 + len);      // rtp payload length
#endif
	}

#ifdef RTSP_REPLAY
    if (p_rua->replay)
    {
        *(buff+offset) = (RTP_VERSION << 6) | (1 << 4);
    }
    else
    {
        *(buff+offset) = (RTP_VERSION << 6);
    }
#else
    *(buff+offset) = (RTP_VERSION << 6);
#endif
	offset++;
	
	*(buff+offset) = (p_rua->channels[av_t].rtp_info.rtp_pt) | ((mbit & 0x01) << 7);
	offset++;

	offset += rtp_write_uint16(buff+offset, p_rua->channels[av_t].rtp_info.rtp_cnt);

	offset += rtp_write_uint32(buff+offset, p_rua->channels[av_t].rtp_info.rtp_ts);

	offset += rtp_write_uint32(buff+offset, p_rua->channels[av_t].rtp_info.rtp_ssrc);

#ifdef RTSP_REPLAY
    if (p_rua->replay)
    {
        offset += rtp_write_uint16(buff+offset, 0xABAC);
        offset += rtp_write_uint16(buff+offset, 3);

        offset += rtp_write_uint32(buff+offset, p_rua->channels[av_t].rep_hdr.ntp_sec);
        offset += rtp_write_uint32(buff+offset, p_rua->channels[av_t].rep_hdr.ntp_frac);

        *(buff+offset) = ((p_rua->channels[av_t].rep_hdr.t & 0x01) << 4) | 
                         ((p_rua->channels[av_t].rep_hdr.d & 0x01) << 5) | 
                         ((p_rua->channels[av_t].rep_hdr.e & 0x01) << 6) | 
                         ((p_rua->channels[av_t].rep_hdr.c & 0x01) << 7) | 
                         p_rua->channels[av_t].rep_hdr.mbz;
    	offset++;
    	
        *(buff+offset) = p_rua->channels[av_t].rep_hdr.seq;
    	offset++;

    	offset += rtp_write_uint16(buff+offset, p_rua->channels[av_t].rep_hdr.padding);

    	p_rua->channels[av_t].rep_hdr.d = 0;
	}
#endif

	p_rtp_ptr = p_data - offset;
	memcpy(p_rtp_ptr, buff, offset);
	
	if (p_rua->rtp_tcp)
	{
		slen = rtp_tcp_tx(p_rua, p_rtp_ptr, offset+len);
		if (slen != (offset+len))
		{
			return -1;
		}
	}
	else
	{
		slen = rtp_udp_tx(p_rua, av_t, p_rtp_ptr, offset+len);
		if (slen != offset+len)
		{
			return -1;
		}
	}

    p_rua->channels[av_t].rtp_info.rtp_cnt++;

#ifdef RTSP_RTCP
    p_rua->channels[av_t].rtcp_info.octet_count += len;
    p_rua->channels[av_t].rtcp_info.packet_count++;
#endif
    
	return slen;
}

/**
 * Build video rtp packet and send (not fragment)
 *
 * @param p_rua rtsp user agent
 * @param p_data payload data
 * @param len payload data length
 * @ts the packet timestamp
 * @return 1 on success, -1 on error
 */
int rtp_video_tx(RSUA * p_rua, uint8 * p_data, int size, uint32 ts)
{
	int ret = 0;
	int len, max_packet_size;
	uint8 * p = p_data;
	
    max_packet_size = 1460;	
	
    while (size > 0) 
    {
        len = max_packet_size;
        if (len > size)
        {
            len = size;
		}
		
        ret = rtp_build(p_rua, AV_TYPE_VIDEO, p, len, ts, (len == size));
        if (ret < 0)
		{
			break;
		}

        p += len;
        size -= len;
    }

    return ret;
}

/**
 * Judgment and segmentation h264 rtp data into a single package
 *
 * @param fu_flag fragment flag
 * @param fu_s fragment start flag
 * @param fu_e fragment end flag
 * @param len data length
 * @return the fragmention length
 */
int rtp_h264_fu_split(int * fu_flag, int * fu_s, int * fu_e, int len)
{
	if ((*fu_flag) == 0) // have not yet begun fragment
	{
		if (len <= H264_RTP_MAX_LEN) return len;	// Need not be fragmented

		*fu_flag = 1;
		*fu_s = 1;
		*fu_e = 0;
		
		return H264_RTP_MAX_LEN;
	}
	else // It has begun to fragment
	{
		*fu_s = 0;
		
		if (len <= H264_RTP_MAX_LEN) // End fragmentation
		{
			*fu_e = 1;
			return len;
		}
		else
		{
			return H264_RTP_MAX_LEN;
		}
	}
}

/**
 * Build and send h264 fragement rtp packet
 *
 * @param p_rua rtsp user agent
 * @nalu_t the NALU type
 * @fu_s fragement start flag
 * @fu_e fragement end start
 * @param p_data payload data
 * @param len payload data length
 * @return the rtp packet length, -1 on error
 */
int rtp_h264_single_fu_build(RSUA * p_rua, uint8 nalu_t, int fu_s, int fu_e, uint8 * p_data, int len)
{
	int mbit = 0;
	int slen;
	int offset = 0;	
	uint8 buff[40];
	uint8 * p_rtp_ptr;

#ifdef RTSP_RTCP
	rtcp_send_packet(p_rua, AV_TYPE_VIDEO);
#endif

	if (p_rua->rtp_tcp)
	{
		*(buff+offset) = 0x24; // magic
		offset++;
		
		*(buff+offset) = p_rua->channels[AV_VIDEO_CH].interleaved;  // channel
		offset++;
		
#ifdef RTSP_REPLAY
        if (p_rua->replay)
        {
            offset += rtp_write_uint16(buff+offset, 12 + 2 + len + 16); // rtp payload length
        }
        else
        {
            offset += rtp_write_uint16(buff+offset, 12 + 2 + len);  // rtp payload length
        }
#else
        offset += rtp_write_uint16(buff+offset, 12 + 2 + len);      // rtp payload length
#endif		
	}

	if (fu_e == 1)
	{
		mbit = 1;
	}	
	else
	{
		mbit = 0;
	}

#ifdef RTSP_REPLAY
	if (p_rua->replay)
    {
        *(buff+offset) = (RTP_VERSION << 6) | (1 << 4);
    }
    else
    {
        *(buff+offset) = (RTP_VERSION << 6);
    }
#else
    *(buff+offset) = (RTP_VERSION << 6);
#endif	
	offset++;
	
	*(buff+offset) = (p_rua->channels[AV_VIDEO_CH].rtp_info.rtp_pt) | ((mbit & 0x01) << 7);
	offset++;

	offset += rtp_write_uint16(buff+offset, p_rua->channels[AV_VIDEO_CH].rtp_info.rtp_cnt);

	offset += rtp_write_uint32(buff+offset, p_rua->channels[AV_VIDEO_CH].rtp_info.rtp_ts);

	offset += rtp_write_uint32(buff+offset, p_rua->channels[AV_VIDEO_CH].rtp_info.rtp_ssrc);

#ifdef RTSP_REPLAY
    if (p_rua->replay)
    {
        offset += rtp_write_uint16(buff+offset, 0xABAC);
        offset += rtp_write_uint16(buff+offset, 3);

        offset += rtp_write_uint32(buff+offset, p_rua->channels[AV_VIDEO_CH].rep_hdr.ntp_sec);
        offset += rtp_write_uint32(buff+offset, p_rua->channels[AV_VIDEO_CH].rep_hdr.ntp_frac);

        *(buff+offset) = ((p_rua->channels[AV_VIDEO_CH].rep_hdr.t & 0x01) << 4) | 
                         ((p_rua->channels[AV_VIDEO_CH].rep_hdr.d & 0x01) << 5) | 
                         ((p_rua->channels[AV_VIDEO_CH].rep_hdr.e & 0x01) << 6) | 
                         ((p_rua->channels[AV_VIDEO_CH].rep_hdr.c & 0x01) << 7) | 
                         p_rua->channels[AV_VIDEO_CH].rep_hdr.mbz;
    	offset++;
    	
        *(buff+offset) = p_rua->channels[AV_VIDEO_CH].rep_hdr.seq;
    	offset++;

        offset += rtp_write_uint16(buff+offset, p_rua->channels[AV_VIDEO_CH].rep_hdr.padding);

    	p_rua->channels[AV_VIDEO_CH].rep_hdr.d = 0;
	}
#endif	

	buff[offset] = (nalu_t & 0x60) | 28;
	offset++;
	
	buff[offset] = (nalu_t & 0x1F);

	if (fu_s == 1)
	{
		buff[offset] |= 0x80;
	}
	
	if (fu_e == 1)
	{
		buff[offset] |= 0x40;
	}
	
	offset++;

	p_rtp_ptr = p_data - offset;
	memcpy(p_rtp_ptr, buff, offset);
	
	if (p_rua->rtp_tcp)
	{
		slen = rtp_tcp_tx(p_rua, p_rtp_ptr, offset+len);
		if (slen != (offset+len))
		{
			return -1;
		}
	}
	else
	{
		slen = rtp_udp_tx(p_rua, AV_TYPE_VIDEO, p_rtp_ptr, offset+len);
		if (slen != offset+len)
		{
			return -1;
		}
	}

	p_rua->channels[AV_VIDEO_CH].rtp_info.rtp_cnt++;
	
#ifdef RTSP_RTCP
	p_rua->channels[AV_VIDEO_CH].rtcp_info.octet_count += len;
    p_rua->channels[AV_VIDEO_CH].rtcp_info.packet_count++;
#endif

	return slen;
}

/**
 * Send h264 rtp packet
 *
 * @param p_rua rtsp user agent
 * @ts the packet timestamp
 * @nalu_t the NALU type
 * @fu_flag fragment flag
 * @fu_s fragement start flag
 * @fu_e fragement end start
 * @param p_data payload data
 * @param len payload data length
 * @return the rtp packet length, -1 on error
 */
int rtp_h264_tx(RSUA * p_rua, uint32 ts, uint8 nalu_t, int fu_flag, int fu_s, int fu_e, uint8 * p_data, int len)
{
	int ret = 0;

	p_rua->channels[AV_VIDEO_CH].rtp_info.rtp_ts = ts;
	
	if (fu_flag == 0)
	{		
		ret = rtp_build(p_rua, AV_TYPE_VIDEO, p_data, len, ts, 1);
	}
	else
	{
		ret = rtp_h264_single_fu_build(p_rua, nalu_t, fu_s, fu_e, p_data, len);
	}

	return ret;
}

/**
 * Build h264 video rtp packet and send
 *
 * @param p_rua rtsp user agent
 * @param p_data payload data
 * @param len payload data length
 * @ts the packet timestamp
 * @i_flag the I-Frame flag
 * @return 1 on success, -1 on error
 */
int rtp_h264_video_pkt_tx(RSUA * p_rua, uint8 * p_data, int len, uint32 ts)
{
	int ret = 1;
	int frame_len = len;
	int fu_s = 0, fu_e = 0, fu_flag = 0;	// Fragement start, end flag
	uint8 * p_tx_data = p_data;
	uint8 nalu_t = p_tx_data[0];
	int keyframe = ((nalu_t & 0x1F) == 5);

#ifdef RTSP_REPLAY    
    p_rua->channels[AV_VIDEO_CH].rep_hdr.c = keyframe;
#endif

	while (frame_len > 0)
	{
		int tlen = rtp_h264_fu_split(&fu_flag, &fu_s, &fu_e, frame_len);
		if (fu_flag == 1)
		{
			if (fu_s == 1)
			{
				p_tx_data++;
				tlen--;
				frame_len--;
			}
		}

		// the sps and pps frame
		if ((nalu_t & 0x1F) == 7 || (nalu_t & 0x1F) == 8)
		{
			ret = rtp_h264_tx(p_rua, ts, nalu_t, fu_flag, fu_s, fu_e, p_tx_data, tlen);
			if (ret == -1)
			{
				break;
			}
		}
		else if (p_rua->iframe_tx == 0 && 0 == keyframe)
		{
			// Non-I-frame, not yet sent I-frame, skip P frame
		}
#ifdef RTSP_REPLAY
        else if (p_rua->replay && 2 == p_rua->frame && 0 == keyframe) 
        {
            // only send I-frame
        }
#endif
		else
		{
			ret = rtp_h264_tx(p_rua, ts, nalu_t, fu_flag, fu_s, fu_e, p_tx_data, tlen);
			if (ret == -1)
			{
				break;
			}
			
			p_rua->iframe_tx = 1;
		}
		
		p_tx_data += tlen;
		frame_len -= tlen;
	}

	return ret;
}

/**
 * Build h264 video rtp packet and send
 *
 * @param p_rua rtsp user agent
 * @param p_data payload data
 * @param len payload data length
 * @ts the packet timestamp
 * @return 1 on success, -1 on error
 */
int rtp_h264_video_tx(RSUA * p_rua, uint8 * p_data, int len, uint32 ts)
{
	int ret = 1;

	uint8 *r, *end = p_data + len;
	r = avc_find_startcode(p_data, end);

	while (r < end) 
	{
        uint8 *r1;
        
        while (!*(r++));
        r1 = avc_find_startcode(r, end);

		ret = rtp_h264_video_pkt_tx(p_rua, r, r1 - r, ts);
		if (ret < 0)
		{
			break;
		}
		
        r = r1;
    }

	return ret;
}

int rtp_h265_video_pkt_tx(RSUA * p_rua, uint8 * p_data, int len, uint32 ts)
{
	int frame_len = len;
	uint8 * p_tx_data = p_data;
	int rtp_payload_size = H265_RTP_MAX_LEN - 3;  
	int nal_type = (p_tx_data[0] >> 1) & 0x3F;
	
	/* send it as one single NAL unit? */  
	if (frame_len <= H265_RTP_MAX_LEN)
	{  
		rtp_build(p_rua, AV_TYPE_VIDEO, p_tx_data, frame_len, ts, 1);  
	}
	else
	{  
		/* 
		create the HEVC payload header and transmit the buffer as fragmentation units (FU) 
		                       
		0                   1 
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
		|F|   Type    |  LayerId  | TID | 
		+-------------+-----------------+ 

		F       = 0 
		Type    = 49 (fragmentation unit (FU)) 
		LayerId = 0 
		TID     = 1 
		*/ 
        
		uint8 tbuf[2048];
		uint8 * buf1 = tbuf+40;
		uint8 * buf = p_tx_data;
		
		buf1[0] = 49 << 1;  
		buf1[1] = 1;    
	 
		/* 
		create the FU header 
		                     
		0 1 2 3 4 5 6 7 
		+-+-+-+-+-+-+-+-+ 
		|S|E|  FuType   | 
		+---------------+ 
		                 
		S       = variable 
		E       = variable 
		FuType  = NAL unit type 
		*/  
		
		buf1[2] = nal_type;  
		/* set the S bit: mark as start fragment */  
		buf1[2] |= 1 << 7;       

		/* pass the original NAL header */  
		buf += 2;
		frame_len -= 2;  
        
		while (frame_len > rtp_payload_size)   
		{
			memcpy(buf1+3, buf, rtp_payload_size);  
			rtp_build(p_rua, AV_TYPE_VIDEO, buf1, H265_RTP_MAX_LEN, ts, 0);  
			  
			buf += rtp_payload_size;  
			frame_len -= rtp_payload_size;  

			/* reset the S bit */  
			buf1[2] &= ~(1 << 7);  
		}  

		/* set the E bit: mark as last fragment */  
		buf1[2] |= 1 << 6;  
		/* complete and send last RTP packet */  
		memcpy(buf1+3, buf, frame_len);  
		
		rtp_build(p_rua, AV_TYPE_VIDEO, buf1, frame_len + 3, ts, 1);
	}

	return 1;
}

int rtp_h265_video_tx(RSUA * p_rua, uint8 * p_data, int len, uint32 ts)
{	
	int ret = -1;

	uint8 *r, *end = p_data + len;
	r = avc_find_startcode(p_data, end);

	while (r < end) 
	{
        uint8 *r1;
        
        while (!*(r++));
        r1 = avc_find_startcode(r, end);

		ret = rtp_h265_video_pkt_tx(p_rua, r, r1 - r, ts);
		if (ret < 0)
		{
			break;
		}
		
        r = r1;
    }

	return ret;	 
}


/**
 * Build jpeg video rtp packet and send
 *
 * @param p_rua rtsp user agent
 * @param p_data payload data
 * @param len payload data length
 * @ts the packet timestamp
 * @return the send data length, -1 on error
 */
int rtp_jpeg_video_tx(RSUA * p_rua, uint8 * p_data, int size, uint32 ts)
{	
	int ret = 0;
	const uint8 *qtables[4] = { NULL };
    int nb_qtables = 0;
    uint8 type;
    uint8 w, h;
    uint8 *p;
    int off = 0; /* fragment offset of the current JPEG frame */
    int len;
    int i;
    int dri = 0;
    int default_huffman_tables = 0;

    /* convert video pixel dimensions from pixels to blocks */
    w = p_rua->media_info.v_width >> 3;
    h = p_rua->media_info.v_height >> 3;
    
    type = 1; // YUVJ420P, YUVJ422P is 0

    /* preparse the header for getting some infos */
    for (i = 0; i < size; i++)
    {
        if (p_data[i] != 0xff)
        {
            continue;
		}
		
        if (p_data[i + 1] == MARKER_DQT) 
        {
            int tables, j;
            
            if (p_data[i + 4] & 0xF0)
            {
            	log_print(HT_LOG_WARN, "%s, Only 8-bit precision is supported\r\n", __FUNCTION__);
            }

            /* a quantization table is 64 bytes long */
            tables = rtp_read_uint16(&p_data[i + 2]) / 65;
            if (i + 5 + tables * 65 > size)
            {      
            	log_print(HT_LOG_ERR, "%s, Too short JPEG header. Aborted!\r\n", __FUNCTION__);
                return -1;
            }
            
            if (nb_qtables + tables > 4) 
            {       
            	log_print(HT_LOG_ERR, "%s, Invalid number of quantisation tables\r\n", __FUNCTION__);
                return -1;
            }

            for (j = 0; j < tables; j++)
            {
                qtables[nb_qtables + j] = p_data + i + 5 + j * 65;
            }    
            nb_qtables += tables;
        } 
        else if (p_data[i + 1] == MARKER_SOF0) 
        {
            if (p_data[i + 14] != 17 || p_data[i + 17] != 17)
            {
                log_print(HT_LOG_ERR, "%s, Only 1x1 chroma blocks are supported. Aborted!\r\n", __FUNCTION__);
                return -1;
            }

            type = (p_data[i + 11] & 1) ? 0 : 1;
        } 
        else if (p_data[i + 1] == MARKER_DHT) 
        {
            int dht_size = rtp_read_uint16(&p_data[i + 2]);
            default_huffman_tables |= 1 << 4;
            i += 3;
            dht_size -= 2;
            if (i + dht_size >= size)
            {
                continue;
            }
            
            while (dht_size > 0)
            {
                switch (p_data[i + 1]) 
                {
                case 0x00:
                    if (   dht_size >= 29
                        && !memcmp(p_data + i +  2, lum_dc_codelens, 16)
                        && !memcmp(p_data + i + 18, lum_dc_symbols, 12))
                    {
                        default_huffman_tables |= 1;
                        i += 29;
                        dht_size -= 29;
                    } 
                    else 
                    {
                        i += dht_size;
                        dht_size = 0;
                    }
                    break;
                    
                case 0x01:
                    if (   dht_size >= 29
                        && !memcmp(p_data + i +  2, chm_dc_codelens, 16)
                        && !memcmp(p_data + i + 18, lum_dc_symbols, 12)) 
                    {
                        default_huffman_tables |= 1 << 1;
                        i += 29;
                        dht_size -= 29;
                    }
                    else 
                    {
                        i += dht_size;
                        dht_size = 0;
                    }
                    break;
                    
                case 0x10:
                    if (   dht_size >= 179
                        && !memcmp(p_data + i +  2, lum_ac_codelens, 16)
                        && !memcmp(p_data + i + 18, lum_ac_symbols, 162)) 
                    {
                        default_huffman_tables |= 1 << 2;
                        i += 179;
                        dht_size -= 179;
                    }
                    else 
                    {
                        i += dht_size;
                        dht_size = 0;
                    }
                    break;
                    
                case 0x11:
                    if (   dht_size >= 179
                        && !memcmp(p_data + i +  2, chm_ac_codelens, 16)
                        && !memcmp(p_data + i + 18, chm_ac_symbols, 162))
                    {
                        default_huffman_tables |= 1 << 3;
                        i += 179;
                        dht_size -= 179;
                    } 
                    else 
                    {
                        i += dht_size;
                        dht_size = 0;
                    }
                    break;
                    
                default:
                    i += dht_size;
                    dht_size = 0;
                    continue;
            	}
            }
        } 
        else if (p_data[i + 1] == MARKER_DRI)
        {
            dri = p_data[i + 5];
        }
        else if (p_data[i + 1] == MARKER_SOS)
        {
            /* SOS is last marker in the header */
            i += rtp_read_uint16(&p_data[i + 2]) + 2;
            if (i > size) 
            {
               	log_print(HT_LOG_ERR, "%s, Insufficient data. Aborted!\r\n", __FUNCTION__);
                return -1;
            }
            break;
        }
    }
    
    if (default_huffman_tables && default_huffman_tables != 31) 
    {
        log_print(HT_LOG_ERR, "%s, RFC 2435 requires standard Huffman tables for jpeg\r\n", __FUNCTION__);
        return -1;
    }
    
    if (nb_qtables && nb_qtables != 2)
    {
    	// log_print(HT_LOG_INFO, "%s, RFC 2435 suggests two quantization tables, %d provided\r\n", __FUNCTION__, nb_qtables);
    }

    /* skip JPEG header */
    p_data += i;
    size -= i;

    for (i = size - 2; i >= 0; i--) 
    {
        if (p_data[i] == 0xff && p_data[i + 1] == MARKER_EOI) 
        {
            /* Remove the EOI marker */
            size = i;
            break;
        }
    }

	uint8 buff[2048];	
    p = buff + 40;
    
    while (size > 0) 
    {
        int hdr_size = 8;

        if (dri > 0)
        {
            type |= 0x40;
            hdr_size += 4;
        }
        
        if (off == 0 && nb_qtables)
        {
            hdr_size += 4 + 64 * nb_qtables;
		}
		
        /* payload max in one packet */
        len = MIN(size, JPEG_RTP_MAX_LEN - hdr_size);

		/* set main header */
		*p = 0;
		p++;
		p[0] = (off >> 16);
		p[1] = (off >> 8);
		p[2] = off;
		p += 3;
		*p = type;
		p++;

		if (nb_qtables)
		{
		    *p = 255;
		}
		else
		{
		    *p = 90;
		}
		
		p++;
		*p = w;
		p++;
		*p = h;
		p++;

        if (dri > 0)
        {
            // There is also a Restart Marker Header
            
            p[0] = dri >> 8;
            p[1] = dri & 0xFF;
            p[2] = p[3] = 0xFF;         // F=L=1; Restart Count = 0x3FFF
            p += 4;
        }
        
        if (off == 0 && nb_qtables)
        {
            /* set quantization tables header */
			*p = 0;
			p++;
			*p = 0;
			p++;
			p[0] = ((64 * nb_qtables) >> 8);
			p[1] = 64 * nb_qtables;
			p += 2;

            for (i = 0; i < nb_qtables; i++)
            {
            	memcpy(p, qtables[i], 64);
    			p += 64;
            }
        }

        /* copy payload data */
        memcpy(p, p_data, len);

        /* marker bit is last packet in frame */
        ret = rtp_build(p_rua, AV_TYPE_VIDEO, buff + 40, len + hdr_size, ts, size == len);
        if (ret < 0)
        {
        	return -1;
        }
 
        p_data += len;
        size -= len;
        off += len;
        p = buff + 40;
    }

    return off;
}

/**
 * Build audio rtp packet and send (not fragment)
 *
 * @param p_rua rtsp user agent
 * @param p_data payload data
 * @param len payload data length
 * @ts the packet timestamp
 * @return the rtp packet length, -1 on error
 */
int rtp_audio_tx(RSUA * p_rua, uint8 * p_data, int size, uint32 ts)
{
	int ret = 0;
	int len, max_packet_size;
	uint8 * p = p_data;
	
    max_packet_size = 1452 - 4 - 12 - 16;
	
    while (size > 0) 
    {
        len = max_packet_size;
        if (len > size)
        {
            len = size;
		}
		
        ret = rtp_build(p_rua, AV_TYPE_AUDIO, p, len, ts, (len == size));
        if (ret < 0)
		{
			break;
		}

        p += len;
        size -= len;
    }

    return ret;
}

/**
 * Build AAC audio rtp packet and send
 *
 * @param p_rua rtsp user agent
 * @param p_data payload data
 * @param size payload data length
 * @ts the packet timestamp
 * @return 1 on success, -1 on error
 */
int rtp_aac_audio_tx(RSUA * p_rua, uint8 * p_data, int size, uint32 ts)
{
	int ret = 0;
    int len, max_packet_size;
    uint8 *p = p_data - 4;
    int au_size = size;

    max_packet_size = 1452 - 4 - 12 - 16;
    
    while (size > 0) 
    {
        len = MIN(size, max_packet_size);

        rtp_write_uint16(p, 2 * 8);
        rtp_write_uint16(&p[2], au_size * 8);
        
        ret = rtp_build(p_rua, AV_TYPE_AUDIO, p, len + 4, ts, len == size);
        if (ret < 0)
		{
			break;
		}
        
        size -= len;
        p += len;
    }

    return ret;
}


#ifdef RTSP_METADATA

int rtp_metadata_tx(RSUA * p_rua, uint8 * p_data, int size, uint32 ts)
{
	int ret = 0;
	int len, max_packet_size;
	uint8 * p = p_data;
	
    max_packet_size = 1452 - 4 - 12 - 16;
	
    while (size > 0) 
    {
        len = max_packet_size;
        if (len > size)
        {
            len = size;
		}
		
        ret = rtp_build(p_rua, AV_TYPE_METADATA, p, len, ts, (len == size));
        if (ret < 0)
		{
			break;
		}

        p += len;
        size -= len;
    }

    return ret;
}

#endif



