/*
 * File      : video_common.h
 * This file is video common header  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-23     fengyong    first version
 */

#ifndef ZM_VIDEO_COMMON_H_
#define ZM_VIDEO_COMMON_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "sys_config.h"
#include "GetSysInfo.h"

#define 	INTER_FRAME_OUT_NAME		"eth1"
#define 	INTER_FRAME_IN_NAME			"eth0"

enum video_common_e {
    VIDEO_FRAME_HEIGHT = 1080,
    VIDEO_FRAME_HEIGHT_STRIDE = 1080,	
	VIDEO_FRAME_WIDTH = 1920,
	VIDEO_RESIZE_WIDTH = 1280,
	VIDEO_RESIZE_HEIGHT = 720,			
};

enum ir_video_common_e {    
    IR_VIDEO_FRAME_HEIGHT = 512,   
	IR_VIDEO_FRAME_WIDTH = 640,			
};


typedef enum {
	TYPE_NO = 0,
	TYPE_HDMI,
	TYPE_RTSP,
	TYPE_TS,
	TYPE_CAMERA,
	TYPE_RTMP,
	TYPE_MP4,
	TYPE_H264,
	TYPE_H265
}VIDEO_INOUT_TYPE;

typedef enum {
	TYPE_E = 0,
	TYPE_IR,
	TYPE_TV,
}CAMERA_INPUT_TYPE;


typedef enum{
	ALG_STOP,	
	ALG_RUN,
	ALG_SERCHING,
	ALG_MISS,
}my_alg_stat;


typedef struct
{
	volatile unsigned int user_state;//用户控制
	volatile unsigned int search_count;
	volatile unsigned int init_state;//
	volatile unsigned int run_state;//
	unsigned int delay_count;
	int trace_shizi_x;
	int trace_shizi_y;
}TRACK_ALG_STATE_T;


typedef struct
{
	 volatile unsigned int user_state;//用户控制
	 volatile unsigned int init_state;//
	 volatile unsigned int run_state;//
	 volatile unsigned int use_to_track;//
}RKNN_ALG_STATE_T;


typedef struct
{
     int    x;
     int    y;
    unsigned int    w;
    unsigned int    h;
	int class_id;
	float prop;
	int has_target;
} RECTH;

typedef enum {
	PIP_NO = 0,
	PIP_TV,
	PIP_IR,
}PIP_STAT;

typedef enum {
	OSD_ON = 0,
	OSD_OFF,
}OSD_STAT;

typedef struct
{
    VIDEO_INOUT_TYPE  input_type;
    VIDEO_INOUT_TYPE  output_type;
    volatile int camera_type;
	int osd_color;
	PIP_STAT old_pip_stat;
	PIP_STAT pip_stat;
	int osd_stat;
	int osd_sz_stat;
	int osd_gps_stat;
	int osd_tg_gps_stat;
	int osd_jd_stat;
	int osd_ir_stat;
	int osd_lrf_stat;
	int osd_eo_stat;
	int osd_time_stat;
	int osd_tf_stat;
	int osd_rknn_stat;
	int  osd_gps_type;
	int  osd_gps_float;
	int  osd_ir_temp;
	int vi_fps;
	int vo_fps;
	int hdmi_vo_fps;
	int encode_1080p;
	int video_save_type;
    int minimum_quality;    // 修改GOP为60，降低I帧MAX/MIN QP 为40，也即最低质量
    int low_rtsp_fps;       // 降低fps为15，实测为12.5；
}zm_vedio_info_t;



enum STM32_UP_STAT{
	STM32_NO_FILE=0,
	STM32_CONNECTING,
	STM32_SENDING,
	STM32_SUCCESS,
	STM32_FAILED,
};

typedef enum {
    RKNN_CLASS_CAR = 0,
    RKNN_CLASS_PERSON,
    RKNN_CLASS_BOTH,
}RKNN_CLASS_T;

typedef struct SelfCheck_Info
{
	int uart_init_flag;
	int ir_protection_flag;
	char computer_stat;//上位机：0 结果正常；1 结果异常
	char servo_stat;//伺服
	char infrared_type;//跟踪器
	char infrared_stat;//红外
	char ir_init_done;//红外main函数里初始完成
	char visible_stat;//可见光
	char laser_stat;//激光
	char input_stat;
	char output_stat;
	char soft_ver[128];
	char rk_eth0_ip[64];
	int  update_show;
	int  update_flag;
	int  webport;
	int  udp_port;
	int  stm32_update_stat;
	int  stm32_update_percent;
	int  cpu_temp;
	int  ip_bak;
	int  videorb_full_cnt;
	int  videorb_in_cnt;
	int  videorb_out_cnt;
	unsigned long long  videorb_wr_byte;
	float videorb_wr_speed;
	int ir_err_stat;
	int ir_nosig;
	int vis_err_stat;
	char chipid[32];
	char hw_ver[32];
	int vis_rknn_err_stat;
	int ir_rknn_err_stat;
	char ir_ver_date[64];
    RKNN_CLASS_T rknn_class;
}selfcheck_info_t;

typedef enum  {
     RECORD_STOP=0,
     RECORDING,
	 RECORDONCE,
	 RECORDSAVING,				
}record_stat_e;

typedef enum {
	SD_Inserted=0,
	SD_Inited,
	SD_ReadOnly,
	SD_formatted,
	SD_formatting,
	SD_full,
	SD_verified,
	SD_invalid
}SDCARD_STAT_E;

typedef struct {				
	unsigned int sdcard_total;
	unsigned int sdcard_space;
	volatile unsigned int sdcard_stat;
	char sdcard_stat_send;
    volatile unsigned int record_user_stat;
	volatile unsigned int record_run_stat;

	volatile unsigned int ir_record_run_stat;
	volatile unsigned int ir_record_user_stat;
	unsigned int record_min_num;	
	unsigned int record_total;
	volatile unsigned int ready_to_copy;
}record_info_t;

typedef struct ir_temp_info
{
    unsigned short x_tl;
    unsigned short y_tl;
    float tl;
    unsigned short x_th;
    unsigned short y_th;
    float th;
	float tc;
}ir_temp_info_t;


enum photo_stat_e {
     PHOTO_STOP=0,
     PHOTO_ONECE,
     PHOTO_CONTINUE,			
};
typedef struct {				
    volatile unsigned int photo_stat;
	unsigned int photo_total;
	unsigned int photo_stat_show;
	volatile unsigned int photo_catch_stat;
//#ifdef  SAVE_RAW_DATA 
	int test_stat0;	
	int test_stat1;
	int test_stat2;
//#endif
}photo_info_t;


extern photo_info_t  gs_photo_info;
extern record_info_t gs_record_info;
extern zm_vedio_info_t gs_vedio_info;
extern RKNN_ALG_STATE_T gs_nnie_state;
extern TRACK_ALG_STATE_T track_state_mechine;
extern selfcheck_info_t Selfcheck_info;
extern int pthread_exit_control;
extern time_info_t gs_time_info;
extern time_info_t gs_rec_time_info;
extern ir_temp_info_t  gs_ir_temp_info;
extern int update_osd_show(void);
extern void analyze_temp_by_ir_param(short *ir_param_data);
#ifdef __cplusplus
}
#endif

#endif /* ZM_VIDEO_COMMON_H */
