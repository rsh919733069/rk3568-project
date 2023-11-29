#ifndef _NNIE_H_
#define _NNIE_H_
#include "interface.h"
#include "video_common.h"

#ifdef __cplusplus
extern "C"{
#endif


#define YOLOV4_TINY_MODEL_PATH	"./yolov4_tiny_tv.rknn"

#define TARGET_NUM_MAX 50 
#define NNIE_RINGBUF_NUM 16//4
typedef struct nnie_res{
    unsigned int frame_num;
	unsigned int tag_num;
	RECTH nnie_result[TARGET_NUM_MAX];
}nnie_res_t;

#define RKNN_MODEL_TYPE_VIS 2
#define RKNN_MODEL_TYPE_IRF 1

#define NO_RKNN_HEAD 0
#define RKNN_HEAD_LENGTH 1024

typedef struct zm_rknn_head{
	char type[4];
	char chn[4];
	char user[16];
	int  cls;
	int  anchors[18];
	char date[16];
	char srcPt[256];
	char srcRknn[256];
	char tmp[396];
}zm_rknn_head_t;

int nnie_yolov3tiny_init(int type);
int nnie_yolov3_init();
int my_rknn_deinit();
int my_rknn_init_early();
int search_biggest_area(target_info_st *ecobbox_ctrl,target_info_st *recth,int target_num_t);
int nnie_yolov3tiny_update(unsigned char *buf,nnie_res_t * g_nnie_res_inner);
int need_reinit(target_info_st *target, target_info_st *obj_detect_target_t, int target_num_t);
int nnie_yolov3_update(unsigned char *buf1, nnie_res_t * g_nnie_res_inner);
int nnie_yolov4tiny_update(unsigned char *buf,nnie_res_t * g_nnie_res_inner);
int InitTrackRect_Can_Traing(int src_w, int src_h, unsigned char *src_buf, float x, float y, float w, float h);
#ifdef __cplusplus
}
#endif

#endif