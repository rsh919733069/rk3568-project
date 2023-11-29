/*
 * File      : utils.c
 * This file is utils file  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-1-12     fengyong    first version
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "xutils.h"

int zm_is_ipv4_addr(char *ip)
{
	if (ip == NULL || ip[0] == '0' || ip[0] == '\0') {
		return -1;
	}

	for (int i = 0, count = 0; i < strlen(ip); i++) {
		if ((ip[i] != '.') && (ip[i] < '0' || ip[i] > '9')) {
			return -1;
		}
		if (ip[i] == '.') {
			count++;
			if (count > 3) {
				return -1;
			}
		}
	}

	int ip_num[4] = {-1, -1, -1, -1};
	char ip_s[4][4];
	memset(ip_s, 0, sizeof(char[4]) * 4);

	sscanf(ip, "%[^.].%[^.].%[^.].%[^ ]", ip_s[0], ip_s[1], ip_s[2], ip_s[3]);
	sscanf(ip_s[0], "%d", &ip_num[0]);
	sscanf(ip_s[1], "%d", &ip_num[1]);
	sscanf(ip_s[2], "%d", &ip_num[2]);
	sscanf(ip_s[3], "%d", &ip_num[3]);

	for (int i = 0; i < 4; i++) {
		if (strlen(ip_s[i]) == 0 || (ip_s[i][0] == '0' && ip_s[i][1] != '\0') || ip_num[i] < 0 || ip_num[i] > 255) {
			return -1;
		}
	}

	return 0;
}
//src_w src_h是原图 1920 * 1080
//使用的是420 4个Y用一组UV, 底下的UV是交错
// 
void draw_uv_box(int src_w, int src_h, unsigned char *data, int x, int y, int draw_w, int draw_h, unsigned u_color, unsigned v_color)
{
	int i;

	if (x < 0) x = 0;
	if (y < 0) y = 0;

	if((draw_w <= 0) && (draw_h <= 0))
		return ;


	if (x + draw_w > src_w) x = src_w - draw_w;
	if (y + draw_h > src_h) y = src_h - draw_h;

	if (x % 2 != 0)
		x -= 1;
	if (y % 2 != 0)
		y -= 1;
	if (draw_w % 2 != 0)
		draw_w -= 1;
	if (draw_h % 2 != 0)
		draw_h -= 1;

	int  y2w= y / 2 * src_w;
	int  y2w1= 1 + y2w;
	int  ydraw_h= (y + draw_h) / 2 * src_w;
	int  ydraw_h1= 1 +ydraw_h;
	for (i = x/2; i <= (x + draw_w)/2; ++i) {
		data[i * 2 + y2w] = u_color;
		data[i * 2 + y2w1] = v_color;
		data[i * 2 + ydraw_h] = u_color;
		data[i * 2 + ydraw_h1] = v_color;
	}

	for (i = y/2; i < (y + draw_h)/2; ++i) {
		data[x + i * src_w] = u_color;
		data[x + 1 + i * src_w] = v_color;
		data[x + draw_w + i * src_w] = u_color;
		data[x + draw_w + 1 + i * src_w] = u_color;
	}

	return;

}
void draw_box_img_onrgb(int w, int h, unsigned char *data,int x1, int y1, int x2, int y2,unsigned char color
, unsigned char color1, unsigned char color2)
{
    int i;
	rgb_buf_tmp_t *rgb_buf_tmp=(rgb_buf_tmp_t *)data;
    if (x1 < 0) x1 = 0;
    if (x1 >= w) x1 = w - 1;
    if (x2 < 0) x2 = 0;
    if (x2 >= w) x2 = w - 1;

    if (y1 < 0) y1 = 0;
    if (y1 >= h) y1 = h - 1;
    if (y2 < 0) y2 = 0;
    if (y2 >= h) y2 = h - 1;
	int  y1w=y1 * w;
	int  y2w=y2 * w;
	int  y11W=(y1+1) * w;
	int  y21W=(y2+1) * w;
    for (i = x1; i <= x2; ++i) {
	     rgb_buf_tmp[i + y1w].R = color;
		rgb_buf_tmp[i + y11W].R= color;
        rgb_buf_tmp[i + y2w].R = color;
		rgb_buf_tmp[i + y21W].R = color;

        rgb_buf_tmp[i + y1w].G = color1;
		rgb_buf_tmp[i + y11W].G = color1;
        rgb_buf_tmp[i + y2w].G = color1;
		rgb_buf_tmp[i + y21W].G = color1;

		rgb_buf_tmp[i + y1w].B = color2;
		rgb_buf_tmp[i + y11W].B = color2;
        rgb_buf_tmp[i + y2w].B = color2;
		rgb_buf_tmp[i + y21W].B = color2;
    }
	int x11=x1+1;
	int x21=x2+1;
    for (i = y1; i <= y2; ++i) {
        rgb_buf_tmp[x1 + i * w].R = color;
		rgb_buf_tmp[x11 + i * w].R = color;
        rgb_buf_tmp[x2 + i * w].R = color;
		rgb_buf_tmp[x21+ i * w].R = color;

		rgb_buf_tmp[x1 + i * w].G = color1;
		rgb_buf_tmp[x11 + i * w].G = color1;
        rgb_buf_tmp[x2 + i * w].G = color1;
		rgb_buf_tmp[x21+ i * w].G = color1;

		rgb_buf_tmp[x1 + i * w].B = color2;
		rgb_buf_tmp[x11 + i * w].B = color2;
        rgb_buf_tmp[x2 + i * w].B = color2;
		rgb_buf_tmp[x21+ i * w].B = color2;
    }
}

void draw_box_img(int w, int h, unsigned char *data,int x1, int y1, int x2, int y2, unsigned char color)
{
    int i;
    if (x1 < 0) x1 = 0;
    if (x1 >= w) x1 = w - 1;
    if (x2 < 0) x2 = 0;
    if (x2 >= w) x2 = w - 1;

    if (y1 < 0) y1 = 0;
    if (y1 >= h) y1 = h - 1;
    if (y2 < 0) y2 = 0;
    if (y2 >= h) y2 = h - 1;

	int  y1w=y1 * w;
	int  y2w=y2 * w;
	int  y11W=(y1+1) * w;
	int  y21W=(y2+1) * w;
	int x11=x1+1;
	int x21=x2+1;
    for (i = x1; i <= x2; ++i) {
        data[i + y1w] = color;
		data[i + y11W] = color;

        data[i + y2w] = color;
		data[i + y21W] = color;
    }
    for (i = y1; i <= y2; ++i) {
        data[x1 + i * w] = color;
		data[x11 + i * w] = color;

        data[x2 + i * w] = color;
		data[x21 + i * w] = color;
    }
}
unsigned long diff_timespec(struct timespec *end_tm, struct timespec *start_tm)
{
	struct timespec r;
	int i = 0;
	unsigned long ret = 0;

	r.tv_sec = end_tm->tv_sec - start_tm->tv_sec;
	if (end_tm->tv_nsec < start_tm->tv_nsec) {
		r.tv_nsec = end_tm->tv_nsec + 1000000000L - start_tm->tv_nsec;
		r.tv_sec--;
	} else {
		r.tv_nsec = end_tm->tv_nsec - start_tm->tv_nsec ;
	}

	ret = r.tv_nsec;
	for (i = 0; i < (int)(r.tv_sec); i++)
		ret += 1000000000L;
	
	return ret;
}


void xt_nanosleep(int sec,long nsec)
{
	struct timespec req = {.tv_sec = sec,
						   .tv_nsec = nsec};
	nanosleep(&req,NULL);
}

int save_image_data(char *data_buf, unsigned int image_size, const char *file_name)
{
	FILE *fp = fopen(file_name, "wb+");	
	if (!fp) {
		printf("(Error):%s Cannot open file %s\n",__func__,file_name);
		return 1;
	}

	int ret = fwrite(data_buf, sizeof(unsigned char), image_size,fp);
	if(ret != image_size) {
		printf("write %s failed\n", file_name);
        fclose(fp);    
        return 2;
	}

	fclose(fp);
	return 0;
}

void save_file_data_append(const char *fileName, const unsigned char *imageData, int size) 
{
	FILE *fp = fopen(fileName, "ab+");
	if (fp == NULL) {
		return;
	}
	
	fwrite(imageData, size, 1, fp);
	fclose(fp);
}

int rt_priority_thread_create(pthread_t *thread, int priority,void *(*start_routine) (void *), void *arg)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	struct sched_param param;
	param.sched_priority = priority; 
	pthread_attr_setschedparam(&attr, &param);

    return pthread_create(thread,&attr,start_routine,arg);
}

static size_t file_size_get(const char* filename) 
{
  struct stat st;
  //return_value_if_fail(filename != NULL, 0);

  if (stat(filename, &st) == 0) {
    return st.st_size;
  }

  return 0;
}

int read_data_to_buf(unsigned char *data_buf, const char *filename)
{
    int ret = 0;    
	FILE *fp = fopen(filename, "rb");	
    if (fp != NULL) {
        int len = file_size_get(filename);
        printf("file %s size=%d\n",filename,len);
        ret = fread((void *)data_buf,len, 1,fp);
        if (1 == ret ) {			
			ret = len;
		}
		else {
			ret = 0;
		}

        fclose(fp);
        return ret;
    }
    
    return 0;	
}

int vyuy_to_nv12(unsigned char *src_buffer, int w, int h, unsigned char *des_buffer)
{
		unsigned char *yuv, *nv12;
		unsigned char u, v, y1, y2;
    	int i = 0, j = 0;
		int size = w * h;

		yuv = src_buffer;
		nv12 = des_buffer;
 
		if (yuv == NULL || nv12 == NULL) {
			printf ("error: input data null!\n");	
			return -1;
		}
  
		for(i = 0; i < size; i += 2) {
			y1 = yuv[2*i + 1];
			y2 = yuv[2*i + 3];		
			nv12[i] = y1;
			nv12[i+1] = y2;

			if (0 == (i%2)) {
				v = yuv[2*i];
				u = yuv[2*i + 2];
				nv12[size+(i>>1)] = u;
				nv12[size+(i>>1)+1] = v;		
			}			
		}
	
		return 0;
}
double what_time_is_it_now()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}


unsigned char check_sum(unsigned char *buff, int length)
{
	unsigned int tmp = 0;
	int i = 0;

	for(i = 0; i < length; i++) {
		tmp += buff[i];
	}

	return (tmp&0xff);
}


unsigned char check_sum_xor(unsigned char *buff, int length)
{
	unsigned char tmp = 0;
	int i = 0;

	for(i = 0; i < length; i++) {
		tmp ^= buff[i];
	}

	return tmp;
}
