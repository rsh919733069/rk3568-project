/*
 * File      : data_engine.h
 * This file is vide data engine interface header  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-1-4     fengyong    first version
 * 2021-6-22    fengyong    modify for CSI Camera and IR Camera 
 */

#ifndef _DATA_MANAGER_H
#define _DATA_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include "types_def.h"
#include "ringbuffer.h"
#include "hetero_interface.h"
#include "xf_rockchip_rga.h"
#include "GetSysInfo.h"

//#define THREAD_TIME_LOG 1

enum data_stream_e {
    IR_DATA_STREAM_RGB_FRAME_MAX = 20,
    DATA_STREAM_RGB_FRAME_MAX = 20,
    DATA_STREAM_YUV_FRAME_MAX = 20,
    //DATA_STREAM_FRAME_MAX = 24,				
};

typedef struct {				
	void * offset;
    uint32_t frame_offset;
	uint32_t frame_index;	
}image_frame_info_t;

typedef struct {				
    char tmp_name[256];
	char real_name[256];	
}video_frame_info_t;

typedef struct {    	
	
	uint32_t frame_count;

	ring_buffer_t *yuv_data_rb;
	ring_buffer_t *ir_yuv_data_rb;
    ring_buffer_t *rgb_tv_in_data_rb;
    ring_buffer_t *rgb_out_data_rb;
    ring_buffer_t *rgb_ir_in_data_rb;
    ring_buffer_t *rgb_zoom_data_rb;
    ring_buffer_t *h264_data_rb;
   
    ring_buffer_t *yuv_encode_data_rb;
    ring_buffer_t *osd_data_rb;
    ring_buffer_t *yuv_video_data_rb;
    ring_buffer_t *rtsp_data_rb;
    ring_buffer_t *rtmp_data_rb;

    ring_buffer_t *video_copy_rb;

    ring_buffer_t *yuv_out_rb;

	// pthread_mutex_t yuv_data_lock;
	// pthread_cond_t yuv_data_cond;

    // pthread_mutex_t yuv_encode_data_lock;
	// pthread_cond_t yuv_encode_data_cond;

    // pthread_mutex_t rga_event_lock;
	// pthread_cond_t rga_event_cond;

    // pthread_mutex_t rgb_zoom_data_lock;
	// pthread_cond_t rgb_zoom_data_cond;

    // pthread_mutex_t rgb_tv_data_lock;
	// pthread_cond_t rgb_tv_data_cond;

    /* YUV Image */
    uint8_t *yuv_data;
    uint8_t *yuv_720p_data;
    uint32_t yuv_buf_total_size;
    uint32_t yuv_data_offset;
    uint32_t yuv_buf_frame_size;

        /*encode output YUV Image */
    uint8_t *output_yuv_data;
    uint32_t output_yuv_buf_total_size;
    uint32_t output_yuv_data_offset;
    uint32_t output_yuv_buf_frame_size;

    uint32_t yuv_handle_frame_count;
    uint32_t yuv_encode_data_count;

    /* RGB Image */
    uint8_t *rgb_data;
    uint32_t rgb_buf_frame_size;
    uint32_t rgb_buf_total_size;
    uint32_t rgb_data_offset;

    
    uint32_t rgb_handle_data_count;

    /* RGB ZOOM Image */
    uint8_t *rgb_zoom_data;
    uint32_t rgb_zoom_buf_total_size;
    uint32_t rgb_zoom_data_offset;
    uint32_t rgb_zoom_buf_frame_size;

    uint32_t rgb_zoom_data_count;
    uint32_t rgb_zoom_handle_data_count;

    XfRockchipRga *rga_yuv_copy;
    XfRockchipRga *rga_rgb_copy;
    XfRockchipRga *rga_yuv_encode;

    //////////////////////////
    /* IR Camera Image */
    
    uint8_t *ir_yuv_data;
    uint32_t ir_buf_frame_size;
    uint32_t ir_buf_total_size;
    uint32_t ir_data_offset;
    
    uint32_t ir_handle_data_count;
    volatile uint32_t osd_count;


    // pthread_mutex_t uart_data_lock;
	// pthread_cond_t uart_data_cond;

    //H264
    uint8_t *h264_data;
    uint32_t h264_buf_frame_size;
    uint32_t h264_buf_total_size;
    uint32_t h264_data_offset;
    uint8_t *jpeg_yuv_buf;

    //rtsp
    uint8_t *rtsp_data;
    uint32_t rtsp_buf_frame_size;
    uint32_t rtsp_buf_total_size;
    uint32_t rtsp_data_offset;

    //cewen  ir
    ring_buffer_t *ir_cewen_data_rb;
    uint8_t *ir_cewen_data;
    uint32_t ir_cewen_buf_frame_size;
    uint32_t ir_cewen_buf_total_size;
    uint32_t ir_cewen_data_offset;

    //GPU
    void* hetero_hdr;
    void* buf_obj_i_tv[DATA_STREAM_RGB_FRAME_MAX];
    void* image_obj_i_tv[DATA_STREAM_RGB_FRAME_MAX];
    void* image_obj_i_ir[DATA_STREAM_RGB_FRAME_MAX];
    void* buf_obj_i_ir_tmp;
    void* img_obj_i_ir_tmp;
    void* buf_obj_i_ir[DATA_STREAM_RGB_FRAME_MAX];
    void* buf_obj_i_osd[DATA_STREAM_RGB_FRAME_MAX];

    void* buf_obj_o_gray[DATA_STREAM_RGB_FRAME_MAX];
    void* ir_obj_o_gray[DATA_STREAM_RGB_FRAME_MAX];
    void* image_i_ir_gray[DATA_STREAM_RGB_FRAME_MAX];
    void* buf_obj_o_rgb416[DATA_STREAM_RGB_FRAME_MAX];
    void* buf_obj_o_rgb1080[DATA_STREAM_RGB_FRAME_MAX];
    void* img_obj_o_rgb1080[DATA_STREAM_RGB_FRAME_MAX];
    void* buf_obj_o_yuv1080[DATA_STREAM_RGB_FRAME_MAX];
    void* img_obj_o_rgb720[DATA_STREAM_RGB_FRAME_MAX];
    void* buf_obj_o_rgb720[DATA_STREAM_RGB_FRAME_MAX];
    void* buf_obj_o_yuv720[DATA_STREAM_RGB_FRAME_MAX];
    void* buf_obj_out_drm1080;

    void* buf_obj_i_vyuy[DATA_STREAM_RGB_FRAME_MAX];

    void* buf_obj_i_ir_vyuy[DATA_STREAM_RGB_FRAME_MAX];

    hetero_world_st htr;
}data_stream_t;



#ifdef __cplusplus
extern "C" {
#endif
extern int dect;
extern char eco_test_buf[23*23*16];
int data_stream_init(void);
void data_stream_deinit(void);
data_stream_t *data_stream_get(void);
void csi_video_in(uint8_t *rgb888_buf,int flag);
unsigned char * get_drm_buf(void);
int  gpu_init(data_stream_t *data_stream);
void OSDInit(int wide, int high);
void H264_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info);
void ir_cewen_data_to_rb(unsigned char *ir_src_data,unsigned char *ir_param_data);
void ir_data_to_rb(unsigned char *ir_data,int flag);
void rtsp_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info);
void rtmp_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info);
void yuv_encode_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info);
#ifdef __cplusplus
}
#endif

#endif /* DATA_MANAGER_H */
