/*
 * File      : mpp_decode.c
 * This file is MPP decode file  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-20     fengyong    first version
 */

#include <stdio.h>
#include <stdlib.h>
#if 1
#include "types_def.h"
#include "mpp_decode.h"
#include "mpp_common.h"
#include "video_common.h"
#include "xutils.h"
#include "mpp_mem.h"
#include "mpp_packet.h"
#include "mpp_buffer.h"
#include "rk_mpi.h"
#include "rk_type.h"

#include "log/log.h"


static double t0,t1;
static MpiDecCmd  *gs_mpi_dec_cmd = NULL;
static MpiDecLoopData *gs_mpi_dec_loop_data = NULL;

enum mpp_decode_e {
    MPP_DECODE_PACKET_BUF_SIZE_MAX = 0x200000,   /* 2MB */    			
};


MppBuffer frm_buf   = NULL;

#ifdef __cplusplus
extern "C" {
#endif
void decode_yuv_to_rb(MppFrame frame);
#ifdef __cplusplus
}
#endif

static void *decode_frame_output(void *arg);

int decode_frame_init(void)
{   
    int ret = RET_OK; 

    size_t packet_size  = 0;
    size_t file_size    = 0;

    MpiDecCmd  *cmd= NULL;
    MpiDecLoopData *data = NULL;
    MpiCmd mpi_cmd      = MPP_CMD_BASE;
    MppParam param      = NULL;
    RK_U32 need_split   = 1;
    pthread_t thd_out;

    cmd = (MpiDecCmd  *)malloc(sizeof(*cmd));
    if (!cmd) {
        printf("XF:mpi_dec_cmd_ctx malloc failed\n");
        return RET_ERR_MALLOC;
    }

    data = (MpiDecLoopData  *)malloc(sizeof(*data));
    if (!data) {
        printf("XF:mpi_dec_loop_data malloc failed\n");
        return RET_ERR_MALLOC;
    }
    
    /* MpiDecCmd init */
    memset((void*)cmd, 0, sizeof(*cmd));
    cmd->format = MPP_FMT_YUV420SP;
    cmd->pkt_size = MPI_DEC_STREAM_SIZE;
        
    cmd->width = VIDEO_FRAME_WIDTH;
    cmd->height = VIDEO_FRAME_HEIGHT;
        
    cmd->type = MPP_VIDEO_CodingMJPEG; /* H.264 */

    //cmd->debug = 1;
    mpp_env_set_u32("mpi_debug", cmd->debug);

    cmd->simple = (cmd->type != MPP_VIDEO_CodingMJPEG) ? (1) : (0);

    /* MpiDecLoopData init */
    memset(data, 0, sizeof(*data));

    if (cmd->have_input) {
        data->fp_input = fopen(cmd->file_input, "rb");
        if (NULL == data->fp_input) {
            SLOGI("failed to open input file %s\n", cmd->file_input);
            goto ERR_OUT;
        }

        fseek(data->fp_input, 0L, SEEK_END);
        file_size = ftell(data->fp_input);
        rewind(data->fp_input);
        mpp_log("input file size %ld\n", file_size);
    }

    if (cmd->have_output) {
        data->fp_output = fopen(cmd->file_output, "w+b");
        if (NULL == data->fp_output) {
            SLOGI("failed to open output file %s\n", cmd->file_output);
            goto ERR_OUT;
        }
    }

    packet_size  = MPP_DECODE_PACKET_BUF_SIZE_MAX*16;

    if (cmd->simple) {
        data->buf = mpp_malloc(char, packet_size);
        if (NULL == data->buf) {
            SLOGI("mpi_dec malloc input stream buffer failed\n");
            goto ERR_OUT;
        }

        ret = mpp_packet_init(&data->packet, data->buf, packet_size);
        if (ret) {
            SLOGI("mpp_packet_init failed\n");
            goto ERR_OUT;
        }
    } else {
      //
        RK_U32 hor_stride = MPP_ALIGN(cmd->width, 16);
        RK_U32 ver_stride = MPP_ALIGN(cmd->height, 16);

        ret = mpp_buffer_group_get_internal(&(data->frm_grp), MPP_BUFFER_TYPE_ION);
        if (ret) {
            mpp_err("failed to get buffer group for input frame ret %d\n", ret);
            goto ERR_OUT;
        }

        ret = mpp_frame_init(&(data->frame)); /* output frame */
        if (ret) {
            mpp_err("mpp_frame_init failed\n");
            goto ERR_OUT;
        }

        /*
         * NOTE: For jpeg could have YUV420 and YUV422 the buffer should be
         * larger for output. And the buffer dimension should align to 16.
         * YUV420 buffer is 3/2 times of w*h.
         * YUV422 buffer is 2 times of w*h.
         * So create larger buffer with 2 times w*h.
         */
        ret = mpp_buffer_get(data->frm_grp, &frm_buf, hor_stride * ver_stride * 4);
        if (ret) {
            mpp_err("failed to get buffer for input frame ret %d\n", ret);
            goto ERR_OUT;
        }

        mpp_frame_set_buffer((data->frame), frm_buf);
      //
    }

    ret = mpp_create(&data->ctx, &data->mpi);
    if (MPP_OK != ret) {
        SLOGI("mpp_create failed\n");
        goto ERR_OUT;
    }

    //NOTE: decoder split mode need to be set before init
    // mpi_cmd = MPP_DEC_SET_PARSER_SPLIT_MODE;
    // param = &need_split;
    // ret
    
    
     data->mpi->control(data->ctx, mpi_cmd, param);
    // if (MPP_OK != ret) {
    //     SLOGI("mpi->control failed\n");
    //     goto ERR_OUT;
    // }

    cmd->timeout = -1; 

    // NOTE: timeout value please refer to MppPollType definition
    //  0   - non-block call (default)
    // -1   - block call
    // +val - timeout value in ms
    if (cmd->timeout) {
        param = &cmd->timeout;
        ret = data->mpi->control(data->ctx, MPP_SET_OUTPUT_TIMEOUT, param);
        if (MPP_OK != ret) {
            SLOGI("Failed to set output timeout %d ret %d\n", cmd->timeout, ret);
            goto ERR_OUT;
        }
    }

    ret = mpp_init(data->ctx, MPP_CTX_DEC, cmd->type);
    if (MPP_OK != ret) {
        SLOGI("mpp_init failed\n");
        goto ERR_OUT;
    }

    data->frame_num      = cmd->frame_num;
    data->frame_count = 0;

    gs_mpi_dec_cmd = cmd;
    gs_mpi_dec_loop_data = data;   

     if (1)
     {
        ret = pthread_create(&thd_out, NULL, decode_frame_output, data);
        if (ret) 
        {
            SLOGI("failed to create thread for decode frame output ret %d\n", ret);
            goto ERR_OUT;
        }
     }
     else
     {
        // if (MPP_FRAME_FMT_IS_YUV(cmd->format) || MPP_FRAME_FMT_IS_RGB(cmd->format)) 
        // {
        //     MPP_RET ret = data->mpi->control(data->ctx, MPP_DEC_SET_OUTPUT_FORMAT, &cmd->format);
        //     if (ret)
        //     {
        //         mpp_err("Failed to set output format %d\n", cmd->format);
        //         return NULL;
        //     }
        // }

        // while (!data->loop_end)
        //     dec_advanced(data);

        // ret = pthread_create(&thd_out, NULL, decode_frame_output, data);
        // if (ret) 
        // {
        //     SLOGI("failed to create thread for decode frame output ret %d\n", ret);
        //     goto ERR_OUT;
        // }
     }
    

    return RET_OK;

ERR_OUT:
    if (data->packet) {
        mpp_packet_deinit(&data->packet);
        data->packet = NULL;
    }

    if (data->frame) {
        mpp_frame_deinit(&data->frame);
        data->frame = NULL;
    }

    if (data->ctx) {
        mpp_destroy(data->ctx);
        data->ctx = NULL;
    }

    if (cmd->simple) {
        if (data->buf) {
            mpp_free(data->buf);
            data->buf = NULL;
        }
    } else {
       //
    }

    if (data->pkt_grp) {
        mpp_buffer_group_put(data->pkt_grp);
        data->pkt_grp = NULL;
    }

    if (data->frm_grp) {
        mpp_buffer_group_put(data->frm_grp);
        data->frm_grp = NULL;
    }

    if (data->fp_output) {
        fclose(data->fp_output);
        data->fp_output = NULL;
    }

    if (data->fp_input) {
        fclose(data->fp_input);
        data->fp_input = NULL;
    }

    return ret;
}

