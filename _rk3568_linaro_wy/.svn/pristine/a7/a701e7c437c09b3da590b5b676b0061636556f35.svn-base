/************************************************************************************
 * @file		main.cpp
 * @brief	    Main functions: Device initialization and thread management.
 * @author 	    WuAng
 * @version 	1.0
 * @date 	    2023/07/26
 *************************************************************************************
 *   Copyright (C) 2020 by Zmvision Technology Co.,Ltd.                              *
 *                      All Rights Reserved                                          *
 *   Property of  Zmvision Technology Co.,Ltd.. Restricted rights to use, duplicate  *
 *   or disclose this code are granted through contract.                             *
 *************************************************************************************/

#include "mpp_decode.h"
#include "mpp_encode.h"
#include "types_def.h"
#include "video_common.h"
#include "rtsp_client.h"
#include "rtsp_server.h"
#include "data_engine.h"
#include "xutils.h"
#include "log/log.h"
#include "config_handler.h"
#include "data_engine.h"

#include "dlfcn.h"
#include <sched.h>
#include "zm_ts.h"
#include "socket_can_ops.h"
#include "track.h"
#include "Tsmux.h"

#include "sys_inc.h"
#include "rtsp_srv.h"
#include "rtmp_pusher.h"
#include "logger.h"

#include "guideusb2livestream.h"
#include "capture_thread.h"

TRACK_ALG_STATE_T track_state_mechine = {0};
RECTH ecobbox_ctrl = {0};
/**跟踪坐标结果**/
RECTH track_recth_result = {0};

int pthread_exit_control = 1;

void *pso1;

zm_ts_udp_info_t gs_ts_udp_info = {0};
zm_rtsp_info_t gs_rtsp_info = {0};

#define CHIP_ID_PATH "/proc/uinfo/serial"
#define HW_VER_PATH "/tmp/hw_ver"

static void sig_handler(int sig)
{
    printf("%s, sig=%d\r\n", __FUNCTION__, sig);
    // mipi_camera_uninit();
    // rtsp_server_deinit();
    // decode_frame_deinit();
    data_stream_deinit();
    encode_frame_deinit();
    // rtsp_client_deinit();
    dlclose(pso1);
    // control_comm_destory();
    pthread_exit_control = 0;
    logger_deinit();
    // rk_watchdog_deinit();
    exit(0);
}

