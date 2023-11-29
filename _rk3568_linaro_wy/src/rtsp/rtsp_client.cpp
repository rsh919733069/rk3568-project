/*
 * File      : rtsp_client.c
 * This file is RTSP Client implementation file  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-08     fengyong    first version
 */

#if 0

#include "types_def.h"
#include "rtsp_client.h"

#include "sys_inc.h"
#include "rtsp_cln.h"
#include "hqueue.h"
#ifdef OVER_HTTP
#include "http.h"
#include "http_parse.h"
#endif
#include "media_format.h"
#include "h264_rtp_rx.h"

#include "log/log.h"

enum rtsp_client_e {
    RTSP_CLIENT_CLN_NUM = 1,
    RTSP_CLIENT_SYSTEM_BUFFER_NUM = 32,
    RTSP_CLIENT_PARSE_BUFFER_NUM = 100,			
};

enum rtsp_client_arg_e {   
    RTSP_CLIENT_USER_NAME_LEN = 64,
    RTSP_CLIENT_URL_PATH_LEN = 256,	
};

enum rtsp_client_arg1_e {   
    RTSP_CLIENT_IMAGE_FRAME_NUM = 4,    
};

enum rtsp_client_arg2_e {   
    RTSP_CLIENT_EVENT_QUEUE_NUM = 10,    
};

typedef struct {
	int 		  event;
	CRtspClient  *rtsp;
} EVENT_PARAMS;

typedef struct {        
    HQUEUE     *queue;;
    int		   flag;
    pthread_t  tid;

    CRtspClient rtsp[RTSP_CLIENT_CLN_NUM];
    uint32_t frame_count; 

    uint8 *image_buf;

    char url_path[RTSP_CLIENT_URL_PATH_LEN];
    char user_name[RTSP_CLIENT_USER_NAME_LEN];
    char user_password[RTSP_CLIENT_USER_NAME_LEN];       
} rtsp_client_t;

static rtsp_client_t *gs_rtsp_client = NULL;

rtsp_client_t *rtsp_client_get(void)
{
    return gs_rtsp_client;
}

#ifdef __cplusplus
extern "C" {
#endif

void rtsp_server_push_video(int ch_index,uint8_t *frame_buf, int frame_size);
//int decode_frame_handle(uint8_t *buf_data, int size, uint32_t ts);
void decode_packet_input(uint8_t *buf_data, int size, uint32_t ts);
#ifdef __cplusplus
}
#endif

#if 0
/**
 * @desc : rtsp notify callback
 *
 * @params :
 *  event : event type
 *  puser : user parameter
 */
static int rtsp_notify_cb(int event, void * puser)
{
    printf("%s, event = %d\r\n", __FUNCTION__, event);

    CRtspClient * rtsp = (CRtspClient *) puser;
    rtsp_client_t *rtsp_client = rtsp_client_get();
    
    if (RTSP_EVE_CONNSUCC == event)
    {
        int vcodec = rtsp->video_codec();
        if (vcodec != VIDEO_CODEC_NONE)
        {
            char codec_str[20] = {'\0'};
            
            switch (vcodec)
            {
            case VIDEO_CODEC_H264:
                strcpy(codec_str, "H264");
                break;

            case VIDEO_CODEC_H265:
                strcpy(codec_str, "H265");
                break;

            case VIDEO_CODEC_MP4:
                strcpy(codec_str, "MP4");
                break;

            case VIDEO_CODEC_JPEG:
                strcpy(codec_str, "JPEG");
                break;    
            }

            printf("video codec is %s\r\n", codec_str);
        }

        int acodec = rtsp->audio_codec();
        if (acodec != AUDIO_CODEC_NONE)
        {
            char codec_str[20] = {'\0'};
            
            switch (acodec)
            {
            case AUDIO_CODEC_G711A:
                strcpy(codec_str, "G711A");
                break;

            case AUDIO_CODEC_G711U:
                strcpy(codec_str, "G711U");
                break;

            case AUDIO_CODEC_G722:
                strcpy(codec_str, "G722");
                break;

            case AUDIO_CODEC_G726:
                strcpy(codec_str, "G726");
                break;

            case AUDIO_CODEC_OPUS:
                strcpy(codec_str, "OPUS");
                break;

            case AUDIO_CODEC_AAC:
                strcpy(codec_str, "AAC");
                break;    
            }

            printf("audio codec is %s\r\n", codec_str);
            printf("audio sample rate is %d\r\n", rtsp->get_audio_samplerate());
            printf("audio channels is %d\r\n", rtsp->get_audio_channels());
        }
    }

	EVENT_PARAMS params;

	params.event = event;
	params.rtsp = rtsp;

    if (!hqBufPut(rtsp_client->queue, (char *) &params))
    {
    	printf("hqBufPut failed\r\n");
    }
    
    return 0;
}