void decode_frame_deinit(void)
{
    MpiDecCmd  *cmd= gs_mpi_dec_cmd;
    MpiDecLoopData *data = gs_mpi_dec_loop_data;

    if (cmd && data) {
        if (data->packet) {
            mpp_packet_deinit(&data->packet);
            data->packet = NULL;
        }

        if (data->frame) {
            mpp_frame_deinit(&data->frame);
            data->frame = NULL;
        }

        if (data->ctx) {
            mpp_destroy(data->ctx);
            data->ctx = NULL;
        }

        if (cmd->simple) {
            if (data->buf) {
                mpp_free(data->buf);
                data->buf = NULL;
            }
        } else {
            //
        }

        if (data->pkt_grp) {
            mpp_buffer_group_put(data->pkt_grp);
            data->pkt_grp = NULL;
        }

        if (data->frm_grp) {
            mpp_buffer_group_put(data->frm_grp);
            data->frm_grp = NULL;
        }

        if (data->fp_output) {
            fclose(data->fp_output);
            data->fp_output = NULL;
        }

        if (data->fp_input) {
            fclose(data->fp_input);
            data->fp_input = NULL;
        }

        free(cmd);
        free(data);

        cmd = NULL;
        data = NULL;
    }    
}

MpiDecLoopData *mpi_dec_loop_data_get(void)
{
    return gs_mpi_dec_loop_data;
}

