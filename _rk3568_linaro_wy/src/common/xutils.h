/*
 * File      : utils.h
 * This file is utils header  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-1-12     fengyong    first version
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * General Purpose Utilities
 */
#define min(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x < __y) ? __x : __y; })

#define max(X, Y)				\
	({ typeof(X) __x = (X);			\
		typeof(Y) __y = (Y);		\
		(__x > __y) ? __x : __y; })

#define __compiler_offsetof(a,b) __builtin_offsetof(a,b)
#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif


typedef struct rgb_buf_tmp{
	char R;
	char G;
	char B;
}rgb_buf_tmp_t;
int zm_is_ipv4_addr(char *ip);
double what_time_is_it_now();
unsigned char check_sum(unsigned char *buff, int length);
unsigned char check_sum_xor(unsigned char *buff, int length);
unsigned long diff_timespec(struct timespec *end_tm, struct timespec *start_tm);
void xt_nanosleep(int sec,long nsec);
int save_image_data(char *data_buf, unsigned int image_size, const char *file_name);  
int rt_priority_thread_create(pthread_t *thread, int priority,void *(*start_routine) (void *), void *arg);
int read_data_to_buf(unsigned char *data_buf, const char *filename);
void draw_uv_box(int src_w, int src_h, unsigned char *data, int x, int y, int draw_w, int draw_h, unsigned u_color, unsigned v_color);
int vyuy_to_nv12(unsigned char *src_buffer, int w, int h, unsigned char *des_buffer);
extern void draw_box_img(int w, int h, unsigned char *data,int x1, int y1, int x2, int y2, unsigned char color);
extern void draw_box_img_onrgb(int w, int h, unsigned char *data,int x1, int y1, int x2, int y2, unsigned char color, unsigned char color1, unsigned char color2);
#ifdef __cplusplus
}
#endif

#endif /* _UTILS_H_ */


