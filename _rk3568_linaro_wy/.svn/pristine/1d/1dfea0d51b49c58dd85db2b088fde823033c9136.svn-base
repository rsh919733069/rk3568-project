/*
 * File      : data_engine.c
 * This file is data engine file  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-1-4     fengyong    first version
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <sched.h>
#include "rk_mpi.h"
#include "mpp_mem.h"
#include "mpp_common.h"
#include "data_engine.h"
#include "video_common.h"
#include "xutils.h"
#include "log/log.h"
#include "track.h"

#include "nnie.h"
#include "rtsp_client.h"

#include "hetero_interface.h"

#include "xutils.h"
#include "GetSysInfo.h"

#include "socket_can_ops.h"
#include "guideusb2livestream.h"
#include "mpp_encode.h"
#include "logger.h"


int dect = 0;
static void  video_for_copy(char *record_name_tmp,char *record_name_real);

data_stream_t *gs_data_stream = NULL;

ir_temp_info_t gs_ir_temp_info={0};

time_info_t gs_time_info={0};
time_info_t gs_rec_time_info={0};
photo_info_t  gs_photo_info={0};
record_info_t gs_record_info={0};
zm_vedio_info_t gs_vedio_info={0};
selfcheck_info_t Selfcheck_info={0};

RKNN_ALG_STATE_T gs_nnie_state= {0};
nnie_res_t g_nnie_res={0};
target_info_st obj_detect_target[100] = {0};

static char clean_osd_buf[VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE*3]={0};
static char vis_nosig_buf[VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE*4]={0};
static char ir_nosig_buf[VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE*4]={0};

pthread_mutex_t mutex_vis_in=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_ir_in=PTHREAD_MUTEX_INITIALIZER;


//static  uint32_t vis_input_frame_count=0;
int rga_copy(XfRockchipRga *rga,int width,int height, unsigned char *src_v_buf,unsigned char *dst_v_buf,int format)
{
    int ret = -EINVAL;

    rga->ops->initCtx(rga);

    rga->ops->setSrcBufferPtr(rga,src_v_buf);
    rga->ops->setDstBufferPtr(rga,dst_v_buf);

    rga->ops->setSrcRectInfo(rga,0,0,width,height,width,height,format);
    rga->ops->setDstRectInfo(rga,0,0,width,height,width,height,format);

    ret = rga->ops->go(rga);

    return ret;
}

int rga_format_convert(XfRockchipRga *rga,int width,int height, 
    unsigned char *src_v_buf,unsigned char *dst_v_buf, int src_format, int dst_format)
{
    int ret = -EINVAL;

    rga->ops->initCtx(rga);

    rga->ops->setSrcBufferPtr(rga,src_v_buf);
    rga->ops->setDstBufferPtr(rga,dst_v_buf);

    rga->ops->setSrcRectInfo(rga,0,0,width,height,width,height,src_format);
    rga->ops->setDstRectInfo(rga,0,0,width,height,width,height,dst_format);

    ret = rga->ops->go(rga);

    return ret;
}

int rga_zoom(XfRockchipRga *rga,int src_width,int src_height,int dst_width,int dst_height, 
    unsigned char *src_v_buf,unsigned char *dst_v_buf, int format)
{
    int ret = -EINVAL;

    rga->ops->initCtx(rga);

    rga->ops->setSrcBufferPtr(rga,src_v_buf);
    rga->ops->setDstBufferPtr(rga,dst_v_buf);

    rga->ops->setSrcRectInfo(rga,0,0,src_width,src_height,src_width,src_height,format);
    rga->ops->setDstRectInfo(rga,0,0,dst_width,dst_height,dst_width,dst_height,format);

    ret = rga->ops->go(rga);

    return ret;
}

static void yuv_out_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info){
	ring_buffer_t *rb=data_stream->yuv_out_rb;
    image_frame_info_t frame_param, tmp_param;
	/* put to image analyzer queue */
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	frame_param.offset = frame_info->offset;
    frame_param.frame_index = frame_info->frame_index;
	frame_param.frame_offset = frame_info->frame_offset;	
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));
}


static void yuv_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->yuv_data_rb;
	image_frame_info_t frame_param, tmp_param;
	
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.offset = frame_info->offset;
    frame_param.frame_index = frame_info->frame_index;
		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->yuv_data_cond);
}
static void rgb_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->rgb_tv_in_data_rb;
	image_frame_info_t event_param, tmp_param;
	
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
    event_param.offset=frame_info->offset;
    event_param.frame_index = frame_info->frame_index;
    		
	ringbuffer_put(rb, (uint8_t *)&event_param, sizeof(event_param));

	//pthread_cond_signal(&data_stream->rgb_tv_data_cond);
}


static void rgb_data_out_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->rgb_out_data_rb;
	image_frame_info_t event_param, tmp_param;
	
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
    event_param.offset=frame_info->offset;
    event_param.frame_index = frame_info->frame_index;
    		
	ringbuffer_put(rb, (uint8_t *)&event_param, sizeof(event_param));

	//pthread_cond_signal(&data_stream->rgb_tv_data_cond);
}

void yuv_encode_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->yuv_encode_data_rb;
	image_frame_info_t frame_param, tmp_param;

	/* put to image analyzer queue */
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.offset = frame_info->offset;
    frame_param.frame_index = frame_info->frame_index;
	frame_param.frame_offset = frame_info->frame_offset;	
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->yuv_encode_data_cond);
}

static void osd_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->osd_data_rb;
	image_frame_info_t frame_param, tmp_param;

	/* put to image analyzer queue */
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.offset = frame_info->offset;
    frame_param.frame_index = frame_info->frame_index;
		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->yuv_encode_data_cond);
}

static void yuv_video_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->yuv_video_data_rb;
	image_frame_info_t frame_param, tmp_param;

	/* put to image analyzer queue */
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.offset = frame_info->offset;
    frame_param.frame_index = frame_info->frame_index;
    frame_param.frame_offset = frame_info->frame_offset;
		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->yuv_encode_data_cond);
}

static void rgb_zoom_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->rgb_zoom_data_rb;
	image_frame_info_t frame_param, tmp_param;
	
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.offset = frame_info->offset;
    frame_param.frame_index = frame_info->frame_index;
    		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->rgb_zoom_data_cond);
}

void H264_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->h264_data_rb;
	image_frame_info_t frame_param, tmp_param;
    Selfcheck_info.videorb_in_cnt++;
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
        Selfcheck_info.videorb_full_cnt++;
        //SLOGI("h264_data_rb full %d",Selfcheck_info.videorb_full_cnt);
	}
	frame_param.offset = frame_info->offset;
    frame_param.frame_offset = frame_info->frame_offset;
    frame_param.frame_index = frame_info->frame_index;
    		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->rgb_zoom_data_cond);
}
void rtsp_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->rtsp_data_rb;
	image_frame_info_t frame_param, tmp_param;
	
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.offset = frame_info->offset;
    frame_param.frame_offset = frame_info->frame_offset;
    frame_param.frame_index = frame_info->frame_index;
    		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->rgb_zoom_data_cond);
}

