#ifndef TRACH_H
#define TRACH_H


#ifdef __cplusplus
extern "C"{
#endif	

	#include "interface.h"
	#include "video_common.h"

	
	typedef struct
	{
		unsigned int work;
		unsigned int startFrame;
		int x;
		int y;
		unsigned int width;
		unsigned int height;
	}TrakCtlPara_t;

	extern target_info_st obj_detect_target[100];
	int eco_handle(void *eco_buff,target_info_st *target_info,int target_num,int buf_w,int buf_h);
	extern RECTH ecobbox_ctrl;
	extern void get_eco_recth(RECTH * tmp_recth);
	int track_init(void);
	void prev_frame_current_up(void);
	unsigned char* get_frame_from_buff(int status,int *now,int track_init_type);
	extern RECTH track_recth_result;
	extern target_info_st ecobbox[4];
	extern int init_track_dl(void);
	extern void *pso1;
#ifdef __cplusplus
}
#endif



#endif