void load_json_setting_file(void)
{
    char value[512] = {0};
    char cmd[128] = {0};
    int ret = -1;
    // zmv_config_file_load(SYSTEM_SETTING_DEFAULT_JSON_FILE, LOAD_INIT_CONFIG);
    //    zmv_config_file_load(SYSTEM_SETTING_CUSTOM_JSON_FILE, LOAD_CUSTOM_CONFIG);

    //   zmv_config_get_value_string(e_device_softe_version,Selfcheck_info.soft_ver,sizeof(Selfcheck_info.soft_ver));
    // logger_info("load","zpk version %s\n",Selfcheck_info.soft_ver);

    // FILE *chipid_fp = NULL;
    // chipid_fp = fopen(CHIP_ID_PATH,"r");
    // if(!chipid_fp)
    // {
    //     logger_info("load","find chipid err!\n");
    // }
    // else
    // {
    //     fread(Selfcheck_info.chipid, 16, 1, chipid_fp);

    //     fclose(chipid_fp);
    // }

    // FILE *hw_ver_fp = NULL;
    // hw_ver_fp = fopen(HW_VER_PATH,"r");
    // if(!hw_ver_fp)
    // {
    //     logger_info("load","find hw_ver_fp err!\n");
    // }
    // else
    // {
    //     fread(Selfcheck_info.hw_ver, 15, 1, hw_ver_fp);

    //     fclose(hw_ver_fp);
    // }

    // SLOGI("chipid_: %s\n", Selfcheck_info.chipid);

    // #ifdef ZM_MP5_PL
    //     zmv_config_get_value_string(e_device_vis_model_version,value,sizeof(value));
    //     logger_info("load","ZM VIS model version %s\n",value);

    //     zmv_config_get_value_string(e_device_irf_model_version,value,sizeof(value));
    //     logger_info("load","ZM IRF model version %s\n",value);

    //     zmv_config_get_value_string(e_device_ir_version,value,sizeof(value));
    //     logger_info("load","IR version %s\n",value);
    // #endif
    // zmv_config_get_value_string(e_device_input_type,value,sizeof(value));
    // if(strcmp(value,"HDMI") == 0)
    // {
    // 	gs_vedio_info.video_save_type = TYPE_MP4;
    //     sprintf(cmd,"/tmp/real_part/bin/ls_mp4.sh");
    //     logger_info("load","cmd %s\n",cmd);
    //     system(cmd);
    // 	logger_info("load","save type mp4\n");
    // }
    // else if(strcmp(value,"TS") == 0)
    // {
    // 	gs_vedio_info.video_save_type = TYPE_TS;
    //     sprintf(cmd,"/tmp/real_part/bin/ls_ts.sh");
    //     logger_info("load","cmd %s\n",cmd);
    //     system(cmd);
    // 	logger_info("load","save type ts\n");
    // }

    // /*************输入坂数设置**************/
    // switch(input_type)
    // {
    // if(gs_vedio_info.input_type ==TYPE_RTSP){
    // printf("111\n");
    // 	zmv_config_get_value_string(e_rtsp_client_server_ip,gs_rtsp_info.rtsp_client_ip,sizeof(gs_rtsp_info.rtsp_client_ip));

    gs_rtsp_info.rtsp_client_port = 554; // zmv_config_get_value_int(e_rtsp_client_port);

    // zmv_config_get_value_string(e_rtsp_client_name,gs_rtsp_info.rtsp_client_stream_name,sizeof(gs_rtsp_info.rtsp_client_stream_name));

    // zmv_config_get_value_string(e_rtsp_client_user_name,gs_rtsp_info.rtsp_client_username,sizeof(gs_rtsp_info.rtsp_client_username));

    // zmv_config_get_value_string(e_rtsp_client_password,gs_rtsp_info.rtsp_client_password,sizeof(gs_rtsp_info.rtsp_client_password));

    // zmv_config_get_value_string(e_rtsp_client_protocol,value,sizeof(value));

    //SLOGI(" rtsp://admin:%s@%s:%d/%s \n",gs_rtsp_info.rtsp_client_password,gs_rtsp_info.rtsp_client_ip,\
            //gs_rtsp_info.rtsp_client_port,gs_rtsp_info.rtsp_client_stream_name);
    //}

    // gs_rtsp_info.tcp_ctl_port = zmv_config_get_value_int(e_device_tcp_ctl_port);
    // logger_info("load","udp server port = %d\n", gs_rtsp_info.tcp_ctl_port);

    // memset(value,0,sizeof(value));
    // zmv_config_get_value_string(e_ts_send_state,value,sizeof(value));

    // if(strcmp(value,"open") == 0)
    // {
    // 	gs_ts_udp_info.open_udpts_flag = 1;
    // }else{
    //     gs_ts_udp_info.open_udpts_flag = 0;
    // }
    // if(Selfcheck_info.ip_bak!=0xff){
    //     zmv_config_get_value_string(e_device_eth1,gs_ts_udp_info.udpts_ip,sizeof(gs_ts_udp_info.udpts_ip));
    // }else{
    //     sprintf(gs_ts_udp_info.udpts_ip,"192.168.2.117");
    //     zmv_config_set_value(e_device_eth1,gs_ts_udp_info.udpts_ip);
    // }
    // ret=zm_is_ipv4_addr(gs_ts_udp_info.udpts_ip);
    // if(ret!=0){
    //     sprintf(gs_ts_udp_info.udpts_ip,"192.168.2.117");
    // }
    //  #if defined ZM_MP5_PL || defined ZM_MP5_XT
    //     	//zmv_config_get_value_string(e_device_eth1,gs_ts_udp_info.udpts_ip,sizeof(gs_ts_udp_info.udpts_ip));

    // // #if defined ZM_MP5_PL
    // //         gs_ts_udp_info.ts_port = zmv_config_get_value_int(e_device_ts_port);
    // //         if(gs_ts_udp_info.ts_port <= 1023 || gs_ts_udp_info.ts_port > 65535)
    // //         {
    // //             gs_ts_udp_info.ts_port = 55012;
    // //         }
    // // #endif

    //         // gs_ts_udp_info.udpts_port = gs_rtsp_info.tcp_ctl_port-1; //tcp port -1 use as udp port
    //         // logger_info("load","udp client link port = %d\n", gs_rtsp_info.tcp_ctl_port-1);
    //         // Selfcheck_info.udp_port=gs_rtsp_info.tcp_ctl_port;
    //  #else
    //    	    //zmv_config_get_value_string(e_device_eth1,gs_ts_udp_info.udpts_ip,sizeof(gs_ts_udp_info.udpts_ip));
    //         gs_ts_udp_info.udpts_port = gs_rtsp_info.tcp_ctl_port;  //tcp port use as udp port
    //         logger_info("load","udp clent link port = %d\n", gs_rtsp_info.tcp_ctl_port);
    //  #endif
    //     	logger_info("load","udp tsSend %s  udp://@%s:%d\n",value,gs_ts_udp_info.udpts_ip,gs_ts_udp_info.udpts_port);

    /****Rtsp Server设置****/
    memset(value, 0, sizeof(value));
    // zmv_config_get_value_string(e_rtsp_server_state,value,sizeof(value));
    // if(strcmp(value,"open") == 0)
    // {
    gs_rtsp_info.rtsp_server_open_flag = 1;

    gs_rtsp_info.rtsp_server_port = 554;

    memset(value, 0, sizeof(value));
    // }

    /******编码码率******/
    // gs_rtsp_info.venc_bitrate = zmv_config_get_value_int(e_device_bitrate);
    // if(gs_rtsp_info.venc_bitrate <= 500){
    //     gs_rtsp_info.venc_bitrate=500;
    // }
    // if(gs_rtsp_info.venc_bitrate >= 6 * 1024 )
    // {
    // 	gs_rtsp_info.venc_bitrate = 6 * 1024;
    // }

    // logger_info("load","final stream bitrate = %d\n",gs_rtsp_info.venc_bitrate);

    /**识别类型配置**/

    // #ifdef ZM_MP5_PL
    //     zmv_config_get_value_string(e_device_rknn_class,value,sizeof(value));
    //     if(strcmp(value, "0") == 0)
    //     {
    //         Selfcheck_info.rknn_class = RKNN_CLASS_CAR;
    //     }
    //     else if (strcmp(value, "1") == 0)
    //     {
    //         Selfcheck_info.rknn_class = RKNN_CLASS_PERSON;
    //     }
    //     else
    //     {
    //         Selfcheck_info.rknn_class = RKNN_CLASS_BOTH;
    //     }
    // #endif

    // Selfcheck_info.webport = zmv_config_get_value_int(e_ts_send_port);

    /****网络设置****/
    memset(Selfcheck_info.rk_eth0_ip, 0, sizeof(Selfcheck_info.rk_eth0_ip));
    // if(Selfcheck_info.ip_bak!=0xff){
    //     zmv_config_get_value_string(e_device_eth0,Selfcheck_info.rk_eth0_ip,sizeof(Selfcheck_info.rk_eth0_ip));
    // }else{
    sprintf(Selfcheck_info.rk_eth0_ip, "192.168.2.119");
    // zmv_config_set_value(e_device_eth0, Selfcheck_info.rk_eth0_ip);
    //  if(zmv_config_file_generate(SYSTEM_SETTING_TMP_CUSTOM_JSON_FILE) == 0)
    //  {
    //      memset(cmd,0,sizeof(cmd));
    //      sprintf(cmd,"cp %s %s;sync",SYSTEM_SETTING_TMP_CUSTOM_JSON_FILE,SYSTEM_SETTING_CUSTOM_JSON_FILE);
    //      system(cmd);
    //  }
    //   //  }
    //     ret=zm_is_ipv4_addr(Selfcheck_info.rk_eth0_ip);
    //     if(ret!=0){
    //         sprintf(Selfcheck_info.rk_eth0_ip,"192.168.2.119");
    //     }

    //     zmv_config_get_value_string(e_rtsp_server_stream_type,value,sizeof(value));

    // #if defined ZM_MP5_PL || defined ZM_MP5_XT
    //  if((Selfcheck_info.webport != 2000) && (Selfcheck_info.webport != gs_rtsp_info.tcp_ctl_port) \
    //  && (Selfcheck_info.webport != (gs_rtsp_info.tcp_ctl_port-1))){
    //     sprintf(cmd,"sed -i '5c Listen :%d' /tmp/appweb.conf",Selfcheck_info.webport);
    //     system(cmd);
    //     logger_info("load", "web port %d", Selfcheck_info.webport);
    // }

    // if(strcmp(value,"h.264") == 0)
    // {
    gs_vedio_info.output_type = TYPE_H264;
    //   SLOGI("output_type: h264");
    //  }
    // else
    // {
    //     gs_vedio_info.output_type=TYPE_H265;
    //     SLOGI("output_type: h265");
    // }

    // gs_vedio_info.output_type=TYPE_H264;

    // #ifdef H265_DEBUG
    //     gs_vedio_info.output_type=TYPE_H265;
    //     SLOGI("output_type: h265");
    // #endif

    // #ifdef ZM_MP5_PL
    //  zmv_config_get_value_string(e_ts_send_resolution, value, sizeof(value));
    //  if (strcmp(value, "1080P") == 0)
    //  {
    gs_vedio_info.encode_1080p = 1;
    gs_rtsp_info.rtsp_server_w = 1920;
    gs_rtsp_info.rtsp_server_h = 1080;
    //}
    // else
    // {
    //     gs_vedio_info.encode_1080p= 0;
    //     gs_rtsp_info.rtsp_server_w=1280;
    //     gs_rtsp_info.rtsp_server_h=720;
    // }
    // #endif

    // #ifdef ZM_MP5_XT
    //     gs_vedio_info.encode_1080p= 1;
    //     gs_rtsp_info.rtsp_server_w=1920;
    //     gs_rtsp_info.rtsp_server_h=1080;
    // #endif

    // sprintf(cmd,"ifconfig eth0 %s up",Selfcheck_info.rk_eth0_ip);
    // //sprintf(cmd,"ifconfig eth0 192.168.144.119 up");
    // logger_info("load","cmd %s\n",cmd);
    // system(cmd);
    // sleep(1);

    // sprintf(cmd,"ping %s -c 1",gs_ts_udp_info.udpts_ip);
    // logger_info("load","cmd %s\n",cmd);
    // system(cmd);
    // sleep(1);

    // #endif

    //    #ifdef ZM_MP5_KBT
    //     sprintf(cmd,"ifconfig eth0 192.168.169.2 up"); //eth0 not use
    //     logger_info("load","cmd %s\n",cmd);
    //     system(cmd);
    //     sleep(1);

    //     sprintf(cmd,"ifconfig eth1 %s up",Selfcheck_info.rk_eth0_ip);
    //     logger_info("load","cmd %s\n",cmd);
    //     system(cmd);
    //     sleep(1);

    // int seg1,seg2,seg3,seg4;
    // sscanf(Selfcheck_info.rk_eth0_ip, "%d.%d.%d.%d", &seg1, &seg2,&seg3,&seg4);
    // logger_info("load","eth0 %d.%d.%d.%d",seg1, seg2,seg3,seg4);

    // sprintf(cmd,"route del -net %d.%d.%d.0 netmask 255.255.255.0",seg1,seg2,seg3);
    // printf("cmd %s\n",cmd);
    // system(cmd);

    // sprintf(cmd,"route add -host %s dev eth0",gs_rtsp_info.rtsp_client_ip);

    // printf("cmd %s\n",cmd);
    // system(cmd);

    // memset(Selfcheck_info.rk_eth0_ip,0,sizeof(Selfcheck_info.rk_eth0_ip));
    // zmv_config_get_value_string(e_device_eth1,Selfcheck_info.rk_eth0_ip,sizeof(Selfcheck_info.rk_eth0_ip));

    // sprintf(cmd,"ifconfig eth1 %s up",Selfcheck_info.rk_eth0_ip);
    // logger_info("load","cmd %s\n",cmd);
    // system(cmd);
    // sleep(1);
    // sscanf(Selfcheck_info.rk_eth0_ip, "%d.%d.%d.%d", &seg1, &seg2,&seg3,&seg4);
    // logger_info("load","eth1 %d.%d.%d.%d",seg1, seg2,seg3,seg4);

    // sprintf(cmd,"route del -net %d.%d.%d.0 netmask 255.255.255.0",seg1,seg2,seg3);
    // logger_info("load","cmd %s\n",cmd);
    // system(cmd);spi_can_init();

    // sprintf(cmd,"route add -net %d.%d.%d.0 netmask 255.255.255.0 dev eth1",seg1,seg2,seg3);
    // logger_info("load","cmd %s\n",cmd);
    // system(cmd);

    // sprintf(cmd,"ping %s -c 1",gs_rtsp_info.rtsp_client_ip);
    // logger_info("load","cmd %s\n",cmd);
    // system(cmd);
    // sleep(1);

    /**********************设置??????CAN id**************************************/
    // can_upward_id = zmv_config_get_value_int(e_ts_send_port);
    // logger_info("load","can_upward_id = 0x%x\n",can_upward_id);

    // track_state_mechine.delay_count = zmv_config_get_value_int(e_device_track_frame_delay);
    // if(track_state_mechine.delay_count>9 || track_state_mechine.delay_count<1){
    //     track_state_mechine.delay_count=1;
    // }
    // #endif
    //  #if defined ZM_MP5_PL || defined ZM_MP5_XT
    // track_state_mechine.delay_count = zmv_config_get_value_int(e_device_track_frame_delay);
    // if(track_state_mechine.delay_count>39 ){
    //     track_state_mechine.delay_count=39;
    // }
    // if(track_state_mechine.delay_count<22 ){
    //     track_state_mechine.delay_count=22;
    // }
    // #endif
    // logger_info("load","init track_state_mechine.delay_count = %d\n",track_state_mechine.delay_count);

    // gs_vedio_info.hdmi_vo_fps = zmv_config_get_value_int(e_device_fps);
    // logger_info("load","device_fps = %d\n",gs_vedio_info.hdmi_vo_fps);
    // gs_vedio_info.vo_fps= 30;
    // gs_vedio_info.vi_fps= 30;//gs_vedio_info.hdmi_vo_fps;

    // #ifdef ZM_MP5_PL

    //     memset(value,0,sizeof(value));
    //     zmv_config_get_value_string(e_osd_srt_set,value,sizeof(value));
    //     if(strcmp(value,"open") == 0)
    //     {
    //         srt_open_flag = 1;
    //     }
    //     else if(strcmp(value,"close") == 0)
    //     {
    //         srt_open_flag = 0;
    //     }
    //     else
    //     {
    //         srt_open_flag = 1;
    //     }

    //     logger_info("load","srt flag: = %d\n",srt_open_flag);

    // #endif

    memset(value, 0, sizeof(value));

    // zmv_config_get_value_string(e_ts_rtsp_image,value,sizeof(value));

    // /**
    //  * TODO: 这里可以采用别的方式实现
    //  */
    // if(strcmp(value, "default") == 0)
    // {
    //     gs_vedio_info.low_rtsp_fps = 0;
    //     gs_vedio_info.minimum_quality = 0;
    // }
    // else if(strcmp(value, "real_time") == 0)
    // {
    //     gs_vedio_info.low_rtsp_fps = 0;
    //     gs_vedio_info.minimum_quality = 1;
    // }
    // // deprecated minimum_quality
    // // else if(strcmp(value, "minimum_quality") == 0)
    // // {
    // //     gs_vedio_info.low_rtsp_fps = 0;
    // //     gs_vedio_info.minimum_quality = 1;
    // // }
    // else if(strcmp(value, "low_rtsp_fps") == 0)
    // {
    //     gs_vedio_info.low_rtsp_fps = 1;
    //     gs_vedio_info.minimum_quality = 1;
    // }
    // else
    // {
    //     logger_info("control","image mode err\n");
    //     gs_vedio_info.low_rtsp_fps = 0;
    //     gs_vedio_info.minimum_quality = 0;
    // }

    // #ifdef DEBUG_MINIMUM_QUALITY
    //     gs_vedio_info.low_rtsp_fps = 1;
    //     gs_vedio_info.minimum_quality = 1;
    // #endif

    // #ifdef ZM_MP5_KBT
    //     gs_vedio_info.vo_fps= 25;
    // #endif

    // #ifdef RTMP_OUTPUT_ENABLE
    //    /**************设置RTMP PUSHER路径*****************/

    //    // zmv_config_get_value_string(e_rtsp_client_user_name,gs_rtsp_info.rtsp_client_username,sizeof(gs_rtsp_info.rtsp_client_username));
    //   //  zmv_config_get_value_string(e_rtsp_client_password,gs_rtsp_info.rtsp_client_password,sizeof(gs_rtsp_info.rtsp_client_password));
    memset(value, 0, sizeof(value));
    // zmv_config_get_value_string(e_rtsp_client_name,value,sizeof(value));
    // if(strncmp(value,"rtmp://",7) == 0)
    // {
    //     strncpy(gs_rtsp_info.rtmp_ip, value, sizeof(value));
    // }
    // else
    // {
    strcpy(gs_rtsp_info.rtmp_ip, "rtmp://172.17.218.22:1935/live");
    // }
    printf("gs_rtsp_info.rtmp_ip = %s\n", gs_rtsp_info.rtmp_ip);

    /**************设置网关路径*****************/
    //  #endif

    // printf("22222.111\n");
    //     zmv_config_get_value_string(e_ts_send_ip,gs_rtsp_info.rtmp_gw,sizeof(gs_rtsp_info.rtmp_gw));
    // 	sprintf(cmd,"route add default gw %s",gs_rtsp_info.rtmp_gw);
    //     //logger_info("load","cmd %s\n",cmd);
    //     system(cmd);
    // printf("33333\n");

#ifdef ZM_MP5_PL
    /*************set net mask**************/
    memset(cmd, 0, sizeof(cmd));
    // zmv_config_get_value_string(e_mask_ip,gs_rtsp_info.mask_ip,sizeof(gs_rtsp_info.mask_ip));
    sprintf(cmd, "ifconfig eth0 netmask %s", gs_rtsp_info.mask_ip);
    //  logger_info("load","cmd %s\n",cmd);
    system(cmd);

    /*************set eth rate**************/
#if 0
    // char eth_rate[32] = {0};

    // memset(cmd,0,sizeof(cmd));
    // zmv_config_get_value_string(e_eth_rate,eth_rate,sizeof(eth_rate));
    // if(strcmp(eth_rate, "100") && strcmp(eth_rate, "10"))
    // {
    //     SLOGI("err: eth rate err %s.\n", eth_rate);
    //     memset(eth_rate, 0, sizeof(eth_rate));
    //     strcpy(eth_rate, "100");
    // }

	// sprintf(cmd,"/tmp/real_part/data/ethtool -s eth0 duplex full autoneg off speed %s",eth_rate);
    // logger_info("load","cmd %s\n",cmd);
    // system(cmd);
#endif

#endif

    return;
}