void rtmp_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->rtmp_data_rb;
	image_frame_info_t frame_param, tmp_param;
	
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.offset = frame_info->offset;
    frame_param.frame_offset = frame_info->frame_offset;
    frame_param.frame_index = frame_info->frame_index;
    		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->rgb_zoom_data_cond);
}

static void video_copy_to_queue(data_stream_t *data_stream,video_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->video_copy_rb;
	video_frame_info_t frame_param, tmp_param;

	/* put to image analyzer queue */
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
        SLOGI("ERR VIDEO RB FULL!\n");
	}
    memcpy(frame_param.real_name,frame_info->real_name,256);
    memcpy(frame_param.tmp_name,frame_info->tmp_name,256);
		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->yuv_encode_data_cond);
}

// static void draw_box_on_buf(uint8_t * buf)
// {
//     int i;
//     if(gs_nnie_state.run_state==ALG_RUN &&gs_nnie_state.init_state==1 &&(!track_state_mechine.user_state)){
// //#ifdef ZM_MP5_PL
//         //  if(OSD_ON==gs_vedio_info.osd_rknn_stat){
//         //     GUI_SetColor(0x00ff00);
//         //     if(gs_vedio_info.camera_type==TYPE_TV){
//         //         //GUI_DrawRect(420,0,1500,1080);
//         //         if(gs_vedio_info.pip_stat==PIP_TV){
//         //             //GUI_DrawLine(420,0,420,1080);
//         //             //GUI_DrawLine(1500,513,1500,1080);
//         //             //GUI_DrawCircle(960,540,540);
//         //             GUI_DrawArc(960,540,540,540,55,360);
//         //             GUI_DrawArc(960,540,540,540,0,2);
//         //             //GUI_DrawArc(960,540,540,540,179,360);
//         //         }else{
//         //             //GUI_DrawLine(420,0,420,1080);
//         //             //GUI_DrawLine(1500,0,1500,1080);
//         //             //GUI_DrawCircle(960,540,540);
//         //             GUI_DrawArc(960,540,540,540,0,360);
//         //         }

//         //     }else{
//         //         //GUI_DrawRect(420,28,1500,1080);
//         //         //GUI_DrawLine(420,28,420,1052);
//         //         //GUI_DrawLine(1500,28,1500,1052);
//         //         //GUI_DrawCircle(960,540,524);
//         //         GUI_DrawArc(960,540,524,524,0,360);
//         //     }
//         //  }
// //#endif
// #ifdef ZM_MP5_KBT
// 	char char_info[64];				
// 	sprintf(char_info,"AI open");
// 	GUI_DispStringAt(char_info,75,32);
// #endif	
//         for(i=0;i<g_nnie_res.tag_num;i++){
//             // draw_box_img_onrgb(VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,buf,g_nnie_res.nnie_result[i].x,\
//             // g_nnie_res.nnie_result[i].y,\
//             // g_nnie_res.nnie_result[i].x+g_nnie_res.nnie_result[i].w,\
//             // g_nnie_res.nnie_result[i].y+g_nnie_res.nnie_result[i].h,\
//             // 0,255,0);
//             if(gs_vedio_info.pip_stat==PIP_TV && g_nnie_res.nnie_result[i].x>1280 \
//             &&g_nnie_res.nnie_result[i].y<512){
//                 continue;
//             }
            
//             /// deprecated
//             // if((g_nnie_res.nnie_result[i].class_id == 1) && (g_nnie_res.nnie_result[i].w/g_nnie_res.nnie_result[i].h >= 8))
//             // {
//             //     continue;
//             // }
// #ifdef ZM_MP5_PL
//             switch(Selfcheck_info.rknn_class)
//             {
//                 case RKNN_CLASS_CAR:
//                     if(g_nnie_res.nnie_result[i].class_id == 1)
//                     {
//                         continue;
//                     }
//                     break;


//                 case RKNN_CLASS_PERSON:
//                     if(g_nnie_res.nnie_result[i].class_id == 0)
//                     {
//                         continue;
//                     }
//                     break;

//                 case RKNN_CLASS_BOTH:
//                     break;

//                 default:
//                     break;
//             }
// #endif

//             Draw_nnie(g_nnie_res.nnie_result[i].class_id,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,\
//             g_nnie_res.nnie_result[i].x,\
//             g_nnie_res.nnie_result[i].y,\
//             g_nnie_res.nnie_result[i].w,\
//             g_nnie_res.nnie_result[i].h);
//         }
//     }
//     if((track_state_mechine.run_state==ALG_RUN)&&(track_state_mechine.user_state)){
//         RECTH  recth;
//         int x, y, w, h;
//         x = recth.x;
//         y = recth.y;
//         w = recth.w;
//         h = recth.h; 

// #ifdef IR_INPUT_ENABLE
//         if(gs_vedio_info.camera_type==TYPE_IR){
//             int piontx=round((int)(x-320)*2)+960;
//             int pionty=round((int)(y-256)*2)+540;
//             x=piontx;
//             y=pionty;
//             w=round(w*2.0);
//             h=round(h*2.0);
//         }
// #endif       


//         // draw_box_img_onrgb(VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,buf,x,y,x+w,\
//         // y+h,255,255,255);
//         //Show_DspTraceVisible(x,y,w,h,0);
//         Draw_nnie(0xff,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,x,y,w,h);
//     }else if((track_state_mechine.run_state==ALG_SERCHING)&&(track_state_mechine.user_state))
// 	{

// 		if((track_state_mechine.search_count % 10) < 5)
// 				Show_Trace_Finding_Block(960,540);
//     }
// 	// }else if(track_state_mechine.run_state==ALG_MISS)
// 	// {
	
// 	// 	Show_Trace_Finding_Block(960,540);
// 	// } 
// }

void check_update_flag(){
    FILE *fp=0;
    static int check_flag=1;
    static int  open_ok_cnt=0;

    if(check_flag>250){
        remove("/tmp/sd_update");
        return;
    }
#ifdef UPDATE_DEBUG
    SLOGI("checkupdate\n");
#endif 
    char tmp[128]={0};
    fp=fopen("/tmp/sd_update","rb");
    if(fp==NULL){
        //SLOGI("no update \n");
        //Selfcheck_info.update_show=0;
        return;
    }else{        
        fread(tmp,1,128,fp);
        //SLOGI("%s \n",tmp);
        sscanf(tmp,"%d",&Selfcheck_info.update_flag);
        fclose(fp);
        open_ok_cnt++;
        if(open_ok_cnt==1){
            Selfcheck_info.update_show=1;
        }
#ifdef UPDATE_DEBUG
    SLOGI("update_flag %d\nupdate_show %d\n",Selfcheck_info.update_flag, Selfcheck_info.update_show);
#endif
        check_flag++;
    }
    // fp=fopen("/tmp/upgrade_success","rb");
    // if(fp==NULL){
    //     //SLOGI("no /tmp/upgrade_success \n");
    //     Selfcheck_info.update_flag=0;
    // }

    //fread(Selfcheck_info.update_string,1,64,fp);
}


