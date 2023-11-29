/*
 * File      : rtsp_server.h
 * This file is RTSP Server interface header  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-01-08     fengyong    first version
 */

#ifndef ZM_RTSP_SERVER_H
#define ZM_RTSP_SERVER_H

#include "types_def.h"
#include "RtspCaster.h"


#define RTSP_SERVER_PORT    (554)
#define UDPTS_FRAME_NUM 	(25)//(25)
#define	PTS_NUM				(1000000 / UDPTS_FRAME_NUM)

enum ht_rtsp_e {
    HT_RTSP_CHANNEL_MAX = 2,		
	HT_RTSP_HANDLE_COUNT_MAX = 128,			
};

typedef struct {        
    HANDLE handle[HT_RTSP_HANDLE_COUNT_MAX];

    uint32_t frame_count;
    
    uint64_t rtsp_pts[HT_RTSP_CHANNEL_MAX];
    uint64_t last_video_pts[HT_RTSP_CHANNEL_MAX];

} rtsp_server_t;

#ifdef __cplusplus
extern "C" {
#endif

int rtsp_server_init(int port, int frame_width, int frame_height);
void rtsp_server_deinit(void);
void rtsp_server_push_video(int ch_index,uint8_t *frame_buf, int frame_size);
void happytime_rtsp_server_data_input(int stream_index,unsigned char* pkt_data,int pkt_size);
void zm_video_data_input(int stream_index,unsigned char* pkt_data,int pkt_size);
#ifdef __cplusplus
}
#endif

#endif /* ZM_RTSP_SERVER_H */