#pragma message("Compiling " __DATE__ ", " __TIME__)

sem_t gs_sem_mpp;

void *read_video_func(void *)
{
    unsigned int cccsize = 0;
    static unsigned int data_number = 0;
    void *gpu_yuv_buf = NULL;
    data_stream_t *data_stream = data_stream_get();
    image_frame_info_t frame_info = {0};

    FILE *fp;

    int number = 0;

    int banle = 0;

    int zhengbanle = 0;

    int cfp = open("/mnt/WUANG/out_qqh_nv12.yuv", O_RDONLY);
    // int cfp = open("/media/usb0/out_qqh_nv12.yuv", O_RDONLY);
    if (cfp <= 0)
    {
        printf(" open error main \n");
    }
    printf("-------------------------open s\n");

    int nr_size = 0;

    cccsize = 0;

    while (1)
    {
        if (data_number >= 1920 * 1080 * 3 / 2 * 20)
        {
            data_number = 0;
        }

        nr_size = read(cfp, data_stream->yuv_data + data_number, 1920 * 1080 * 3 / 2); // 1920*1080*3/2);
        if (nr_size <= 0)
        {
            printf("-------------------------file read over\n");
            break;
        }

        cccsize += nr_size;

        if (cccsize >= 1920 * 1080 * 3 / 2)
        {
            frame_info.frame_offset = data_number;
            yuv_encode_data_to_queue(data_stream, &frame_info);

            data_number += 1920 * 1080 * 3 / 2;

            cccsize -= 1920 * 1080 * 3 / 2;
        }

        usleep(30000); // 33毫秒读取一次
    }

    // fclose(fp);
    close(cfp);
    return NULL;
}