pthread_t tid_1, tid_2, tid_3,tid_4,tid_5,tid_6,tid_7,tid_8,tid_9,tid_10,tid_11;

static void zm_check_sd_err(volatile unsigned int *sd_stat)
{
    static int check_i = 0;
    FILE *check_fp = NULL;
    if(check_i < 1)
    {
        
    check_i++;
    
    switch(*sd_stat)
    {
        case 0:
        {
            
            break;
        }

        case SD_full:
        {
            break;
        }

        case 1:
        {
            

            check_fp = fopen("/mnt/sdcard/check_sdcard.bak", "w+");
            if(!check_fp)
            {
                goto err1;
            }

            int check_num = 0; 
            check_num = fwrite("test,check sdcard\n",20,1,check_fp);
            if(check_num <= 0)
            {
                goto err2;
            }

            check_num = 0;
            char check_str[24];

            fclose(check_fp);

            check_fp = fopen("/mnt/sdcard/check_sdcard.bak", "r");
            if(!check_fp)
            {
                SLOGI("check_sd_bak not find\n");
                system("rm /mnt/sdcard/check_sdcard.bak");
                goto err1;
            }

            check_num = fread(check_str,24,1,check_fp);

            if(check_num < 0)
            {
                SLOGI("\n sd test1: %s \n",check_str);
                goto err2;
            }

            SLOGI("\n sd test2: %s \n",check_str);
            fclose(check_fp);
            system("rm /mnt/sdcard/check_sdcard.bak");
            break;
        }

        default:
        {
            break;
        }
    }

    }
    return;

    err2:
    fclose(check_fp);
    check_fp = NULL;
    system("rm /mnt/sdcard/check_sdcard.bak");
    err1:
    *sd_stat = SD_invalid;
    return;
}

void zm_update_sd_state(){
    int sd_card_ok=1;
    double sdcard_total=0;
    double sdcard_res=0;
    // get_sd_mount_capacity(&sdcard_total,&sdcard_res);
    gs_record_info.sdcard_total=round(sdcard_total);
    gs_record_info.sdcard_space=round(sdcard_res);
    // sd_card_ok=get_sd_mount_status();
    if(1==sd_card_ok){
        gs_record_info.sdcard_stat_send |= 0x49;         
    }else{
        gs_record_info.sdcard_stat_send=0;
    } 
    if(gs_record_info.sdcard_space<(gs_record_info.sdcard_total*0.1)){
        gs_record_info.sdcard_stat_send|=(1<<5);
        gs_record_info.record_user_stat=RECORD_STOP; 
        sleep(1);
        gs_record_info.sdcard_stat=SD_full;
    }else if(sd_card_ok==1){
        gs_record_info.sdcard_stat=1;
    }else{
        gs_record_info.sdcard_stat=0;
    }

    //zm_check_sd_err(&(gs_record_info.sdcard_stat));

}

static void *time_handle_thread(void* parameter)
{
    double t0,t1;
    //data_stream_t *data_stream = (data_stream_t *)parameter;
    struct timeval time;


    while(1) {
        time.tv_sec=0;
		time.tv_usec=(500*1000);
		select(0,NULL,NULL,NULL,&time);
        gs_time_info.m_second=gs_time_info.m_second0+round((what_time_is_it_now()-gs_time_info.t0)*1000);
        gs_time_info.sec = (unsigned int)(gs_time_info.m_second / 1000);
        gs_time_info.hour = (unsigned int)((gs_time_info.sec  / 3600) % 24);
		if(gs_time_info.hour >= 24)
		{
			gs_time_info.hour -= 24;
		}
		gs_time_info.min = (unsigned int)((gs_time_info.sec  % 3600) / 60);
		gs_time_info.sec = gs_time_info.sec % 60;

    }
}

static void  video_for_copy(char *record_name_tmp,char *record_name_real){

    char record_name_s[256];
    char record_name_d[256];
    memcpy(record_name_s,record_name_tmp,256);
    memcpy(record_name_d,record_name_real,256);
    char cmd[1024] = {0};
    
    sprintf(cmd,"cp %s %s",record_name_s,record_name_d);
    logger_info("data","cmd %s\n",cmd);
    system(cmd);

    sprintf(cmd,"rm %s",record_name_s);
    logger_info("data","cmd %s\n",cmd);
    system(cmd);

    sprintf(cmd,"sync");
    system(cmd);
}

// static int nnie_sig_out=1;
// static void nnie_sig_eixt_handler(int sig){
//   nnie_sig_out=0;
//   printf("nnie_sig_out");
//   pthread_exit(0);
// }

unsigned char * osd_buf=NULL;

void csi_video_in(unsigned char *buf,int flag){
#ifdef TV_THREAD_TIME_LOG     
    static double t0,t1,t2,t20;
    t0=what_time_is_it_now();
#endif
    // cpu_set_t mask;
	// int cpuid = 2;
	// CPU_ZERO(&mask);
	// CPU_SET(cpuid, &mask);
    pthread_mutex_lock(&mutex_vis_in);
    data_stream_t *data_stream = data_stream_get();
    image_frame_info_t frame_info;
 
    unsigned char * gpu_rgb_buf;
    int pitch=0;
    float algorithm_cost;
    data_stream->rgb_handle_data_count=data_stream->rgb_handle_data_count%DATA_STREAM_RGB_FRAME_MAX;
	void * gpu_buf_obj_tmp;
#ifdef CAMERA_INPUT_FORMAT_YVYU
	 	gpu_buf_obj_tmp=data_stream->buf_obj_i_vyuy[data_stream->rgb_handle_data_count];
#else
	 	gpu_buf_obj_tmp=data_stream->buf_obj_i_tv[data_stream->rgb_handle_data_count];
#endif


    gpu_rgb_buf = data_stream->htr.buffer_map(data_stream->hetero_hdr, \
    gpu_buf_obj_tmp,e_mapping_write);


    
    int ret;
#ifdef CAMERA_INPUT_FORMAT_YVYU    
    //  ret = rga_copy(data_stream->rga_rgb_copy,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,buf,\
    //  gpu_rgb_buf,RK_FORMAT_YCbCr_422_SP);
    memcpy(gpu_rgb_buf,buf,VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE*2);
//    if(data_stream->yuv_handle_frame_count==100){
//        save_image_data(buf,VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE*2,"/tmp/yuv_test.yuv");
//        gs_vedio_info.pip_stat=PIP_TV;
//    }
#else  
    ret = rga_copy(data_stream->rga_rgb_copy,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,buf,\
    gpu_rgb_buf,RK_FORMAT_RGB_888);
   // memcpy(gpu_rgb_buf,buf,VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE*3);
#endif
    // ret = rga_format_convert(data_stream->rga_rgb_copy,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,\
    // buf,gpu_rgb_buf,RK_FORMAT_YCbCr_422_SP,RK_FORMAT_BGR_888);
  //


    data_stream->htr.buffer_unmap(data_stream->hetero_hdr, \
    gpu_buf_obj_tmp, gpu_rgb_buf);


    frame_info.offset=gpu_buf_obj_tmp;
    frame_info.frame_index = data_stream->rgb_handle_data_count;
    rgb_data_to_queue(data_stream,&frame_info);

    pthread_mutex_unlock(&mutex_vis_in);

    data_stream->rgb_handle_data_count++;
    if(flag){
        data_stream->yuv_handle_frame_count++;
        Selfcheck_info.vis_err_stat=0;
    }
   // vis_input_frame_count+=1;
    // rga_copy(data_stream->rga_rgb_copy,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,rgb888_buf,drm_buf,\
    //   RK_FORMAT_RGB_888);  
#ifdef TV_THREAD_TIME_LOG         
    t1=what_time_is_it_now();
    t2+=(t1-t0);
    if(data_stream->yuv_handle_frame_count % 100==50){
      SLOGI(" tv rgb888_to_rb  %fms %fms\n",t2*10,(t1-t20)*1000);
      t2=0;
    }
    t20=t1;
#endif   
	
    #ifdef  SAVE_RAW_DATA  
    static int save=1;
    if(data_stream->yuv_handle_frame_count>=10*30 && save ){
        gs_photo_info.test_stat0=PHOTO_ONECE;
        gs_photo_info.test_stat1=PHOTO_ONECE;
        gs_photo_info.test_stat2=PHOTO_ONECE;
        save=0;
    }

    if(PHOTO_ONECE==gs_photo_info.test_stat0){
        save_image_data(buf,VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE*3,"/mnt/sdcard/yuv_in.raw");
        gs_photo_info.test_stat0=0;
    }
    #endif



}