/**
 * @desc : rtsp audio data callback
 *
 * @params :
 *  pdata : audio data buffer
 *  len : audio data buffer length
 *  ts : timestamp
 *  seq : sequential
 *  puser : user parameter
 */
static int rtsp_audio_cb(uint8 * pdata, int len, uint32 ts, uint16 seq, void * puser)
{
    CRtspClient * rtsp = (CRtspClient *) puser;
    
    printf("%s, len = %d, ts = %u, seq = %d\r\n", __FUNCTION__, len, ts, seq);
    
    return 0;
}
#endif 

/**
 * @desc : rtsp video data callback
 *
 * @params :
 *  pdata : video data buffer
 *  len : video data buffer length
 *  ts : timestamp
 *  seq : sequential
 *  puser : user parameter
 */
static int rtsp_video_cb(uint8 * pdata, int len, uint32 ts, uint16 seq, void * puser)
{
    CRtspClient * rtsp = (CRtspClient *) puser;
    
    //printf("%s, len = %d, ts = %u, seq = %d\r\n", __FUNCTION__, len, ts, seq);

    //SLOGI("%s, len = %d, ts = %u, seq = %d\r\n", __FUNCTION__, len, ts, seq);
    //rtsp_server_push_video(0,pdata,len);
#if 1  
    if (seq > 0) {
        //decode_frame_handle(pdata,len,ts);
        decode_packet_input(pdata,len,ts);
    }
#endif     
    
    return 0;
}
volatile int my_rtsp_stat=0;
static int  rtsp_notify_handler(int arg,void * argc)
{
    my_rtsp_stat=arg;
    //SLOGI("-------------- rtsp_notify_handler  %d \n",arg); 
	return 0;
}

static void rtsp_reconn(CRtspClient * p_rtsp)
{
    int i=0;
    rtsp_client_t *rtsp_client = rtsp_client_get();
	  // create event handler thread
    //rtsp_client->flag = 0;

    //rtsp_client->rtsp[0].rtsp_stop();
    rtsp_client->rtsp[0].rtsp_stop();
    memset(rtsp_client, 0,sizeof(*rtsp_client));
    rtsp_client->rtsp[0].set_notify_cb(rtsp_notify_handler, &rtsp_client->rtsp[0]);
        //rtsp_client->rtsp[i].set_audio_cb(rtsp_audio_cb);
    rtsp_client->rtsp[0].set_video_cb(rtsp_video_cb);
    //rtsp_client->rtsp[0].rtsp_close();
    rtsp_client->rtsp[0].set_rx_timeout(10);        // No data within 10s, receiving timeout          
    char filepath[1024] ="rtsp://172.17.254.40/chn0"; 
    char p_user[1024] = "";
    char p_pass[1024] = "";     
    if(strlen(gs_rtsp_info.rtsp_client_ip) && strlen(gs_rtsp_info.rtsp_client_stream_name)){
        sprintf(filepath,"rtsp://%s/%s",gs_rtsp_info.rtsp_client_ip,gs_rtsp_info.rtsp_client_stream_name);
        printf("+++rtsp filepath %s \r\n",filepath);
    }
    if(strlen(gs_rtsp_info.rtsp_client_username)>0 && strlen(gs_rtsp_info.rtsp_client_username) <512){
        sprintf(p_user,"%s",gs_rtsp_info.rtsp_client_username);
        printf("+++rtsp p_user %s \r\n",p_user);
    }
        if(strlen(gs_rtsp_info.rtsp_client_password)  && strlen(gs_rtsp_info.rtsp_client_password) <512 ){
        sprintf(p_pass,"%s",gs_rtsp_info.rtsp_client_password);
        printf("+++rtsp p_pass %s \r\n",p_pass);
    }
    rtsp_client->flag = 1;
    BOOL ret = rtsp_client->rtsp[0].rtsp_start(filepath, p_user, p_pass);
    printf("1111111111+++rtsp %d %s start ret = %d\r\n", i,filepath,ret);
}

