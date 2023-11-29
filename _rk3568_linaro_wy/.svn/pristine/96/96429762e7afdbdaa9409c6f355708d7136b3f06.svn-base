/*
 * File      : socket_can_ops.h
 * This file is socket can operator interface header 
 * COPYRIGHT (C) 2020 zmvision
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-7-21     fengyong    first version
 */

#ifndef _SOCKET_CAN_OPS_H_
#define _SOCKET_CAN_OPS_H_

#include <pthread.h>
#include "types_def.h"
#include "raw_can.h"
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif

enum can_data_stream_e {		
	DATA_LEN_COUNT = 8,		
	ERROR_FRAME_BUF_SIZE = 256,		
};

typedef struct {   
    int port;
	int send_fd;		
	int recv_fd;
    int can_frame_len;    
    uint32_t bitrate;
    uint32_t recv_frame_count;
    S_CanFrame send_frame;
    S_CanFrame recv_frame;
    char errframe_info_buf[ERROR_FRAME_BUF_SIZE];
}can_data_stream_t;

typedef union u_canframe_data
{
    uint8_t            data[DATA_LEN_COUNT];

    struct {
        uint32_t       dl;
        uint32_t       dh;
    }s;
} u_canframe_data_t;

typedef struct sys_error_frame
{
    uint32_t    id;
    char        *err_info;
} s_sys_error_frame_t;

typedef enum {
	CAN_SEND_TAKE_PHOTO = 1,
    CAN_SEND_TRACK = 2,
}
CAN_SEND_TYPE;

typedef struct
{
	pthread_mutex_t mutex;
	sem_t awake_sem;
	CAN_SEND_TYPE msg_type;
}sem_msg_hanle_t;

int can_data_stream_init(void);
void can_data_stream_destory(void); 
can_data_stream_t *can_data_stream_get(void);

void open_can(const int port);
void close_can(const int port);
//int socket_listen(const int port);
int recv_socket_connect(const int port);
int send_socket_connect(const int port);
void disconnect(int *sockfd);
void set_bitrate(const int port, const int bitrate);
int set_can_filter(int send_socket_fd, int recv_socket_fd,int can_id);
void set_send_frame_property(uint32_t send_frame_id, int extended_frame);
void set_send_frame_data(uint8_t *data,int dlc);

int send_frame(const int sockfd, const uint8_t* data, const int count);
int recv_frame(const int sockfd, uint8_t* buf, const int count, const int timeout_ms);

void logout_head(void);
void logout_frame(const uint32_t frame_id, unsigned char *data, const uint8_t len,
    const u_int8_t extended,
    const u_int8_t ok_flag,
    const u_int8_t sendflag);

int rt_priority_thread_create(pthread_t *thread, int priority,void *(*start_routine) (void *), void *arg);

void can_stop(void);
void can_up(void);
void can_raw_set_bitrate(unsigned int bitrate);
void can_restart(void);
extern short can_upward_id;
int spi_can_init(void);
int msg_send_handle_send(sem_msg_hanle_t *param,CAN_SEND_TYPE type);
extern sem_msg_hanle_t gs_can_send_msg;
#ifdef __cplusplus
}
#endif

#endif /* _SOCKET_CAN_OPS_H_ */