void rtsp_video_in_rb(uint8_t *rgb888_buf){
#ifdef THREAD_TIME_LOG     
    static double t0,t1,t2;
    t0=what_time_is_it_now();
#endif
    // cpu_set_t mask;
	// int cpuid = 0;
	// CPU_ZERO(&mask);
	// CPU_SET(cpuid, &mask);
#if 1
    data_stream_t *data_stream = data_stream_get();
    image_frame_info_t frame_info;
 
    unsigned char * gpu_rgb_buf;
    int pitch=0;
    float algorithm_cost;
    data_stream->rgb_handle_data_count=data_stream->rgb_handle_data_count%DATA_STREAM_RGB_FRAME_MAX;

//
    gpu_rgb_buf = data_stream->htr.buffer_map(data_stream->hetero_hdr, \
    data_stream->buf_obj_i_tv[data_stream->rgb_handle_data_count],e_mapping_write);

    static int test1 = 1;
    FILE *fph = NULL;
    if(test1 == 1)
    {
        test1++;
        fph = fopen("/mnt/sdcard/d3.raw","wb+");

        fwrite(rgb888_buf,1920*1080*3,1,fph);

        fclose(fph);
    }

    int ret;
    ret = rga_format_convert(data_stream->rga_rgb_copy,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,\
    rgb888_buf,gpu_rgb_buf,RK_FORMAT_YCrCb_420_SP,RK_FORMAT_BGR_888);
  //
    data_stream->htr.buffer_unmap(data_stream->hetero_hdr, \
    data_stream->buf_obj_i_tv[data_stream->rgb_handle_data_count], gpu_rgb_buf);

    frame_info.offset=data_stream->buf_obj_i_tv[data_stream->rgb_handle_data_count];
    frame_info.frame_index = data_stream->rgb_handle_data_count;
    rgb_data_to_queue(data_stream,&frame_info);
  

    data_stream->rgb_handle_data_count++;
    data_stream->yuv_handle_frame_count++;
#endif
    // rga_copy(data_stream->rga_rgb_copy,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,rgb888_buf,drm_buf,\
    //   RK_FORMAT_RGB_888);  
#ifdef THREAD_TIME_LOG         
    t1=what_time_is_it_now();
    //if((t1-t0)*1000>33){
      SLOGI(" tv rgb888_to_rb  %f  ms  %f  ms  \n",(t1-t0)*1000,(t1-t2)*1000);
    //}
    t2=what_time_is_it_now();
#endif    
}
static void ir_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->rgb_ir_in_data_rb;
	image_frame_info_t frame_param, tmp_param;
	
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.offset = frame_info->offset;
    frame_param.frame_index = frame_info->frame_index;
		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));

	//printf("***%s+++\n",__func__);

	//pthread_cond_signal(&data_stream->ir_data_cond);
}

void ir_cewen_data_to_queue(data_stream_t *data_stream,image_frame_info_t *frame_info)
{
	ring_buffer_t *rb = data_stream->ir_cewen_data_rb;
	image_frame_info_t frame_param, tmp_param;
	
	if (ringbuffer_is_full(rb)) {					
		ringbuffer_get(rb, (uint8_t *)&tmp_param, sizeof(tmp_param));
	}
	
	frame_param.frame_offset = frame_info->frame_offset;
    frame_param.frame_index = frame_info->frame_index;
		
	ringbuffer_put(rb, (uint8_t *)&frame_param, sizeof(frame_param));
}



void analyze_temp_by_ir_param(short *ir_param_data){
    short *ir_param_line=(short *)ir_param_data;
    //if(ir_param_line[0]==0x55AA && ir_param_line[1]==0x0038){
        unsigned short ir_tmp = 0;
        ir_tmp = (unsigned short)(ir_param_line[44]);
        gs_ir_temp_info.x_th = ir_tmp;
       // SLOGI("xxxx%d\n",ir_tmp);
        ir_tmp = 0;
        ir_tmp = (unsigned short)(ir_param_line[45]);
        gs_ir_temp_info.y_th=ir_tmp;
     //   SLOGI("xxxx%d\n",ir_tmp);
        ir_tmp = 0;
        ir_tmp=((unsigned short)(ir_param_line[46]))*0.1;
        gs_ir_temp_info.th=ir_tmp;
     //   SLOGI("xxxx%d\n",ir_tmp);
        ir_tmp = 0;
        ir_tmp=(unsigned short)(ir_param_line[47]);
        gs_ir_temp_info.x_tl=ir_tmp;
     //   SLOGI("xxxx%d\n",ir_tmp);
        ir_tmp = 0;
        ir_tmp = (unsigned short)(ir_param_line[48]);
        gs_ir_temp_info.y_tl=ir_tmp;
    //    SLOGI("xxxx%d\n",ir_tmp);
        ir_tmp = 0;
        ir_tmp=((unsigned short)(ir_param_line[49]))*0.1;
        gs_ir_temp_info.tl=ir_tmp;
    //    SLOGI("xxxx%d\n",ir_tmp);
        ir_tmp = 0;
        ir_tmp=((unsigned short)(ir_param_line[53]))*0.1;

        gs_ir_temp_info.tc=ir_tmp;

        unsigned short piontx=0;
        unsigned short pionty=0;
        if(gs_vedio_info.camera_type==TYPE_IR){ 
            piontx=round((int)(gs_ir_temp_info.x_th-320)*2)+960;
            pionty=round((int)(gs_ir_temp_info.y_th-256)*2)+540;
            gs_ir_temp_info.x_th=piontx;
            gs_ir_temp_info.y_th=pionty;
            piontx=round((int)(gs_ir_temp_info.x_tl-320)*2)+960;
            pionty=round((int)(gs_ir_temp_info.y_tl-256)*2)+540;
            gs_ir_temp_info.x_tl=piontx;
            gs_ir_temp_info.y_tl=pionty;   
        }else{
            gs_ir_temp_info.x_th+=1280;
            gs_ir_temp_info.x_tl+=1280; 
        }
        gs_ir_temp_info.x_th%=1920;
        gs_ir_temp_info.y_th%=1080;
        gs_ir_temp_info.x_tl%=1920;
        gs_ir_temp_info.y_tl%=1080;
    //}
    // static int cnt=0;
    // cnt++;
    // if(cnt%100==99){
    //     SLOGI("%d %d %x %x",gs_ir_temp_info.x_th,gs_ir_temp_info.y_th,ir_param_line[45],ir_param_line[46]);
    // }
    
}

