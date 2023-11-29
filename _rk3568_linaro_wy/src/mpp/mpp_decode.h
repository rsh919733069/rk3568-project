/*
 * File      : mpp_decode.h
 * This file is RK3399PRO MPP decode interface header  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-18     fengyong    first version
 */

#ifndef ZM_MPPDECODE_H
#define ZM_MPPDECODE_H
#if 0
#define MODULE_TAG "mpi_dec"
#include <string.h>
#include "rk_mpi.h"
#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_env.h"
#include "mpp_time.h"
#include "mpp_common.h"

#include "mpp_frame.h"
#include "mpp_buffer_impl.h"
#include "mpp_frame_impl.h"
#endif

#include <string.h>

#include "rk_mpi.h"
#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_env.h"
#include "mpp_time.h"
#include "mpp_common.h"

//#include "mpp_utils.h"

#define MPI_DEC_LOOP_COUNT          4
#define MPI_DEC_STREAM_SIZE         (SZ_4K*4)
#define MAX_FILE_NAME_LENGTH        256

typedef struct {
    MppCtx          ctx;
    MppApi          *mpi;

    /* end of stream flag when set quit the loop */
    RK_U32          eos;

    /* buffer for stream data reading */
    char            *buf;

    /* input and output */
    MppBufferGroup  frm_grp;
    MppBufferGroup  pkt_grp;
    MppPacket       packet;
    size_t          packet_size;
    MppFrame        frame;

    FILE            *fp_input;
    FILE            *fp_output;    
    RK_S32          frame_count;
    RK_S32          frame_num;
    size_t          max_usage;

    volatile RK_U32 loop_end;
} MpiDecLoopData;

typedef struct {
    char            file_input[MAX_FILE_NAME_LENGTH];
    char            file_output[MAX_FILE_NAME_LENGTH];  
    
    MppCodingType   type;
    MppFrameFormat  format;
    RK_U32          width;
    RK_U32          height;
    RK_U32          debug;

    RK_U32          have_input;
    RK_U32          have_output;
    
    RK_U32          simple;
    RK_S32          timeout;
    RK_S32          frame_num;
    size_t          pkt_size;

    // report information
    size_t          max_usage;
} MpiDecCmd;



#ifdef __cplusplus
extern "C" {
#endif

int decode_frame_init(void);
void decode_frame_deinit(void);
MpiDecLoopData *mpi_dec_loop_data_get(void);
int decode_frame_handle(uint8_t *buf_data, int size, uint32_t ts);
void decode_packet_input(uint8_t *buf_data, int size, uint32_t ts);

#ifdef __cplusplus
}
#endif

#endif /* ZM_MPPDECODE_H */