static struct timespec pre_packet_tm, cur_packet_tm;
static uint32_t gs_decode_packet_count = 0;

void decode_packet_input(uint8_t *buf_data, int size, uint32_t ts)
{
    SLOGI("oh! put data\n");

    MpiDecLoopData *data = mpi_dec_loop_data_get();
    MppCtx ctx  = data->ctx;
    MppApi *mpi = data->mpi;
    char   *buf = data->buf;
    MppPacket packet = data->packet;
    MPP_RET ret = MPP_OK;
    RK_U32 pkt_done = 0;
    RK_U32 pkt_eos  = 0;
    uint32_t diff_tm = 0;

    void *tmp_pos = NULL;
    void *tmp_data = NULL;

#if DATA_ENGINE_PERFORMANCE_ANALYSIS
    struct timespec start_tm,end_tm;
    clock_gettime(CLOCK_REALTIME, &start_tm);
    SLOGI("%s size = %d, ts = %u\n", __FUNCTION__, size, ts);
#endif

#if 0 /* Fixed */
    clock_gettime(CLOCK_REALTIME, &cur_packet_tm);
    if (gs_decode_packet_count > 50) {
        diff_tm = diff_timespec(&cur_packet_tm,&pre_packet_tm);
        if (diff_tm < 1800000) {
            SLOGI("%s discard:%u\n", __FUNCTION__, diff_tm);
            return;
        }
    } 
#endif 
    
#if 1
    memcpy(buf,buf_data,size);

    // write data to packet
    //mpp_packet_write(packet, 0, buf, size);

    // reset pos and set valid length
    mpp_packet_set_pos(packet, buf);
    mpp_packet_set_length(packet, size);
#else
    ret = mpp_packet_init(&packet, (void *)buf_data, size);
    mpp_packet_set_pts(packet, ts);
#endif

    gs_decode_packet_count++;
    memcpy(&pre_packet_tm,&cur_packet_tm,sizeof(struct timespec));

    // setup eos flag
    if (pkt_eos) {
        mpp_packet_set_eos(packet);
        // mpp_log("found last packet\n");
    } else {
        mpp_packet_clr_eos(packet);
    }
        
    // send packet until it success
    do {
        ret = mpi->decode_put_packet(ctx, packet);
        if (MPP_OK == ret)
        {
            pkt_done = 1;
            SLOGI("oh! put done\n");
        }
        else {
            // if failed wait a moment and retry
            SLOGI("NONONO! put fail\n");
            msleep(5);
        }
    } while (!pkt_done);

#if DATA_ENGINE_PERFORMANCE_ANALYSIS
    clock_gettime(CLOCK_REALTIME, &end_tm);
    SLOGI("%s size = %d packet send finished diff tm:%u\n", __FUNCTION__,size,diff_timespec(&end_tm,&start_tm));
#endif
    
}