void ir_cewen_data_to_rb(unsigned char *ir_src_data,unsigned char *ir_param_data){
    data_stream_t *data_stream = data_stream_get();    
    image_frame_info_t frame_info;
    if (data_stream->ir_cewen_data_offset + data_stream->ir_cewen_buf_frame_size >  data_stream->ir_cewen_buf_total_size) {
        data_stream->ir_cewen_data_offset = 0;
    }
    memcpy(data_stream->ir_cewen_data+data_stream->ir_cewen_data_offset,ir_src_data,640*512*2);
    memcpy(data_stream->ir_cewen_data+data_stream->ir_cewen_data_offset+640*512*2,ir_param_data,640*2);
    frame_info.frame_offset=data_stream->ir_cewen_data_offset;
    ir_cewen_data_to_queue(data_stream,&frame_info);
    data_stream->ir_data_offset += data_stream->ir_buf_frame_size;
}

void ir_data_to_rb(unsigned char *ir_data,int flag)
{
#ifdef SAVE_RAW_DATA
    static int a = 0;
#endif

#ifdef IR_THREAD_TIME_LOG     
    static double t0,t1,t2,t20;
    static int ir_data_in_cnt=0;
    t0=what_time_is_it_now();
#endif   
    pthread_mutex_lock(&mutex_ir_in);
    data_stream_t *data_stream = data_stream_get();    
    image_frame_info_t frame_info;
    void *   gpu_rgb_buf;
    //int pitch=0;
    static unsigned int ir_ring_cnt=0;
    ir_ring_cnt=data_stream->ir_handle_data_count%DATA_STREAM_RGB_FRAME_MAX;
    //data_stream->ir_handle_data_count=data_stream->ir_handle_data_count%DATA_STREAM_RGB_FRAME_MAX;
    // if (data_stream->ir_data_offset + data_stream->ir_buf_frame_size >  data_stream->ir_buf_total_size) {
    //     data_stream->ir_data_offset = 0;
    // }

    // if(trace_status){
    //     save_image_data(ir_data,640*512*2,"/tmp/vyuy_ir.raw");
    //     trace_status=0;
    // }
    // vyuy_to_nv12(ir_data,IR_VIDEO_FRAME_WIDTH,IR_VIDEO_FRAME_HEIGHT\
    // ,data_stream->ir_yuv_data + data_stream->ir_data_offset);
    gpu_rgb_buf = data_stream->htr.buffer_map(data_stream->hetero_hdr, \
    data_stream->buf_obj_i_ir_vyuy[ir_ring_cnt], e_mapping_write);

    // rga_copy(data_stream->rga_yuv_copy,IR_VIDEO_FRAME_WIDTH,IR_VIDEO_FRAME_HEIGHT,\
    //  ir_data,gpu_rgb_buf,RK_FORMAT_YCbCr_422_SP);  


    memcpy(gpu_rgb_buf,ir_data,IR_VIDEO_FRAME_WIDTH*IR_VIDEO_FRAME_HEIGHT*2);

#ifdef SAVE_RAW_DATA
    if (a==0) {
        if(gs_photo_info.test_stat0 != PHOTO_ONECE)
        {
            gs_photo_info.test_stat0 = PHOTO_ONECE;
        }
        a++;
    }
#endif
    #ifdef  SAVE_RAW_DATA    
    if(PHOTO_ONECE==gs_photo_info.test_stat0){
        //printf("before save ir image\n");
        save_image_data(ir_data,IR_VIDEO_FRAME_WIDTH*IR_VIDEO_FRAME_HEIGHT*2,"/mnt/sdcard/ir_in.raw");
        gs_photo_info.test_stat0=0;
    }
    #endif
    data_stream->htr.buffer_unmap(data_stream->hetero_hdr, \
    data_stream->buf_obj_i_ir_vyuy[ir_ring_cnt], gpu_rgb_buf);

  
    // static int cnt=0;
    // cnt++;
    // if(cnt%10==0){
    //     Sensor_info.coeff_irz+=0.1;
    // }
    
    // rga_format_convert(data_stream->rga_yuv_copy,IR_VIDEO_FRAME_WIDTH,IR_VIDEO_FRAME_HEIGHT,\
    // data_stream->ir_yuv_data + data_stream->ir_data_offset,\
    // gpu_rgb_buf,RK_FORMAT_YCrCb_420_SP,RK_FORMAT_RGB_888);

    frame_info.frame_index = ir_ring_cnt;
    frame_info.offset= data_stream->buf_obj_i_ir_vyuy[ir_ring_cnt];
    ir_data_to_queue(data_stream,&frame_info);

    if(flag){
        data_stream->ir_handle_data_count++;
    }
     pthread_mutex_unlock(&mutex_ir_in);
    // struct timeval time;
	// time.tv_sec=0;
	// time.tv_usec=(5*1000);
	// select(0,NULL,NULL,NULL,&time);

   // data_stream->ir_data_offset += data_stream->ir_buf_frame_size;  
#ifdef IR_THREAD_TIME_LOG 
    ir_data_in_cnt++;
    t1=what_time_is_it_now();
    t20+=(t1-t2);
    if(ir_data_in_cnt%100==99){
        SLOGI("ir %fms %f\n",t20*10,(t1-t0)*1000);
        t20=0;
    }
    t2=t1;
#endif   
    
}