int main(int argc, char *argv[])
{
    int ret = RET_OK;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sig_handler);
    signal(SIGKILL, sig_handler);
    signal(SIGTERM, sig_handler);

    logger_init();

    // logger_info("main","bulid project time %s %s\n", __TIME__, __DATE__);

    ret = data_stream_init();
    printf("main -------data_stream_init %d\n", ret);
    if (ret != RET_OK)
    {
        return ret;
    }

    load_json_setting_file();

#if 1
#ifdef RTSP_OUTPUT_ENABLE
    if (1)
    {
        // ret = rtsp_server_init(RTSP_SERVER_PORT,gs_rtsp_info.rtsp_server_w,gs_rtsp_info.rtsp_server_h);
        ret = run_rtsp_srv();
        sleep(1);
        printf("main", "----------rtsp_server_init----------ret %d\n", ret);
        // if (ret != RET_OK) {
        //     return ret;
        // }
    }
#endif

    ret = encode_frame_init();
    printf("------encode_frame_init------ret %d\n", ret);
    if (ret != RET_OK)
    {
        return ret;
    }
#endif

    pthread_t read_file;
    pthread_create(&read_file, NULL, capture_thread, NULL);
    // pthread_create(&read_file, NULL, read_video_func, NULL);

    printf("begin rtsp init \n");

