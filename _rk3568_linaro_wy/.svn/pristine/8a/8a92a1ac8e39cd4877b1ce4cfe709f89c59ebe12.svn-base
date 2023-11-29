/*
 * File      : rtsp_client.h
 * This file is RTSP client interface header  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-08     fengyong    first version
 */

#ifndef ZM_RTSP_CLIENT_H
#define ZM_RTSP_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif




typedef struct zm_rtsp_client_info{
    char rtsp_client_ip[64];
    int rtsp_client_port;
    int rtsp_client_protocol;//0 udp  1 tcp
    char rtsp_client_stream_name[512];
    char rtmp_ip[512];
    char rtmp_gw[512];
    int  venc_bitrate;
    int  rtsp_server_port;
    int  tcp_ctl_port;
    int rtsp_server_open_flag;
    int rtsp_server_w;
    int rtsp_server_h;
    char mask_ip[128];
}zm_rtsp_info_t;


//int rtsp_client_init(void);
//void rtsp_client_deinit(void);
//void *  rtsp_notify_handler_my(void *arg);

extern zm_rtsp_info_t gs_rtsp_info;

#ifdef __cplusplus
}
#endif

#endif /* ZM_RTSP_CLIENT_H */