int  encode_data_wait_and_get(uint8_t *buf, uint32_t width, uint32_t height,
                   uint32_t hor_stride, uint32_t ver_stride)
{   
#ifdef encode_THREAD_TIME_LOG  
    static double t0,t1,t2;
    t0=what_time_is_it_now();
#endif
    data_stream_t *data_stream = data_stream_get(); 
    ring_buffer_t *rb = data_stream->yuv_encode_data_rb;
    image_frame_info_t frame_info={0};
    uint32_t size = 0;
    uint8_t *src_buf=NULL;
    void * gpu_yuv_buf=NULL;
    data_stream->yuv_encode_data_count++;
    struct timeval time;
    int i=0;
    int size_w,size_h,offset_h;
    //printf("***%s+++0\n",__func__);
    //double t0,t1;

    
    static FILE* fp = NULL;
    static int number = 0;

    int write_number = 0;

  

    while(ringbuffer_is_empty(rb)){
        time.tv_sec=0;
        time.tv_usec=(2*1000);
        select(0,NULL,NULL,NULL,&time);
        // return;
        //memcpy(&frame_info,&frame_info_pre,sizeof(image_frame_info_t));
    }
    for(i=0;i<DATA_STREAM_YUV_FRAME_MAX && (!ringbuffer_is_empty(rb)) ; i++){
        ringbuffer_get(rb, (uint8_t *)&frame_info, sizeof(image_frame_info_t));
    }    
    
  
   // ringbuffer_get(rb, (uint8_t *)&frame_info, sizeof(image_frame_info_t));
        //memcpy(&frame_info_pre,&frame_info,sizeof(image_frame_info_t));
    
    // src_buf=get_encode_buf();
    // if(src_buf==NULL){
    //     return 0;
    // }
    if(1){
         src_buf = data_stream->yuv_data+frame_info.frame_offset;
       //  printf("frame_info.frame_offset:%d\n",frame_info.frame_offset);
         size_w=VIDEO_FRAME_WIDTH;
         offset_h=VIDEO_FRAME_HEIGHT_STRIDE;
         size_h=1080;
         //ver_stride = 1080;
     }else{                         
          src_buf = data_stream->yuv_720p_data+frame_info.frame_offset;//data_stream->yuv_720p_data
          size_w=VIDEO_RESIZE_WIDTH;
          offset_h=VIDEO_RESIZE_HEIGHT;
          size_h=VIDEO_RESIZE_HEIGHT;
        //   gpu_yuv_buf= data_stream->htr.hetero_buffer_map(data_stream->hetero_hdr, \
        //    data_stream->buf_obj_o_yuv720[frame_info.frame_index],e_mapping_read);
        //   src_buf=gpu_yuv_buf;
     }



    //  if(number == 0)
    //  {
    //        fp = fopen("/1.yuv","w+");              
    //         if(!fp)            
    //         {              
    //             printf("open error mpp\n");        
    //         }
    //  }

          
    // if(number < 60)
    // {              
    //     //printf("data_stream->yuv_data+frame_info.frame_offset: %d \n",*(data_stream->yuv_data+frame_info.frame_offset));


    //     write_number = fwrite(src_buf,1,1920*1080/2*3,fp);


    //     printf("write_number:%d\n",write_number);

    //     number ++;				      
    // }

    // if(number == 60)
    // { 
    //     printf("i am close ------------------------------\n");
    //     fclose(fp);
    //     number++;
    // }

    uint8_t *buf_y = buf;
    uint8_t *buf_u = buf_y + hor_stride * ver_stride; // NOTE: diff from gen_yuv_image    
  
    //int32_t frame_size = (hor_stride * ver_stride)*3/2;

    // size =(hor_stride*ver_stride);
    size=size_w*offset_h;
    memcpy((void *)buf_y,(void *)src_buf,size);//size

    //  rga_copy(data_stream->rga_yuv_copy,size_w,size_h/2,\
    //  src_buf,buf_y,RK_FORMAT_YCbCr_422_SP);

    src_buf = src_buf + size;//size;
    //size = (width*height)/2;
    size=size_w*size_h/2;
    memcpy((void *)buf_u,(void *)src_buf,size);  //size /* hor_stride == width */


    //  rga_copy(data_stream->rga_yuv_copy,size_w,size_h,\
    //  src_buf,buf_y,RK_FORMAT_YCbCr_420_SP);

    // if(0==gs_vedio_info.encode_1080p){
    //  data_stream->htr.hetero_buffer_unmap(data_stream->hetero_hdr, \
    //  data_stream->buf_obj_o_yuv720[frame_info.frame_index], gpu_yuv_buf);
    // }
    //usleep(9000);
#ifdef encode_THREAD_TIME_LOG 
    t1=what_time_is_it_now();
    //if((t1-t0)*1000>33){
    SLOGI("encode %fms %fms %08x \n",(t1-t0)*1000,(t1-t2)*1000,frame_info.frame_index);
    //}
    t2=t1;  
#endif  
    return 0;
}
int  video_data_wait_and_get(uint8_t *buf, uint32_t width, uint32_t height,
                   uint32_t hor_stride, uint32_t ver_stride,uint32_t frame_conut)
{   
#ifdef video_THREAD_TIME_LOG  
    static double t0,t1,t2,t20;
    static int vedio_cnt=0;
#endif
    data_stream_t *data_stream = data_stream_get(); 
    ring_buffer_t *rb = data_stream->yuv_video_data_rb;
    image_frame_info_t frame_info;
    uint32_t size = 0;
    uint8_t *src_buf;
   // data_stream->yuv_encode_data_count++;
    struct timeval time;
    int i=0;
    //printf("***%s+++0\n",__func__);
    //double t0,t1;
    while(ringbuffer_is_empty(rb)){
         time.tv_sec=0;
         time.tv_usec=(2*1000);
         select(0,NULL,NULL,NULL,&time);
    }
    while(frame_conut>30 && (gs_record_info.record_user_stat==0)&& (gs_record_info.record_run_stat!=1)){
         time.tv_sec=0;
         time.tv_usec=(20*1000);
         select(0,NULL,NULL,NULL,&time);  
         for(i=0;i<DATA_STREAM_YUV_FRAME_MAX && (!ringbuffer_is_empty(rb)) ; i++){
         ringbuffer_get(rb, (uint8_t *)&frame_info, sizeof(image_frame_info_t));
       }   
    }
#ifdef video_THREAD_TIME_LOG      
    t0=what_time_is_it_now();
#endif
    //for(i=0;i<DATA_STREAM_YUV_FRAME_MAX && (!ringbuffer_is_empty(rb)) ; i++){
         ringbuffer_get(rb, (uint8_t *)&frame_info, sizeof(image_frame_info_t));
    //}   
   
    src_buf = data_stream->yuv_data+frame_info.frame_offset;
    // void * gpu_rgb_buf = data_stream->htr.hetero_buffer_map(data_stream->hetero_hdr, \
    // frame_info.offset,e_mapping_read);
    // src_buf=gpu_rgb_buf;
    uint8_t *buf_y = buf;
    uint8_t *buf_u = buf_y + hor_stride * ver_stride; // NOTE: diff from gen_yuv_image    
  
    //int32_t frame_size = (hor_stride * ver_stride)*3/2;

   // size =(hor_stride*ver_stride);
    size=VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE;
    memcpy((void *)buf_y,(void *)src_buf,size);//size

    //  rga_copy(data_stream->rga_yuv_copy,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE/2,\
    //  src_buf,buf_y,RK_FORMAT_YCbCr_422_SP);

    src_buf = src_buf + size;//size;
    //size = (width*height)/2;
    size=VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT/2;
    memcpy((void *)buf_u,(void *)src_buf,size);  //size /* hor_stride == width */

    // rga_copy(data_stream->rga_yuv_copy,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE,\
    //  src_buf,buf_y,RK_FORMAT_YCbCr_420_SP);

    // data_stream->htr.hetero_buffer_unmap(data_stream->hetero_hdr, \
    // frame_info.offset, gpu_rgb_buf);
    
    //usleep(9000);
#ifdef video_THREAD_TIME_LOG 
    t1=what_time_is_it_now();
    vedio_cnt++;
    t20+=(t1-t2);
    if(vedio_cnt%100==99){
     SLOGI("vedio %fms\n",t20*10);
     t20=0;
    }
    t2=t1;  
#endif  
    return 0;
}