#if 0
    ret = decode_frame_init();
    logger_info("main","----------decode_frame_init----------ret %d\n",ret);
    if (ret != RET_OK) {
        return ret;
     logger_info("main","----------decode_frame_init err----------ret %d\n",ret);
    }
#endif

    // RTSP Client
    //  if(gs_vedio_info.input_type==TYPE_HDMI)
    //{

    // if (ret != RET_OK) {
    //     //return ret;
    // }
    //}
    // else if(gs_vedio_info.input_type==TYPE_RTSP)
    //{
    /*
    ret = rtsp_client_init();
        logger_info("load","----------rtsp_client_init----------ret %d\n",ret);
    char cmd[128] = {0};
    sprintf(cmd,"ping %s -c 1",gs_ts_udp_info.udpts_ip);
        logger_info("load","cmd %s\n",cmd);
    system(cmd);
    // if (ret != RET_OK) {
    //    // return ret;
    // }
    */
    //}

    // char segment_ip[64]={0};
    // char ping_ip[64] = {0};
    // char *gw_p[5];
    // char *in_ptr = NULL;
    // char *out_ptr = NULL;
    // int in = 0;

    // strcpy(segment_ip,Selfcheck_info.rk_eth0_ip);

    // in_ptr = segment_ip;

    // while((gw_p[in] = strtok_r(in_ptr,".",&out_ptr)) != NULL)
    // {
    //     //SLOGI("p[%d]: %s \n", in,gw_p[in]);
    //     in_ptr = NULL;
    //     in++;
    // }

    // sprintf(ping_ip,"%s.%s.%s",gw_p[0],gw_p[1],gw_p[2]);

    // SLOGI("ping %s...\n",ping_ip);
    // logger_info("main","ping %s...\n",ping_ip);

    // for(int i = 1; i < 255; i++)
    // {
    // sprintf(cmd,"timeout 0.01 ping %s.%d -c 1",ping_ip ,i);
    // system(cmd);
    // }

    // memset(cmd,0,sizeof(cmd));
    // sprintf(cmd,"touch /tmp/net_ok");
    // logger_info("main","cmd %s\n",cmd);
    // system(cmd);

#ifdef RTMP_OUTPUT_ENABLE
    ret = run_rtmp_pusher();
    SLOGI("----------run_rtmp_pusher----------ret %d\n", ret);
#endif

    while (1)
    {

        sleep(5);
    }

    return 0;
}
