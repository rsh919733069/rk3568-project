/*
 * File      : rtsp_server.c
 * This file is RTSP Server interface  file  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-08     fengyong    first version
 */

#include "rtsp_server.h"
#include "video_common.h"
#include "queue.h"
#include "log/log.h"

#include "data_engine.h"
static rtsp_server_t *gs_rtsp_server = NULL;

//happytime rtsp server globle param
QueueStruct rtsp_srv_ServerQueue[4];
int rtsp_srv_length[4][1000] = {0};

// /* RTSP Server using HTRtsp to Push RTSP Stream */
// int rtsp_server_init(int port, int frame_width, int frame_height)
// {
//     rtsp_server_t *rtsp_server = NULL;

//     rtsp_server = (rtsp_server_t *)malloc(sizeof(*rtsp_server));
//     if (!rtsp_server) {
//         printf("XF:malloc rtsp server failed\n");
//         return RET_ERR_MALLOC;
//     }

//     memset(rtsp_server, 0,sizeof(*rtsp_server));

//     caster_init(port, "0.0.0.0");

// 	MFormat fmt = {0};
// 	fmt.codec = MCODEC_H264;
// 	fmt.width = frame_width;
// 	fmt.height = frame_height;
// 	fmt.framerate = UDPTS_FRAME_NUM;
// 	fmt.profile = 77;
// 	fmt.clockRate = 1000000;//2000000;
// 	fmt.bitrate = 1024 * 2; //*4
// 	fmt.audioCodec = MCODEC_NONE;

// 	caster_chl_open(&rtsp_server->handle[0], "554", &fmt);
//     caster_tcp_force_complete_frame(1);
//     //MFormat fmt = {0};
// 	// fmt.codec = MCODEC_H264;
// 	// fmt.width = frame_width;
// 	// fmt.height = frame_height;
// 	// fmt.framerate = UDPTS_FRAME_NUM;
// 	// fmt.profile = 77;
// 	// fmt.clockRate = 1000000; //2000000;
// 	// fmt.bitrate = 1024 * 2; //*4
// 	// fmt.audioCodec = MCODEC_NONE;
//     // caster_chl_open(&rtsp_server->handle[1], "chn1", &fmt);
//      gs_rtsp_server = rtsp_server;

//     printf("XF:rtsp server init ok\n");

//     return RET_OK;
// } 

// rtsp_server_t *rtsp_server_get(void)
// {
//     return gs_rtsp_server;
// }

// void rtsp_server_deinit(void)
// {
//     rtsp_server_t *rtsp_server = rtsp_server_get();

//     if (rtsp_server) {
//         free(rtsp_server);
//         rtsp_server = NULL;   
//     }
// }

// void rtsp_server_push_video(int ch_index,uint8_t *frame_buf, int frame_size)
// {
//     uint64_t pts_diff = 0;
//     rtsp_server_t *rtsp_server = rtsp_server_get();
//     int pts_num_tmp=1000000/gs_vedio_info.vo_fps;
//     rtsp_server->rtsp_pts[ch_index] += pts_num_tmp;
    
//     MPacket pkt = {0};
//     pkt.type = MTYPE_VIDEO;
//     pkt.data = (uint8_t *)(frame_buf);
//     pkt.size = frame_size;
//     pkt.duration = pts_diff;

//     //pkt.pts =  stStream.pstPack[n].u64PTS;
//     pkt.pts = rtsp_server->rtsp_pts[ch_index];

//     //pkt.flags = (rtsp_server->frame_count > 1) ? 1 : 0; //key 

//     pkt.flags = (frame_buf[4]&0x1f==6) ? 1 : 0;

//     rtsp_server->last_video_pts[ch_index] = pkt.pts;
        
//     caster_chl_write_video(rtsp_server->handle[ch_index], &pkt);

//     rtsp_server->frame_count++;

//     //SLOGI("+++%s++++\n",__func__);
// }

#ifdef H265_DEBUG
        // static int flag = 1;
        // static FILE *pFile;
        // static int frame_cnt = 0;
#endif

void happytime_rtsp_server_data_input(int stream_index,unsigned char* pkt_data,int pkt_size)
{

    // static int rtsp_srv_length_index[4] = {0};

    // if(rtsp_srv_ServerQueue[stream_index].m_dataBuf != NULL)
    // {
    //     rtsp_srv_length_index[stream_index] %= 1000;
    //     rtsp_srv_length[stream_index][rtsp_srv_length_index[stream_index]] = pkt_size;
    //     //SLOGI("pkt_size %d",pkt_size);
    //     rtsp_srv_length_index[stream_index]++;
    // }

    // int ret = QueuePush(&rtsp_srv_ServerQueue[stream_index],pkt_data,pkt_size);

#ifdef H265_DEBUG
        // frame_cnt++;
        // if(flag)
        // {
        //         pFile = fopen("/tmp/stream_temp.h265", "wb");
        //         if (!pFile)
        //         {
        //                 printf("open file err!\n");
        //         }    
        //         flag = 0;
        // }


        // fwrite(pkt_data, pkt_size, 1, pFile);
        // fflush(pFile);

        // if(frame_cnt > 30 * 10)
        // {
        //         fclose(pFile);
        //         SLOGI("/tmp/stream_temp.h265 has been closed\n");
        // }
#endif

        data_stream_t *data_stream = data_stream_get();  
        image_frame_info_t frame_info;
        if (data_stream->rtsp_data_offset +pkt_size > data_stream->rtsp_buf_total_size) {
                data_stream->rtsp_data_offset = 0;
        }
        memcpy(data_stream->rtsp_data+data_stream->rtsp_data_offset,pkt_data,pkt_size);
        frame_info.frame_offset=data_stream->rtsp_data_offset;
        frame_info.frame_index=pkt_size;
        rtsp_data_to_queue(data_stream,&frame_info);


        #ifdef RTMP_OUTPUT_ENABLE
        rtmp_data_to_queue(data_stream,&frame_info);
        #endif
        data_stream->rtsp_data_offset+=pkt_size;     
}

void zm_video_data_input(int stream_index,unsigned char* pkt_data,int pkt_size)
{
        data_stream_t *data_stream = data_stream_get();  
        image_frame_info_t frame_info;
        if (data_stream->h264_data_offset +pkt_size > data_stream->h264_buf_total_size) {
                data_stream->h264_data_offset = 0;
        }
        memcpy(data_stream->h264_data+data_stream->h264_data_offset,pkt_data,pkt_size);
        frame_info.frame_offset=data_stream->h264_data_offset;
        frame_info.frame_index=pkt_size;
        H264_data_to_queue(data_stream,&frame_info);
        data_stream->h264_data_offset+=pkt_size;       
}