int  jpeg_data_wait_and_get(uint8_t *buf, uint32_t width, uint32_t height,
                   uint32_t hor_stride, uint32_t ver_stride)
{   
#ifdef THREAD_TIME_LOG  
    static double t0,t1,t2;
    t0=what_time_is_it_now();
#endif
    data_stream_t *data_stream = data_stream_get(); 
    uint32_t size = 0;
    uint8_t *src_buf;
    struct timeval time;
    while(PHOTO_STOP==gs_photo_info.photo_stat){
        time.tv_sec=0;
        time.tv_usec=(20*1000);
        select(0,NULL,NULL,NULL,&time);
    }
    //src_buf = data_stream->jpeg_yuv_buf;

    if(PHOTO_ONECE==gs_photo_info.photo_stat){
        gs_photo_info.photo_stat=PHOTO_STOP;
        gs_photo_info.photo_stat_show=30;
    }
    // src_buf=get_encode_buf();
    // if(src_buf==NULL){
    //     return 0;
    // }
    src_buf = data_stream->jpeg_yuv_buf;
    // void * gpu_rgb_buf = data_stream->htr.hetero_buffer_map(data_stream->hetero_hdr, \
    // frame_info.offset,e_mapping_read);
    //src_buf=gpu_rgb_buf;
    uint8_t *buf_y = buf;
    uint8_t *buf_u = buf_y + hor_stride * ver_stride; // NOTE: diff from gen_yuv_image    
  
    //int32_t frame_size = (hor_stride * ver_stride)*3/2;

   // size =(hor_stride*ver_stride);
    size=VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE;
    memcpy((void *)buf_y,(void *)src_buf,size);//size

    src_buf = src_buf + size;//size;
    //size = (width*height)/2;
    size=VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE/2;
    memcpy((void *)buf_u,(void *)src_buf,size);  //size /* hor_stride == width */

    // data_stream->htr.hetero_buffer_unmap(data_stream->hetero_hdr, \
    // frame_info.offset, gpu_rgb_buf);
    
    //usleep(9000);
#ifdef THREAD_TIME_LOG 
    t1=what_time_is_it_now();
    //if((t1-t0)*1000>33){
    SLOGI(" encode_data  %f  ms  %f  ms  \n",(t1-t0)*1000,(t1-t2)*1000);
    //}
    t2=t1;  
#endif  
    return 0;
}

