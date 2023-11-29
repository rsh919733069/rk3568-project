


#ifndef ZM_TS_H_
#define ZM_TS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <arpa/inet.h>
#include "queue.h"
typedef struct udp_ts_info{
    char udpts_ip[32];
    int udpts_port ;
    int ts_port;
    int udpts_frame_type;
    int udp_ts_sockfd;
    struct sockaddr_in udp_ts_send_addr;
    int open_udpts_flag;
}zm_ts_udp_info_t;

extern zm_ts_udp_info_t gs_ts_udp_info;
extern QueueStruct tsQueue;
#ifdef __cplusplus
}
#endif

#endif 