static void *decode_frame_output(void *arg)
{
    MpiDecLoopData *data = (MpiDecLoopData *)arg;
    MppCtx ctx  = data->ctx;
    MppApi *mpi = data->mpi;
    MppFrame  frame  = NULL;

    SLOGI("get decode frame thread start\n");

    // then get all available frame and release
    do {
        RK_S32 times = 5;
        MPP_RET ret = MPP_OK;

    GET_AGAIN:        
        ret = mpi->decode_get_frame(ctx, &frame);
        if (MPP_ERR_TIMEOUT == ret) {
            if (times > 0) {
                times--;
                msleep(2);
                goto GET_AGAIN;
            }
            SLOGI("decode_get_frame failed too much time\n");
        }
        if (MPP_OK != ret) {
            mpp_err("decode_get_frame failed ret %d\n", ret);
            continue;
        }
#if DATA_ENGINE_PERFORMANCE_ANALYSIS        
        SLOGI("%s get decode frame\n", __FUNCTION__);
#endif        

SLOGI("___if frame____\n");
        if (frame) {
            if (mpp_frame_get_info_change(frame)) {
                // found info change and create buffer group for decoding
                RK_U32 width = mpp_frame_get_width(frame);
                RK_U32 height = mpp_frame_get_height(frame);
                RK_U32 hor_stride = mpp_frame_get_hor_stride(frame);
                RK_U32 ver_stride = mpp_frame_get_ver_stride(frame);
                RK_U32 buf_size = mpp_frame_get_buf_size(frame);

                SLOGI("decode_get_frame get info changed found\n");
                SLOGI("decoder require buffer w:h [%d:%d] stride [%d:%d] size %d\n",
                        width, height, hor_stride, ver_stride, buf_size);

                if (NULL == data->frm_grp) {
                    SLOGI("decode_anl_frame1\n");
                    /* If buffer group is not set create one and limit it */
                    ret = mpp_buffer_group_get_internal(&data->frm_grp, MPP_BUFFER_TYPE_ION);
                    if (ret) {
                        mpp_err("get mpp buffer group failed ret %d\n", ret);
                        SLOGI("decode_anl_frame err1\n");
                        break;
                    }


                    SLOGI("decode_anl_frame2\n");
                    /* Set buffer to mpp decoder */
                    ret = mpi->control(ctx, MPP_DEC_SET_EXT_BUF_GROUP, data->frm_grp);
                    if (ret) {
                        mpp_err("set buffer group failed ret %d\n", ret);
                        break;
                    }
                } else {
                    /* If old buffer group exist clear it */
                    ret = mpp_buffer_group_clear(data->frm_grp);
                    if (ret) {
                        mpp_err("clear buffer group failed ret %d\n", ret);
                        break;
                    }
                }

                /* Use limit config to limit buffer count to 24 */
                ret = mpp_buffer_group_limit_config(data->frm_grp, buf_size, 24);
                if (ret) {
                    mpp_err("limit buffer group failed ret %d\n", ret);
                    break;
                }

                ret = mpi->control(ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
                if (ret) {
                    mpp_err("info change ready failed ret %d\n", ret);
                    break;
                }
            } else {
                // found normal output frame
                RK_U32 err_info = mpp_frame_get_errinfo(frame) | mpp_frame_get_discard(frame);
                if (0/*err_info*/)
                    mpp_log("decoder_get_frame get err info:%d discard:%d.\n",
                            mpp_frame_get_errinfo(frame), mpp_frame_get_discard(frame));

                if (!err_info) {
                    SLOGI("---start decode_frame_to rb\n");
                    
                    decode_yuv_to_rb(frame);
                     //t1=what_time_is_it_now();
                    // if((t1-t0)*1000>66){
                     //    SLOGI("decode_yuv_to_rb frame   %fms\n",(t1-t0)*1000);
                    // }
                    
                     //t0=what_time_is_it_now();
                }
               
                data->frame_count++;
            }

            if (mpp_frame_get_eos(frame)) {
                // mpp_log("found last frame\n");
                // when get a eos status mpp need a reset to restart decoding
                ret = mpi->reset(ctx);
                if (MPP_OK != ret)
                    mpp_err("mpi->reset failed\n");
            }

            mpp_frame_deinit(&frame);
            frame = NULL;
        }
    } while (!data->loop_end);

    printf("get decode frame thread end\n");

    return NULL;
}

#if 0
static void *decode_frame_output2(void *arg)
{
    MpiDecLoopData *data = (MpiDecLoopData *)arg;
    MppCtx ctx  = data->ctx;
    MppApi *mpi = data->mpi;
    MppFrame  frame  = NULL;

    MPP_RET ret = 0;
    SLOGI("get decode frame thread start\n");

    ret = mpi->poll(ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK);
    if (ret) {
        mpp_err("%p mpp input poll failed\n", ctx);
        return ret;
    }

    ret = mpi->dequeue(ctx, MPP_PORT_INPUT, &task);  /* input queue */
    if (ret) {
        mpp_err("%p mpp task input dequeue failed\n", ctx);
        return ret;
    }

    mpp_assert(task);

    mpp_task_meta_set_packet(task, KEY_INPUT_PACKET, packet);
    mpp_task_meta_set_frame (task, KEY_OUTPUT_FRAME,  frame);

    ret = mpi->enqueue(ctx, MPP_PORT_INPUT, task);  /* input queue */
    if (ret) {
        mpp_err("%p mpp task input enqueue failed\n", ctx);
        return ret;
    }

    if (!data->first_pkt)
        data->first_pkt = mpp_time();

    /* poll and wait here */
    ret = mpi->poll(ctx, MPP_PORT_OUTPUT, MPP_POLL_BLOCK);
    if (ret) {
        mpp_err("%p mpp output poll failed\n", ctx);
        return ret;
    }

    ret = mpi->dequeue(ctx, MPP_PORT_OUTPUT, &task); /* output queue */
    if (ret) {
        mpp_err("%p mpp task output dequeue failed\n", ctx);
        return ret;
    }

    mpp_assert(task);

    if (task) {
        MppFrame frame_out = NULL;

        mpp_task_meta_get_frame(task, KEY_OUTPUT_FRAME, &frame_out);

        if (frame) {
            if (!data->first_frm)
                data->first_frm = mpp_time();

            /* write frame to file here */
            if (data->fp_output)
                dump_mpp_frame_to_file(frame, data->fp_output);

            if (data->fp_verify) {
                calc_frm_crc(frame, checkcrc);
                write_frm_crc(data->fp_verify, checkcrc);
            }

            mpp_log_q(quiet, "%p decoded frame %d\n", ctx, data->frame_count);
            data->frame_count++;

            if (mpp_frame_get_eos(frame_out)) {
                mpp_log_q(quiet, "%p found eos frame\n", ctx);
            }
            fps_calc_inc(cmd->fps);
        }

        if (data->frame_num > 0) {
            if (data->frame_count >= data->frame_num)
                data->loop_end = 1;
        } else if (data->frame_num == 0) {
            if (slot->eos)
                data->loop_end = 1;
        }

        /* output queue */
        ret = mpi->enqueue(ctx, MPP_PORT_OUTPUT, task);
        if (ret)
            mpp_err("%p mpp task output enqueue failed\n", ctx);
    }

    /*
     * The following input port task dequeue and enqueue is to make sure that
     * the input packet can be released. We can directly deinit the input packet
     * after frame output in most cases.
     */
    if (0) {
        mpp_packet_deinit(&packet);
    } else {
        ret = mpi->dequeue(ctx, MPP_PORT_INPUT, &task);  /* input queue */
        if (ret) {
            mpp_err("%p mpp task input dequeue failed\n", ctx);
            return ret;
        }

        mpp_assert(task);
        if (task) {
            MppPacket packet_out = NULL;

            mpp_task_meta_get_packet(task, KEY_INPUT_PACKET, &packet_out);

            if (!packet_out || packet_out != packet)
                mpp_err_f("mismatch packet %p -> %p\n", packet, packet_out);

            mpp_packet_deinit(&packet_out);

            /* input empty task back to mpp to maintain task status */
            ret = mpi->enqueue(ctx, MPP_PORT_INPUT, task);
            if (ret)
                mpp_err("%p mpp task input enqueue failed\n", ctx);
        }
    }

    return NULL;
}
#endif

#if 0
int decode_frame_handle(uint8_t *buf_data, int size, uint32_t ts)
{
    RK_U32 pkt_done = 0;
    RK_U32 pkt_eos  = 0;
    RK_U32 err_info = 0;
    MPP_RET ret = MPP_OK;
    MpiDecLoopData *data = mpi_dec_loop_data_get();
    MppCtx ctx  = data->ctx;
    MppApi *mpi = data->mpi;
    //char   *buf = av_packet->data;
    MppPacket packet = NULL; //data->packet;
    MppFrame  frame  = NULL;
    
    size_t packet_size = data->packet_size;
    
    ret = mpp_packet_init(&packet, (void *)buf_data, size);
    mpp_packet_set_pts(packet, ts);

    SLOGI("%s size = %d, ts = %u\r\n", __FUNCTION__, size, ts);

#if 0
    // write data to packet
    mpp_packet_write(packet, 0, av_packet->data, av_packet->size);
    // reset pos and set valid length
    mpp_packet_set_pos(packet, av_packet->data);
    mpp_packet_set_length(packet, av_packet->size);   

    mpp_packet_set_pts(packet, av_packet->pts); 
#endif        

    do {
        RK_S32 times = 5;
        // send the packet first if packet is not done
        if (!pkt_done) {
            ret = mpi->decode_put_packet(ctx, packet);
            if (MPP_OK == ret)
                pkt_done = 1;
        }
        
        // then get all available frame and release
        do {
            RK_S32 get_frm = 0;
            RK_U32 frm_eos = 0;

        try_again:
            ret = mpi->decode_get_frame(ctx, &frame);
            if (MPP_ERR_TIMEOUT == ret) {
                if (times > 0) {
                    times--;
                    msleep(2);
                    goto try_again;
                }
                mpp_err("decode_get_frame failed too much time\n");
            }
            if (MPP_OK != ret) {
                mpp_err("decode_get_frame failed ret %d\n", ret);
                break;
            }
            
            if (frame) {
                if (mpp_frame_get_info_change(frame)) {
                    RK_U32 width = mpp_frame_get_width(frame);
                    RK_U32 height = mpp_frame_get_height(frame);
                    RK_U32 hor_stride = mpp_frame_get_hor_stride(frame);
                    RK_U32 ver_stride = mpp_frame_get_ver_stride(frame);
                    RK_U32 buf_size = mpp_frame_get_buf_size(frame);

                    mpp_log("decode_get_frame get info changed found\n");
                    mpp_log("decoder require buffer w:h [%d:%d] stride [%d:%d] buf_size %d",
                            width, height, hor_stride, ver_stride, buf_size);

                    /*
                     * NOTE: We can choose decoder's buffer mode here.
                     * There are three mode that decoder can support:
                     *
                     * Mode 1: Pure internal mode
                     * In the mode user will NOT call MPP_DEC_SET_EXT_BUF_GROUP
                     * control to decoder. Only call MPP_DEC_SET_INFO_CHANGE_READY
                     * to let decoder go on. Then decoder will use create buffer
                     * internally and user need to release each frame they get.
                     *
                     * Advantage:
                     * Easy to use and get a demo quickly
                     * Disadvantage:
                     * 1. The buffer from decoder may not be return before
                     * decoder is close. So memroy leak or crash may happen.
                     * 2. The decoder memory usage can not be control. Decoder
                     * is on a free-to-run status and consume all memory it can
                     * get.
                     * 3. Difficult to implement zero-copy display path.
                     *
                     * Mode 2: Half internal mode
                     * This is the mode current test code using. User need to
                     * create MppBufferGroup according to the returned info
                     * change MppFrame. User can use mpp_buffer_group_limit_config
                     * function to limit decoder memory usage.
                     *
                     * Advantage:
                     * 1. Easy to use
                     * 2. User can release MppBufferGroup after decoder is closed.
                     *    So memory can stay longer safely.
                     * 3. Can limit the memory usage by mpp_buffer_group_limit_config
                     * Disadvantage:
                     * 1. The buffer limitation is still not accurate. Memory usage
                     * is 100% fixed.
                     * 2. Also difficult to implement zero-copy display path.
                     *
                     * Mode 3: Pure external mode
                     * In this mode use need to create empty MppBufferGroup and
                     * import memory from external allocator by file handle.
                     * On Android surfaceflinger will create buffer. Then
                     * mediaserver get the file handle from surfaceflinger and
                     * commit to decoder's MppBufferGroup.
                     *
                     * Advantage:
                     * 1. Most efficient way for zero-copy display
                     * Disadvantage:
                     * 1. Difficult to learn and use.
                     * 2. Player work flow may limit this usage.
                     * 3. May need a external parser to get the correct buffer
                     * size for the external allocator.
                     *
                     * The required buffer size caculation:
                     * hor_stride * ver_stride * 3 / 2 for pixel data
                     * hor_stride * ver_stride / 2 for extra info
                     * Total hor_stride * ver_stride * 2 will be enough.
                     *
                     * For H.264/H.265 20+ buffers will be enough.
                     * For other codec 10 buffers will be enough.
                     */

                    if (NULL == data->frm_grp) {
                        /* If buffer group is not set create one and limit it */
                        ret = mpp_buffer_group_get_internal(&data->frm_grp, MPP_BUFFER_TYPE_ION);
                        if (ret) {
                            mpp_err("get mpp buffer group failed ret %d\n", ret);
                            break;
                        }

                        /* Set buffer to mpp decoder */
                        ret = mpi->control(ctx, MPP_DEC_SET_EXT_BUF_GROUP, data->frm_grp);
                        if (ret) {
                            mpp_err("set buffer group failed ret %d\n", ret);
                            break;
                        }
                    } else {
                        /* If old buffer group exist clear it */
                        ret = mpp_buffer_group_clear(data->frm_grp);
                        if (ret) {
                            mpp_err("clear buffer group failed ret %d\n", ret);
                            break;
                        }
                    }

                    /* Use limit config to limit buffer count to 24 with buf_size */
                    ret = mpp_buffer_group_limit_config(data->frm_grp, buf_size, 24);
                    if (ret) {
                        mpp_err("limit buffer group failed ret %d\n", ret);
                        break;
                    }

                    /*
                     * All buffer group config done. Set info change ready to let
                     * decoder continue decoding
                     */
                    ret = mpi->control(ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
                    if (ret) {
                        mpp_err("info change ready failed ret %d\n", ret);
                        break;
                    }
                } else {
                    err_info = mpp_frame_get_errinfo(frame) | mpp_frame_get_discard(frame);
                    if (err_info) {
                        mpp_log("decoder_get_frame get err info:%d discard:%d.\n",
                                mpp_frame_get_errinfo(frame), mpp_frame_get_discard(frame));
                    }
                    data->frame_count++;
                    //mpp_log("decode_get_frame get frame %d\n", data->frame_count);
                    //printf("+++decode_get_frame get frame %d\n", data->frame_count);
                    SLOGI("+++decode_get_frame get frame %d\n", data->frame_count);

                    //if (data->fp_output && !err_info)
                    //    dump_mpp_frame_to_file(frame, data->fp_output);

                    if (!err_info) {
                        decode_yuv_to_rb(frame);
                    }
                }
                frm_eos = mpp_frame_get_eos(frame);
                mpp_frame_deinit(&frame);
                frame = NULL;
                get_frm = 1;
            }

            // try get runtime frame memory usage
            if (data->frm_grp) {
                size_t usage = mpp_buffer_group_usage(data->frm_grp);
                if (usage > data->max_usage)
                    data->max_usage = usage;
            }

            // if last packet is send but last frame is not found continue
            if (pkt_eos && pkt_done && !frm_eos) {
                msleep(10);
                continue;
            }

            if (frm_eos) {
                mpp_log("found last frame\n");
                break;
            }

            if (data->frame_num > 0 && data->frame_count >= data->frame_num) {
                data->eos = 1;
                break;
            }

            if (get_frm)
                continue;
            break;
        } while (1);

        if (data->frame_num > 0 && data->frame_count >= data->frame_num) {
            data->eos = 1;
            mpp_log("reach max frame number %d\n", data->frame_count);
            break;
        }

        if (pkt_done)
            break;

        /*
         * why sleep here:
         * mpi->decode_put_packet will failed when packet in internal queue is
         * full,waiting the package is consumed .Usually hardware decode one
         * frame which resolution is 1080p needs 2 ms,so here we sleep 3ms
         * * is enough.
         */
        msleep(3);
    } while (1);
    
    mpp_packet_deinit(&packet);

    return ret;
}
#endif
#endif