int data_stream_init(void)
{
    int ret = 0;
    data_stream_t *data_stream = NULL;

    data_stream = (data_stream_t *)malloc(sizeof(*data_stream));
    if (!data_stream) {
        printf("XF:malloc data stream failed\n");
        return RET_ERR_MALLOC;
    }

    memset(data_stream, 0,sizeof(*data_stream));
    memset(&gs_photo_info, 0,sizeof(photo_info_t));
    memset(&gs_record_info, 0,sizeof(record_info_t));
    memset(&gs_vedio_info, 0,sizeof(zm_vedio_info_t));
  
    gs_vedio_info.camera_type=TYPE_TV;
    gs_vedio_info.input_type=TYPE_HDMI;
    gs_vedio_info.osd_color=0xffffff;
    gs_vedio_info.osd_ir_temp=OSD_OFF;
    #ifdef ZM_DEBUG_MODE
    gs_vedio_info.camera_type=TYPE_IR;
     Sensor_info.coeff_irz=1.1;
    #endif
#ifdef SINGLE_IR_DEBUG
    gs_vedio_info.camera_type=TYPE_IR;
#endif
    data_stream->yuv_buf_frame_size = (VIDEO_FRAME_WIDTH*VIDEO_FRAME_HEIGHT_STRIDE*2);
    data_stream->yuv_buf_total_size = (data_stream->yuv_buf_frame_size*(DATA_STREAM_YUV_FRAME_MAX));

    data_stream->yuv_data = (uint8_t *)malloc(data_stream->yuv_buf_total_size);
    if (!data_stream->yuv_data) {
        printf("XF:malloc yuv_data buffer failed\n");
        return RET_ERR_MALLOC;
    }

     printf("data_stream->yuv_data: %p\n",data_stream->yuv_data);

  //  VisibleOSDInit(&data_stream->yuv_data,VIDEO_FRAME_WIDTH,VIDEO_FRAME_HEIGHT_STRIDE);
    data_stream->yuv_720p_data = (uint8_t *)malloc(data_stream->yuv_buf_total_size);
    if (!data_stream->yuv_720p_data) {
        printf("XF:malloc yuv_720p_data buffer failed\n");
        return RET_ERR_MALLOC;
    }
#ifdef IR_INPUT_ENABLE
    data_stream->ir_buf_frame_size = (IR_VIDEO_FRAME_WIDTH*IR_VIDEO_FRAME_HEIGHT*2);
    data_stream->ir_buf_total_size = (data_stream->ir_buf_frame_size*IR_DATA_STREAM_RGB_FRAME_MAX);
    data_stream->ir_yuv_data = (uint8_t *)malloc(data_stream->ir_buf_total_size);
    if (!data_stream->ir_yuv_data) {
        printf("XF:malloc ir_yuv_data buffer failed\n");
        return RET_ERR_MALLOC;
    }

    data_stream->ir_yuv_data_rb = ringbuffer_create(DATA_STREAM_YUV_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->ir_yuv_data_rb) {
        printf("XF:create ir_yuv_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }
    data_stream->rgb_ir_in_data_rb = ringbuffer_create(IR_DATA_STREAM_RGB_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->rgb_ir_in_data_rb) {
        printf("XF:create rgb_ir_in_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }


#endif 

    data_stream->video_copy_rb = ringbuffer_create(IR_DATA_STREAM_RGB_FRAME_MAX*sizeof(video_frame_info_t));
    if (!data_stream->video_copy_rb) {
        printf("XF:create video_copy_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }
    data_stream->h264_buf_total_size = (256*1024*1024);
    data_stream->h264_data = (uint8_t *)malloc(data_stream->h264_buf_total_size);
    if (!data_stream->h264_data) {
        printf("XF:malloc h264_data buffer failed\n");
        return RET_ERR_MALLOC;
    }


    data_stream->ir_cewen_data_rb = ringbuffer_create(DATA_STREAM_YUV_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->ir_cewen_data_rb) {
        printf("XF:create ir_cewen_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }
    data_stream->ir_cewen_buf_frame_size =(IR_VIDEO_FRAME_WIDTH*(IR_VIDEO_FRAME_HEIGHT+1)*2);
    data_stream->ir_cewen_buf_total_size = (data_stream->ir_cewen_buf_frame_size*DATA_STREAM_YUV_FRAME_MAX);
    data_stream->ir_cewen_data = (uint8_t *)malloc(data_stream->ir_cewen_buf_total_size);
    if (!data_stream->ir_cewen_data) {
        printf("XF:malloc ir_cewen_data buffer failed\n");
        return RET_ERR_MALLOC;
    }


    data_stream->yuv_encode_data_rb = ringbuffer_create(DATA_STREAM_YUV_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->yuv_encode_data_rb) {
        printf("XF:create yuv encode data rb buffer failed\n");
        return RET_ERR_FAILED;
    }

    data_stream->osd_data_rb = ringbuffer_create(DATA_STREAM_YUV_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->osd_data_rb) {
        printf("XF:create osd_data_rb buffer failed\n");
        return RET_ERR_FAILED;
    }


    data_stream->yuv_video_data_rb = ringbuffer_create(DATA_STREAM_YUV_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->yuv_video_data_rb) {
        printf("XF:create yuv_video_data_rb buffer failed\n");
        return RET_ERR_FAILED;
    }
    
    data_stream->yuv_data_rb = ringbuffer_create(DATA_STREAM_YUV_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->yuv_data_rb) {
        printf("XF:create yuv_data rb buffer failed\n");
        return RET_ERR_FAILED;
    }

    data_stream->rgb_zoom_data_rb = ringbuffer_create(DATA_STREAM_RGB_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->rgb_zoom_data_rb) {
        printf("XF:create rgb_zoom_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }


    data_stream->rgb_tv_in_data_rb = ringbuffer_create(DATA_STREAM_RGB_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->rgb_tv_in_data_rb) {
        printf("XF:create rgb_tv_in_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }

    data_stream->rgb_out_data_rb = ringbuffer_create(DATA_STREAM_RGB_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->rgb_out_data_rb) {
        printf("XF:create rgb_out_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }

    data_stream->h264_data_rb = ringbuffer_create(600*sizeof(image_frame_info_t));
    if (!data_stream->h264_data_rb) {
        printf("XF:create h264_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }

    data_stream->rtsp_buf_total_size = (1024*1024*DATA_STREAM_YUV_FRAME_MAX);
    data_stream->rtsp_data = (uint8_t *)malloc(data_stream->rtsp_buf_total_size);
    if (!data_stream->rtsp_data) {
        printf("XF:malloc rtsp_data buffer failed\n");
        return RET_ERR_MALLOC;
    }

    data_stream->rtsp_data_rb = ringbuffer_create(DATA_STREAM_RGB_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->rtsp_data_rb) {
        printf("XF:create rtsp_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }

    data_stream->yuv_out_rb = ringbuffer_create(DATA_STREAM_RGB_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->yuv_out_rb) {
        printf("XF:create yuv_out_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }
   #ifdef RTMP_OUTPUT_ENABLE
    data_stream->rtmp_data_rb = ringbuffer_create(DATA_STREAM_RGB_FRAME_MAX*sizeof(image_frame_info_t));
    if (!data_stream->rtmp_data_rb) {
        printf("XF:create rtmp_data_rb rb buffer failed\n");
        return RET_ERR_FAILED;
    }
    #endif

    data_stream->rga_yuv_copy = RgaCreate();
    if (!data_stream->rga_yuv_copy) {
        printf("create rga_yuv_copy failed !\n");
        return RET_ERR_FAILED;
    }

    data_stream->rga_rgb_copy = RgaCreate();
    if (!data_stream->rga_rgb_copy) {
        logger_info("data","create rga_rgb_copy failed !\n");
        return RET_ERR_FAILED;
    }


    data_stream->rga_yuv_encode = RgaCreate();
    if (!data_stream->rga_yuv_encode) {
        logger_info("data","create rga_yuv_encode failed !\n");
        return RET_ERR_FAILED;
    }

    gs_data_stream = data_stream;

    printf("data stream init ok \n");
  
    return RET_OK;
}

data_stream_t *data_stream_get(void)
{
    return gs_data_stream;
}

void data_stream_deinit(void)
{
    data_stream_t *data_stream = data_stream_get();

    if (data_stream) {
        if (data_stream->yuv_data) {
            free(data_stream->yuv_data);
        }
        if (data_stream->output_yuv_data) {
            free(data_stream->output_yuv_data);
        }
        if (data_stream->rgb_zoom_data) {
            free(data_stream->rgb_zoom_data);
        }
        if (data_stream->yuv_720p_data) {
            free(data_stream->yuv_720p_data);
        }
        if (data_stream->h264_data) {
            free(data_stream->h264_data);
        }
        if (data_stream->ir_yuv_data) {
            free(data_stream->ir_yuv_data);
        }  

        if (data_stream->rtsp_data) {
            free(data_stream->rtsp_data);
        }
        if (data_stream->ir_cewen_data) {
            free(data_stream->ir_cewen_data);
        }
    }
    free(data_stream);
    data_stream = NULL;   
      
	rk_rga_deinit();
}

void decode_yuv_to_rb(MppFrame frame)
{
  #if 1
  SLOGI("decode_yuv_to_rb ...\n");

    double t0,t1;
    cpu_set_t mask;
	int cpuid = 0;
	CPU_ZERO(&mask);
	CPU_SET(cpuid, &mask);
	if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
	{
		printf("yolo_handle_thread init error\n");
	}

    data_stream_t *data_stream = data_stream_get();
    ring_buffer_t *rb = data_stream->yuv_data_rb;

    image_frame_info_t frame_info;

    RK_U32 width    = 0;
    RK_U32 height   = 0;
    RK_U32 h_stride = 0;
    RK_U32 v_stride = 0;
    MppFrameFormat fmt  = MPP_FMT_YUV420SP;
    MppBuffer buffer    = NULL;
    RK_U8 *base = NULL;
    RK_U8 *base_y = NULL; 
    RK_U8 *base_c = NULL; 
    RK_U32 i = 0,offset = 0;
    RK_U32 yuv_frame_size = 0;

#if DATA_ENGINE_PERFORMANCE_ANALYSIS
    struct timespec start_tm,end_tm;
    clock_gettime(CLOCK_REALTIME, &start_tm);
#endif 
    width    = mpp_frame_get_width(frame);
    height   = mpp_frame_get_height(frame);
    h_stride = mpp_frame_get_hor_stride(frame);
    v_stride = mpp_frame_get_ver_stride(frame);
    fmt      = mpp_frame_get_fmt(frame);
    buffer   = mpp_frame_get_buffer(frame);

    SLOGI("mpp_frame_ge_width %d_ _ _\n",width);
    SLOGI("mpp_frame_ge_height %d_ _ _\n",height);
    SLOGI("mpp_frame_ge_h %d_ _ _\n",h_stride);
    SLOGI("mpp_frame_ge_v %d_ _ _\n",v_stride);
    SLOGI("mpp_frame_ge_fmt %d_ _ _\n",fmt);

    base = (RK_U8 *)mpp_buffer_get_ptr(buffer);    
    // base_y = base;
    // base_c = base + h_stride * v_stride;
    // }
    rtsp_video_in_rb(base);
#endif

}
