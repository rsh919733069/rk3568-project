


#ifndef ZM_SYS_CONFIG_H_
#define ZM_SYS_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif


//#define  ZM_MP5_XT 1
//#define  ZM_MP5_PL 1


//pl jiami jiemi  
//#define ZM_ENC_RKNN 1

#ifdef ZM_MP5_PL

#define V5_RKNN_W  480
#define V5_RKNN_H  256

#define VO_720P_ENABLE      0
#define IR_INPUT_ENABLE     0
#undef  TS_OUTPUT_ENABLE    
#undef  RTSP_INPUT_ENABLE   
#define HDMI_OUTPUT_ENABLE  0
#define RTSP_OUTPUT_ENABLE  1
#undef  CAN_SEND_ENABLE 
#define VIDEO_SAVE_ENABLE  0  
#define PHOTO_SAVE_ENABLE  0  
//#define CAMERA_INPUT_FORMAT_RGB24  1
#define CAMERA_INPUT_FORMAT_YVYU  1
#define PROC_USE_PL 1
#define OSD_USE_PL 1
#define H264_VIDEO_TYPE 1
//#define PHOTO_CATCH_ENABLE   1
#define HALF_AUTO_TRACE_SIZE 64
#endif


#define RTSP_OUTPUT_ENABLE  1
#define RTMP_OUTPUT_ENABLE  1


#ifdef ZM_MP5_XT

#define V5_RKNN_W  480
#define V5_RKNN_H  256

//#define VO_720P_ENABLE      1
//#define IR_INPUT_ENABLE     1
#define  TS_OUTPUT_ENABLE     1    
#undef  RTSP_INPUT_ENABLE   
#define HDMI_OUTPUT_ENABLE  1
#define RTSP_OUTPUT_ENABLE  1
#undef  CAN_SEND_ENABLE 
#define VIDEO_SAVE_ENABLE  1  
#define PHOTO_SAVE_ENABLE  1  
//#define CAMERA_INPUT_FORMAT_RGB24  1
#define CAMERA_INPUT_FORMAT_YVYU  1
#define PROC_USE_ZM 1
#define OSD_USE_XT 1
//#define H264_VIDEO_TYPE 1
//#define PHOTO_CATCH_ENABLE   1
#define HALF_AUTO_TRACE_SIZE 64
#endif



#define SYSTEM_SETTING_DEFAULT_JSON_FILE		        "/zmv_default.json"	//	"/tmp/real_part/config/zmv_default.json"
#define SYSTEM_SETTING_CUSTOM_JSON_FILE					"/userdata/userconfig/user_custom.json"
#define SYSTEM_SETTING_TMP_CUSTOM_JSON_FILE				"/userdata/userconfig/new_custom.json"

#define RK_TRACK_LIB_PATH	 "/tmp/real_part/lib/zmvision/libtrack.so"
#define RK_GPU_LIB_PATH      "/tmp/real_part/lib/zmvision/libhetero.so"

#define TV_MODEL_PATH	"/tmp/real_part/data/yolov3_tiny_tv.rknn"
#define IR_MODEL_PATH	"/tmp/real_part/data/yolov3_tiny_ir.rknn"

#ifdef ZM_ENC_RKNN
#define USER_TV_MODEL_PATH "/mnt/sdcard/user_tv_d.rknn"
#define USER_IR_MODEL_PATH "/userdata/user_ir_d.rknn"
#else
#define USER_TV_MODEL_PATH "/mnt/sdcard/user_tv.rknn"
#define USER_IR_MODEL_PATH "/userdata/user_ir.rknn"
#endif


#define USER_TV_CONF_PATH "/mnt/sdcard/user_tv.txt"
#define USER_IR_CONF_PATH "/userdata/user_ir.txt"


#define MEDIA_LIVE 1
#ifdef __cplusplus
}
#endif

#endif 