void *  rtsp_notify_handler_my(void *arg)
{
	//EVENT_PARAMS params;
    char cmd[128] = {0};
    int err_cnt=0;
    //sprintf(cmd,"reboot");
    sprintf(cmd,"/tmp/real_part/start_app.sh");
    volatile int my_rtsp_stat_old=RTSP_EVE_CONNSUCC;
    rtsp_client_init();
    rtsp_client_t *rtsp_client = rtsp_client_get();
    while(1){
        if(my_rtsp_stat == RTSP_EVE_NODATA ||\
           my_rtsp_stat == RTSP_EVE_CONNFAIL){
            if(my_rtsp_stat==my_rtsp_stat_old){
                // SLOGI("-------------- rtsp_notify_handler_my  %d \n",my_rtsp_stat); 
                //rtsp_reconn(&rtsp_client->rtsp[0]);
                //sleep(5);
                //rtsp_client_init();
                err_cnt++;
                if(err_cnt>6){
                    rtsp_reconn(&rtsp_client->rtsp[0]);
                    sleep(2);
                }
            }       
        }else if(my_rtsp_stat == RTSP_EVE_STOPPED){
            if(my_rtsp_stat_old==my_rtsp_stat){
                printf("cmd %s\n",cmd);
                //system(cmd);
            }
        }else{
            err_cnt=0;
        } 
        my_rtsp_stat_old=my_rtsp_stat;
        sleep(10); 
    }  
   // SLOGI("-------------- rtsp_notify_handler_my  %d \n",arg); 
}
int rtsp_client_init(void)
{
    int i = 0;
    rtsp_client_t *rtsp_client = NULL;

    rtsp_client = (rtsp_client_t *)malloc(sizeof(*rtsp_client));
    if (!rtsp_client) {
        printf("XF:malloc rtsp client failed\n");
        return RET_ERR_MALLOC;
    }

    memset(rtsp_client, 0,sizeof(*rtsp_client));

    gs_rtsp_client = rtsp_client;

#if 0
	log_init("rtsp_client.txt");
	log_set_level(HT_LOG_ERR);

	network_init();
#endif 

    // allocate system BUFFER and rtsp parser BUFFER
	sys_buf_init(RTSP_CLIENT_SYSTEM_BUFFER_NUM);
	rtsp_parse_buf_init(RTSP_CLIENT_PARSE_BUFFER_NUM);

#ifdef OVER_HTTP
    // allocate http message parser BUFFER
    http_msg_buf_init(100);
#endif

    // create event queue
	// rtsp_client->queue = hqCreate(RTSP_CLIENT_EVENT_QUEUE_NUM, sizeof(EVENT_PARAMS), HQ_GET_WAIT | HQ_PUT_WAIT);
	// if (NULL == rtsp_client->queue) {
	// 	printf("create queue failed\r\n");
	// 	return RET_ERR_FAILED;
	// }
    	
    // create event handler thread
    rtsp_client->flag = 1;
    
   
    //rtsp_client->tid = sys_os_create_thread((void *)rtsp_notify_handler, NULL);
    // if (rtsp_client->tid <= 0) {
    //     printf("create rtsp notify handler thread failed\r\n");
    //     return RET_ERR_FAILED;
    // }
    
    for (i = 0; i < RTSP_CLIENT_CLN_NUM; i++) {          
        rtsp_client->rtsp[i].set_notify_cb(rtsp_notify_handler, &rtsp_client->rtsp[i]);
        //rtsp_client->rtsp[i].set_audio_cb(rtsp_audio_cb);
        rtsp_client->rtsp[i].set_video_cb(rtsp_video_cb);
        //rtsp_client->rtsp[i].set_notify_cb(set_notify_cb);           

#ifdef METADATA    
        rtsp[i].set_metadata_cb(rtsp_metadata_cb);
#endif
        // rtsp[i].set_rtp_over_udp(1);    // rtp over udp
    	// rtsp[i].set_rtp_multicast(1);   // force multicast rtp via rtsp
#ifdef BACKCHANNEL
    	// rtsp[i].set_bc_flag(1);         // enable audio backchannel
    	// rtsp[i].set_bc_data_flag(1);    // enable send audio data
#endif
#ifdef REPLAY
        // rtsp[i].set_replay_flag(1);     // enable replay
        // rtsp[i].set_replay_range(time(NULL) - 3600, time(NULL)); // set the replay timestamp range
#endif
#ifdef OVER_HTTP
        // rtsp[i].set_rtsp_over_http(1, 6000); // rtsp over http
#endif
        rtsp_client->rtsp[i].set_rx_timeout(10);        // No data within 10s, receiving timeout          
#if 0  	
    	//char filepath[] ="rtsp://172.17.254.45/chn0";
		char filepath[] ="rtsp://172.17.254.221:554/Streaming/Channels/101";
		char p_user[] = "admin";
		char p_pass[] = "zheng19931213";
#else
#if 1
        char filepath[1024] ="rtsp://172.17.254.40/chn0"; 
        char p_user[1024] = "";
		char p_pass[1024] = "";     
        if(strlen(gs_rtsp_info.rtsp_client_ip) && strlen(gs_rtsp_info.rtsp_client_stream_name)){
           sprintf(filepath,"rtsp://%s/%s",gs_rtsp_info.rtsp_client_ip,gs_rtsp_info.rtsp_client_stream_name);
           printf("+++rtsp filepath %s \r\n",filepath);
        }
        if(strlen(gs_rtsp_info.rtsp_client_username)>0 && strlen(gs_rtsp_info.rtsp_client_username) <512){
            sprintf(p_user,"%s",gs_rtsp_info.rtsp_client_username);
            printf("+++rtsp p_user %s \r\n",p_user);
        }
         if(strlen(gs_rtsp_info.rtsp_client_password)  && strlen(gs_rtsp_info.rtsp_client_password) <512 ){
            sprintf(p_pass,"%s",gs_rtsp_info.rtsp_client_password);
            printf("+++rtsp p_pass %s \r\n",p_pass);
        }
    
        
         //char filepath[] ="rtsp://192.168.1.110/chn0";
		    
#else 
        char filepath[] ="rtsp://192.168.3.233:554/Streaming/Channels/101";
		char p_user[] = "admin";
		char p_pass[] = "fy123456";
#endif
#endif        
    	BOOL ret = rtsp_client->rtsp[i].rtsp_start(filepath, p_user, p_pass);
        printf("1111111111+++rtsp %d %s start ret = %d\r\n", i,filepath,ret);
        //SLOGI("RTSP Client %d start ret=%d\n",i, ret);
    }
    
	return RET_OK;
}

void rtsp_client_deinit(void)
{
    int i = 0;
    rtsp_client_t *rtsp_client = rtsp_client_get();

    for (i = 0; i < RTSP_CLIENT_CLN_NUM; i++) {
	    rtsp_client->rtsp[i].rtsp_close();
	}

    rtsp_client->flag = 0;

	// EVENT_PARAMS params;

	// params.event = -1;
	// params.rtsp = NULL;
	
	// hqBufPut(rtsp_client->queue, (char *) &params);

	// // waiting for event handler thread to exit
	// while (rtsp_client->tid > 0) {
	// 	usleep(10*1000);
	// }

	// hqDelete(rtsp_client->queue);

    // free memory resources
	rtsp_parse_buf_deinit();
	sys_buf_deinit();

    // close log
   // log_close();

    if (rtsp_client) {
        free(rtsp_client);
        rtsp_client = NULL;   
    }
}

#endif