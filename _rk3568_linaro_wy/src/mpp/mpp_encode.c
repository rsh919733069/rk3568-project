/*
 * File      : mpp_encode.c
 * This file is MPP encode file  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-20     fengyong    first version
 */

#include <stdio.h>
#include <stdlib.h>
#include "rk_mpi.h"

#include "mpp_env.h"
#include "mpp_mem.h"
#include "mpp_log.h"
#include "mpp_time.h"
#include "mpp_common.h"

// #include "utils.h"
// #include "mpi_enc_utils.h"

#include "vpu_api.h"

#include "types_def.h"
#include "mpp_encode.h"
#include "video_common.h"
#include "xutils.h"
#include "log/log.h"
#include "Tsmux.h"
#include "queue.h"
#include "rtsp_server.h"

#include "data_engine.h"

#include "zm_ts.h"
#include "rtsp_client.h"
#include "logger.h"


#define MPI_ENC_IO_COUNT            (4)
#define MAX_FILE_NAME_LENGTH        256

#define MPI_ENC_TEST_SET_IDR_FRAME  0

typedef struct {
    char            file_input[MAX_FILE_NAME_LENGTH];
    char            file_output[MAX_FILE_NAME_LENGTH];
    MppCodingType   type;
    RK_U32          width;
    RK_U32          height;

    RK_U32          hor_stride;
    RK_U32          ver_stride;

    MppFrameFormat  format;
    RK_U32          debug;
    RK_U32          num_frames;
    RK_U32          target_bps;

    RK_U32          have_input;
    RK_U32          have_output;

    RK_U32          payload_cnts;
} MpiEncCmd;

typedef struct {
     // global flow control flag
    RK_U32 frm_eos;
    RK_U32 pkt_eos;
    RK_U32 frame_count;
    RK_U64 stream_size;

    // src and dst
    FILE *fp_input;
    FILE *fp_output;

    // base flow context
    MppCtx ctx;
    MppApi *mpi;
    MppEncPrepCfg prep_cfg;
    MppEncRcCfg rc_cfg;
    MppEncCodecCfg codec_cfg;

    // input / output
    MppBufferGroup  frm_grp;
    MppBufferGroup  pkt_grp;
    MppBufferGroup  buf_grp;
    MppFrame        frame;
    MppPacket       packet;
    MppBuffer       frm_buf[MPI_ENC_IO_COUNT];
    MppBuffer       pkt_buf[MPI_ENC_IO_COUNT];
    MppBuffer       md_buf[MPI_ENC_IO_COUNT];
    MppEncSeiMode   sei_mode;

    // paramter for resource malloc
    RK_U32 width;
    RK_U32 height;
    RK_U32 hor_stride;
    RK_U32 ver_stride;
    MppFrameFormat fmt;
    MppCodingType type;
    RK_U32 num_frames;

    // resources
    size_t frame_size;
    /* NOTE: packet buffer may overflow */
    size_t packet_size;
    /* 32bits for each 16x16 block */
    size_t mdinfo_size;
    /* osd idx size range from 16x16 bytes(pixels) to hor_stride*ver_stride(bytes). for general use, 1/8 Y buffer is enough. */
    size_t osd_idx_size;
    RK_U32 plt_table[8];

    // rate control runtime parameter
    RK_S32 gop;
    RK_S32 fps;
    RK_S32 bps;
    RK_S32 qp_min;
    RK_S32 qp_max;
    RK_S32 qp_step;
    RK_S32 qp_init;
} MpiEncData;



static  MpiEncCmd  gs_cmd_ctx;
static  MpiEncCmd  gs_cmd_ctx1;
static  MpiEncCmd  gs_cmd_ctx2;
static  MpiEncData *gs_enc_data = NULL;
static  MpiEncData *gs_enc_data1 = NULL;
static  MpiEncData *gs_enc_data2 = NULL;
#ifdef __cplusplus
extern "C" {
#endif
extern int  video_data_wait_and_get(uint8_t *buf, uint32_t width, uint32_t height,uint32_t hor_stride, uint32_t ver_stride,uint32_t frame_conut);
extern int  encode_data_wait_and_get(uint8_t *buf, uint32_t width, uint32_t height,
                   uint32_t hor_stride, uint32_t ver_stride);
extern int  jpeg_data_wait_and_get(uint8_t *buf, uint32_t width, uint32_t height,
                   uint32_t hor_stride, uint32_t ver_stride);
extern void rtsp_server_push_video(int ch_index,uint8_t *frame_buf, int frame_size);

extern int af_save_jpeg(char *file_path_name, unsigned char *frame_buf,\
    unsigned int frame_len, unsigned int width, unsigned int height);
#ifdef __cplusplus
}
#endif

 
unsigned char klv_buf2[188] = {
    0x47,0x40,0x42,0x31,0x37,0x10,0x09,0xa7,0xdc,0x74,0x7e,0x00,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x01,0xbd,
    0x00,0x7a,0x81,0x80,0x05,0x21,0x4d,0x3f,0xc9,0xb5,0x06,0x0e,0x2b,0x34,0x02,0x0b,
    0x01,0x01,0x0e,0x01,0x03,0x01,0x01,0x00,0x00,0x00,0x61,0x02,0x08,0x00,0x05,0xde,
    0xb9,0x51,0xf7,0x7d,0x4c,0x03,0x09,0x4d,0x49,0x53,0x53,0x49,0x4f,0x4e,0x30,0x31,
    0x04,0x06,0x41,0x46,0x2d,0x31,0x30,0x31,0x05,0x02,0x71,0xc2,0x06,0x02,0xfd,0x3d,
    0x07,0x02,0x08,0xb8,0x08,0x01,0x93,0x09,0x01,0x9f,0x0a,0x0d,0x5a,0x6d,0x76,0x73,
    0x69,0x6f,0x6e,0x31,0x32,0x33,0x34,0x35,0x36,0x0b,0x02,0x45,0x4f,0x0c,0x06,0x57,
    0x47,0x53,0x2d,0x38,0x34,0x0d,0x04,0x55,0x95,0xb6,0x6d,0x0e,0x04,0x5b,0x53,0x60,
    0xc4,0x0f,0x02,0xc2,0x21,0x41,0x01,0x0e,0x01,0x02,0x38,0xb3,
};//188-114

const unsigned char buf_pat[188] = {
    0x47,0x40,0x00,0x35,0xa6,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0xb0,0x0d,0x00,
    0x01,0xc1,0x00,0x00,0x00,0x01,0xe0,0x20,0xa2,0xc3,0x29,0x41,
};
const unsigned char buf_pmt[188] = {
    0x47,0x40,0x20,0x35,0x80,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0x00,0x02,0xb0,0x33,0x00,0x01,0xc1,0x00,0x00,0xe0,0x42,
    0xf0,0x0c,0x05,0x04,0x48,0x44,0x4d,0x56,0x88,0x04,0x0f,0xff,0xfc,0xfc,0x1b,0xe0,
    0x44,0xf0,0x0a,0x05,0x08,0x48,0x44,0x4d,0x56,0xff,0x1b,0x44,0x3f,0x06,0xe0,0x42,
    0xf0,0x06,0x05,0x04,0x4b,0x4c,0x56,0x41,0x68,0x6a,0x2d,0x36,
};

int injectKLV(int klv_num,int size1, char* buf, char* new_buf){
    //load_bin();
    //static int klv_num=0;
    //klv_num++;
    if (klv_num==16) klv_num=0;


    int ts_pck_len = size1 / 188;
    for(int i = 0; i < ts_pck_len; i++){
      __uint8_t* temp = (__uint8_t*)buf+i*188;

      // PMT
      if(temp[1]==0x4F && temp[2]==0xFF){
        memcpy(temp+4, buf_pmt+4, 184);
        memcpy(temp, buf_pmt, 3);
        temp[3] = temp[3] | (0b00100000);
        // char crctemp[] = {0x68, 0x6a, 0x2d, 0x36};
        // memcpy(temp+184, crctemp, 4);
      }

      // PAT
      if(temp[1]==0x40 && temp[2]==0x00){
        memcpy(temp+4, buf_pat+4, 184); //相当于只要他们pat的continuity counter
        memcpy(temp, buf_pat, 3);
        temp[3] = temp[3] | (0b00100000);
      }

      // video frame edit the klv pcr and pts
      if(temp[15]==0xE0 && temp[14]==0x01){
        //cout << i << ' ';

        memcpy(klv_buf2+69, temp+21, 5);
        klv_buf2[69] = klv_buf2[69] & (0b11101111); // 把视频帧pts去掉dts_flag

        memcpy(klv_buf2+5, temp+5, 7); // pcr

        klv_buf2[3] = (0b00110000) + klv_num;       
      }
    }

    int sdt_num = 0;
    for(int i = 0; i < ts_pck_len; i++){
      __uint8_t* temp = (__uint8_t*)buf+i*188;
      // SDT/BAT 换 KLV
      if(temp[1]==0x40 && temp[2]==0x11){
        memcpy(temp, klv_buf2, 188);
        sdt_num++;
        break;
      }
    
    }
    //cout << sdt_num << endl;
    //如果没有SDT/BAT 追加一个KLV
    if(!sdt_num){
    //   size1+=188;
    //   memcpy(new_buf, buf, size1-188);
    //   memcpy(new_buf+size1-188, klv_buf2, 188);
     // WriteBinData("frame_test.bin", new_buf, size1);
     memcpy(new_buf, buf, size1);
      return size1;
    } else {
      memcpy(new_buf, buf, size1);
     // WriteBinData("frame_test.bin", new_buf, size1);
      return size1;
    }

}

#define UUID_SIZE 16
 
//FFMPEG uuid
//static unsigned char uuid[] = { 0xdc, 0x45, 0xe9, 0xbd, 0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef };
//self UUID
static unsigned char uuid[] = { 0x54, 0x80, 0x83, 0x97, 0xf0, 0x23, 0x47, 0x4b, 0xb7, 0xf7, 0x4f, 0x32, 0xb5, 0x4e, 0x06, 0xac };
 
//开始码
static unsigned char start_code[] = {0x00,0x00,0x00,0x01};
 
static uint32_t reversebytes(uint32_t value) {
	return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
		(value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}


// int fill_sei_packet(unsigned char * packet,int isAnnexb, const char * content, uint32_t size)
// {
// 	unsigned char * data = (unsigned char*)packet;
// 	unsigned int nalu_size = (unsigned int)get_sei_nalu_size(size);
// 	uint32_t sei_size = nalu_size;
// 	//大端转小端
// 	nalu_size = reversebytes(nalu_size);
 
// 	//NALU开始码
// 	unsigned int * size_ptr = &nalu_size;
// 	if (isAnnexb)
// 	{
// 		memcpy(data, start_code, sizeof(unsigned int));
// 	}
// 	else
// 	{
// 		memcpy(data, size_ptr, sizeof(unsigned int));
// 	}
// 	data += sizeof(unsigned int);
 
// 	unsigned char * sei = data;
// 	//NAL header
// 	*data++ = 6; //SEI
// 	//sei payload type
// 	*data++ = 5; //unregister
// 	size_t sei_payload_size = size + UUID_SIZE;
// 	//数据长度
// 	while (1)
// 	{
// 		*data++ = (sei_payload_size >= 0xFF ? 0xFF : (char)sei_payload_size);
// 		if (sei_payload_size < 0xFF) break;
// 		sei_payload_size -= 0xFF;
// 	}
 
// 	//UUID
// 	memcpy(data, uuid, UUID_SIZE);
// 	data += UUID_SIZE;
// 	//数据
// 	memcpy(data, content, size);
// 	data += size;
 
// 	//tail 截止对齐码
// 	if (sei + sei_size - data == 1)
// 	{
// 		*data = 0x80;
// 	}
// 	else if (sei + sei_size - data == 2)
// 	{
// 		*data++ = 0x00;
// 		*data++ = 0x80;
// 	}
 
// 	return 1;
// }


MPP_RET fill_image(RK_U8 *buf, RK_U32 width, RK_U32 height,
                   RK_U32 hor_stride, RK_U32 ver_stride, MppFrameFormat fmt,
                   RK_U32 frame_count)
{
    MPP_RET ret = MPP_OK;
    RK_U8 *buf_y = buf;
    RK_U8 *buf_c = buf + hor_stride * ver_stride;
    RK_U32 x, y;

    switch (fmt) {
    case MPP_FMT_YUV420SP : {
        RK_U8 *p = buf_y;

        for (y = 0; y < height; y++, p += hor_stride) {
            for (x = 0; x < width; x++) {
                p[x] = x + y + frame_count * 3;
            }
        }

        p = buf_c;
        for (y = 0; y < height / 2; y++, p += hor_stride) {
            for (x = 0; x < width / 2; x++) {
                p[x * 2 + 0] = 128 + y + frame_count * 2;
                p[x * 2 + 1] = 64  + x + frame_count * 5;
            }
        }
    } break;
    case MPP_FMT_YUV420P : {
        RK_U8 *p = buf_y;

        for (y = 0; y < height; y++, p += hor_stride) {
            for (x = 0; x < width; x++) {
                p[x] = x + y + frame_count * 3;
            }
        }

        p = buf_c;
        for (y = 0; y < height / 2; y++, p += hor_stride / 2) {
            for (x = 0; x < width / 2; x++) {
                p[x] = 128 + y + frame_count * 2;
            }
        }

        p = buf_c + hor_stride * ver_stride / 4;
        for (y = 0; y < height / 2; y++, p += hor_stride / 2) {
            for (x = 0; x < width / 2; x++) {
                p[x] = 64 + x + frame_count * 5;
            }
        }
    } break;
    case MPP_FMT_YUV422_UYVY : {
        RK_U8 *p = buf_y;

        for (y = 0; y < height; y++, p += hor_stride) {
            for (x = 0; x < width / 2; x++) {
                p[x * 4 + 1] = x * 2 + 0 + y + frame_count * 3;
                p[x * 4 + 3] = x * 2 + 1 + y + frame_count * 3;
                p[x * 4 + 0] = 128 + y + frame_count * 2;
                p[x * 4 + 2] = 64  + x + frame_count * 5;
            }
        }
    } break;
    case MPP_FMT_RGB888 :
    case MPP_FMT_BGR888 :
    case MPP_FMT_ARGB8888 : {
        RK_U8 *p = buf_y;
        RK_U32 pix_w = (fmt == MPP_FMT_ARGB8888 || fmt == MPP_FMT_ABGR8888) ? 4 : 4;

        for (y = 0; y < height; y++, p += hor_stride * pix_w) {
            for (x = 0; x < width; x++) {
                p[x * 4 + 0] = x * 3 + 0 + y + frame_count * 3;
                p[x * 4 + 1] = x * 3 + 1 + y + frame_count * 3;
                p[x * 4 + 2] = x * 3 + 2 + y + frame_count * 3;
                p[x * 4 + 3] = 0;
            }
        }
    } break;
    default : {
        mpp_err_f("filling function do not support type %d\n", fmt);
        ret = MPP_NOK;
    } break;
    }
    return ret;
}

MPP_RET test_ctx_init(MpiEncData **data, MpiEncCmd *cmd)
{
    MpiEncData *p = NULL;
    MPP_RET ret = MPP_OK;

    if (!data || !cmd) {
        mpp_err_f("invalid input data %p cmd %p\n", data, cmd);
        return MPP_ERR_NULL_PTR;
    }

    p = mpp_calloc(MpiEncData, 1);
    if (!p) {
        mpp_err_f("create MpiEncData failed\n");
        ret = MPP_ERR_MALLOC;
        goto RET;
    }

    // get paramter from cmd
    p->width        = cmd->width;
    p->height       = cmd->height;
    p->hor_stride   = MPP_ALIGN(cmd->width, 16);
    p->ver_stride   = MPP_ALIGN(cmd->height, 16);
    p->fmt          = cmd->format;
    p->type         = cmd->type;
     if (cmd->type == MPP_VIDEO_CodingMJPEG)
         cmd->num_frames = 1;
    p->num_frames   = cmd->num_frames;
    p->bps          = cmd->target_bps;

    if (cmd->have_input) {
        p->fp_input = fopen(cmd->file_input, "rb");
        if (NULL == p->fp_input) {
            mpp_err("failed to open input file %s\n", cmd->file_input);
            mpp_err("create default yuv image for test\n");
        }
    }

    if (cmd->have_output) {
        p->fp_output = fopen(cmd->file_output, "w+b");
        if (NULL == p->fp_output) {
            mpp_err("failed to open output file %s\n", cmd->file_output);
            ret = MPP_ERR_OPEN_FILE;
        }
    }

    // update resource parameter
    if (p->fmt <= MPP_FMT_YUV_BUTT)
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64) * 3 / 2;
    else
        p->frame_size = MPP_ALIGN(p->hor_stride, 64) * MPP_ALIGN(p->ver_stride, 64) * 4;
    p->packet_size  = p->width * p->height;
    //NOTE: hor_stride should be 16-MB aligned
    p->mdinfo_size  = (((p->hor_stride + 255) & (~255)) / 16) * (p->ver_stride / 16) * 4;
    /*
     * osd idx size range from 16x16 bytes(pixels) to hor_stride*ver_stride(bytes).
     * for general use, 1/8 Y buffer is enough.
     */
    p->osd_idx_size  = p->hor_stride * p->ver_stride / 8;
    p->plt_table[0] = MPP_ENC_OSD_PLT_RED;
    p->plt_table[1] = MPP_ENC_OSD_PLT_YELLOW;
    p->plt_table[2] = MPP_ENC_OSD_PLT_BLUE;
    p->plt_table[3] = MPP_ENC_OSD_PLT_GREEN;
    p->plt_table[4] = MPP_ENC_OSD_PLT_CYAN;
    p->plt_table[5] = MPP_ENC_OSD_PLT_TRANS;
    p->plt_table[6] = MPP_ENC_OSD_PLT_BLACK;
    p->plt_table[7] = MPP_ENC_OSD_PLT_WHITE;

RET:
    *data = p;
    return ret;
}

MPP_RET test_ctx_deinit(MpiEncData **data)
{
    MpiEncData *p = NULL;

    if (!data) {
        mpp_err_f("invalid input data %p\n", data);
        return MPP_ERR_NULL_PTR;
    }

    p = *data;
    if (p) {
        if (p->fp_input) {
            fclose(p->fp_input);
            p->fp_input = NULL;
        }
        if (p->fp_output) {
            fclose(p->fp_output);
            p->fp_output = NULL;
        }
        MPP_FREE(p);
        *data = NULL;
    }

    return MPP_OK;
}

MPP_RET test_res_init(MpiEncData *p)
{
    RK_U32 i;
    MPP_RET ret;

    mpp_assert(p);

    ret = mpp_buffer_group_get_internal(&p->frm_grp, MPP_BUFFER_TYPE_ION);
    if (ret) {
        mpp_err("failed to get buffer group for input frame ret %d\n", ret);
        goto RET;
    }

    ret = mpp_buffer_group_get_internal(&p->pkt_grp, MPP_BUFFER_TYPE_ION);
    if (ret) {
        mpp_err("failed to get buffer group for output packet ret %d\n", ret);
        goto RET;
    }

    ret = mpp_buffer_group_get_internal(&p->buf_grp, MPP_BUFFER_TYPE_ION);
    if (ret) {
        mpp_err("failed to get buffer group for output packet ret %d\n", ret);
        goto RET;
    }

    for (i = 0; i < MPI_ENC_IO_COUNT; i++) {
        ret = mpp_buffer_get(p->frm_grp, &p->frm_buf[i], p->frame_size);
        if (ret) {
            mpp_err("failed to get buffer for input frame ret %d\n", ret);
            goto RET;
        }

        ret = mpp_buffer_get(p->pkt_grp, &p->pkt_buf[i], p->packet_size);
        if (ret) {
            mpp_err("failed to get buffer for input frame ret %d\n", ret);
            goto RET;
        }

        ret = mpp_buffer_get(p->pkt_grp, &p->md_buf[i], p->mdinfo_size);
        if (ret) {
            mpp_err("failed to get buffer for motion detection info ret %d\n", ret);
            goto RET;
        }
    }
RET:
    return ret;
}

MPP_RET test_res_deinit(MpiEncData *p)
{
    RK_U32 i;

    mpp_assert(p);

    for (i = 0; i < MPI_ENC_IO_COUNT; i++) {
        if (p->frm_buf[i]) {
            mpp_buffer_put(p->frm_buf[i]);
            p->frm_buf[i] = NULL;
        }

        if (p->pkt_buf[i]) {
            mpp_buffer_put(p->pkt_buf[i]);
            p->pkt_buf[i] = NULL;
        }

        if (p->md_buf[i]) {
            mpp_buffer_put(p->md_buf[i]);
            p->md_buf[i] = NULL;
        }
    }

    if (p->frm_grp) {
        mpp_buffer_group_put(p->frm_grp);
        p->frm_grp = NULL;
    }

    if (p->pkt_grp) {
        mpp_buffer_group_put(p->pkt_grp);
        p->pkt_grp = NULL;
    }

    if (p->buf_grp) {
        mpp_buffer_group_put(p->pkt_grp);
        p->buf_grp = NULL;
    }

    return MPP_OK;
}

MPP_RET test_mpp_init(MpiEncData *p)
{
    MPP_RET ret;

    if (NULL == p)
        return MPP_ERR_NULL_PTR;

    ret = mpp_create(&p->ctx, &p->mpi);
    if (ret) {
        mpp_err("mpp_create failed ret %d\n", ret);
        goto RET;
    }

    ret = mpp_init(p->ctx, MPP_CTX_ENC, p->type);
    if (ret)
        mpp_err("mpp_init failed ret %d\n", ret);

RET:
    return ret;
}

static MPP_RET mpp_enc_setup(MpiEncData *p,int streamID)
{
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;
    MppEncCodecCfg *codec_cfg;
    MppEncPrepCfg *prep_cfg;
    MppEncRcCfg *rc_cfg;

    if (NULL == p)
        return MPP_ERR_NULL_PTR;

    mpi = p->mpi;
    ctx = p->ctx;
    codec_cfg = &p->codec_cfg;
    prep_cfg = &p->prep_cfg;
    rc_cfg = &p->rc_cfg;

    /* setup default parameter */
    // if (gs_vedio_info.low_rtsp_fps == 1 && streamID == 1)
    // {
    //     p->fps = 15;
    // }
    // else
    
        p->fps = 30; //30;
    

    // if (gs_vedio_info.minimum_quality == 1 && streamID == 1)
    // {
    //     p->gop = 3;
    // }
    // else
    // {
        p->gop = 30;//60;
    // }

    if (!p->bps) {
        p->bps = p->width * p->height / 8 * p->fps;
        printf("p->bps %d\n",p->bps);
    }

    p->qp_init  = (p->type == MPP_VIDEO_CodingMJPEG) ? (10) : (26);

    prep_cfg->change        = MPP_ENC_PREP_CFG_CHANGE_INPUT |
                             // MPP_ENC_PREP_CFG_CHANGE_ROTATION |
                              MPP_ENC_PREP_CFG_CHANGE_FORMAT;
    prep_cfg->width         = p->width;
    prep_cfg->height        = p->height;
    prep_cfg->hor_stride    = p->hor_stride;
    prep_cfg->ver_stride    = p->ver_stride;
    prep_cfg->format        = p->fmt;
    prep_cfg->rotation      = MPP_ENC_ROT_0;
    ret = mpi->control(ctx, MPP_ENC_SET_PREP_CFG, prep_cfg);
    if (ret) {
        mpp_err("mpi control enc set prep cfg failed ret %d\n", ret);
        goto RET;
    }

    rc_cfg->change  = MPP_ENC_RC_CFG_CHANGE_ALL;

    if(streamID==3){
        rc_cfg->rc_mode = MPP_ENC_RC_MODE_VBR;
    }else{
        rc_cfg->rc_mode = MPP_ENC_RC_MODE_CBR;
    }
    rc_cfg->quality = MPP_ENC_RC_QUALITY_BEST;

    if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_FIXQP) {
        /* constant QP does not have bps */
        rc_cfg->bps_target   = -1;
        rc_cfg->bps_max      = -1;
        rc_cfg->bps_min      = -1;
    } else if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_CBR) {
        /* constant bitrate has very small bps range of 1/16 bps */
        rc_cfg->bps_target   = p->bps ;
        rc_cfg->bps_max      = p->bps *17 / 16;
        rc_cfg->bps_min      = p->bps *15 / 16;
    } else if (rc_cfg->rc_mode ==  MPP_ENC_RC_MODE_VBR) {
        /* variable bitrate has large bps range */
        rc_cfg->bps_target   = p->bps;
        rc_cfg->bps_max      = p->bps * 17 / 16;
        rc_cfg->bps_min      = p->bps * 1 / 16;
    }

    /* fix input / output frame rate */
    rc_cfg->fps_in_flex      = 0;
    rc_cfg->fps_in_num       = p->fps;
    rc_cfg->fps_in_denorm    = 1;
    rc_cfg->fps_out_flex     = 0;
    rc_cfg->fps_out_num      = p->fps;
    rc_cfg->fps_out_denorm   = 1;

    rc_cfg->gop              = p->gop;
    rc_cfg->max_reenc_times  = 1;
    rc_cfg->skip_cnt         = 0;

    if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_FIXQP) {
        /* constant QP mode qp is fixed */
        p->qp_max   = p->qp_init;
        p->qp_min   = p->qp_init;
        p->qp_step  = 0;
    } else if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_CBR) {
        /* constant bitrate do not limit qp range */
        p->qp_max   = 48;
        p->qp_min   = 4;
        p->qp_step  = 16;
        p->qp_init  = 0;
    } else if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_VBR) {
        /* variable bitrate has qp min limit */
        p->qp_max   = 40;
        p->qp_min   = 12;
        p->qp_step  = 8;
        p->qp_init  = 0;
    }

    rc_cfg->qp_max          = p->qp_max;
    rc_cfg->qp_min          = p->qp_min;
    rc_cfg->qp_max_i        = p->qp_max;
    rc_cfg->qp_min_i        = p->qp_min;
    rc_cfg->qp_init         = p->qp_init;
    rc_cfg->qp_max_step     = p->qp_step;
    rc_cfg->qp_delta_ip     = 4;
    rc_cfg->qp_delta_vi     = 2;


    if(streamID==3) {
        int qp_set=track_state_mechine.delay_count;
        rc_cfg->qp_max          = qp_set;
        rc_cfg->qp_min          = qp_set;
        rc_cfg->qp_max_i        = qp_set;
        rc_cfg->qp_min_i        = qp_set;
        rc_cfg->qp_init         = qp_set;
    }

   

    printf("%d bps %d fps %d gop %d qp %d\n",streamID,\
            rc_cfg->bps_target, rc_cfg->fps_out_num, rc_cfg->gop,rc_cfg->qp_init);
    ret = mpi->control(ctx, MPP_ENC_SET_RC_CFG, rc_cfg);
    if (ret) {
        mpp_err("mpi control enc set rc cfg failed ret %d\n", ret);
        goto RET;
    }

    codec_cfg->coding = p->type;
    switch (codec_cfg->coding) {
    case MPP_VIDEO_CodingAVC : {
        codec_cfg->h264.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
                                 MPP_ENC_H264_CFG_CHANGE_ENTROPY |
                                 MPP_ENC_H264_CFG_CHANGE_TRANS_8x8;
        /*
         * H.264 profile_idc parameter
         * 66  - Baseline profile
         * 77  - Main profile
         * 100 - High profile
         */
        codec_cfg->h264.profile  = 100;
        /*
         * H.264 level_idc parameter
         * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
         * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
         * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
         * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
         * 50 / 51 / 52         - 4K@30fps
         */

        //--------------------
        


        // if(gs_vedio_info.minimum_quality == 1 && streamID == 1)
        // {
        //     codec_cfg->h264.qp_max_i=40;
        //     codec_cfg->h264.qp_min_i=40;
        // }
        codec_cfg->h264.level    = 40;
        codec_cfg->h264.entropy_coding_mode  = 1;
        codec_cfg->h264.cabac_init_idc  = 0;
        codec_cfg->h264.transform8x8_mode = 1;
        if(streamID==1){
            codec_cfg->h264.intra_refresh_mode=1;
            codec_cfg->h264.intra_refresh_arg=1;
            codec_cfg->h264.deblock_disable=0;
        }

        if(streamID==3){
            int qp_set=track_state_mechine.delay_count;
            codec_cfg->h264.qp_init=qp_set;
            codec_cfg->h264.qp_max=qp_set;
            codec_cfg->h264.qp_min=qp_set;
            codec_cfg->h264.qp_max_i=qp_set;
            codec_cfg->h264.qp_min_i=qp_set;
            logger_info("enc","VIDEO CODE QP %d\n",qp_set);
        }

        
    } break;
    case MPP_VIDEO_CodingMJPEG : {
        //codec_cfg->jpeg.change      = MPP_ENC_JPEG_CFG_CHANGE_QFACTOR;
        codec_cfg->jpeg.q_factor    = 90;
        codec_cfg->jpeg.qf_min      = 1;
        codec_cfg->jpeg.qf_max      = 99;
	    codec_cfg->jpeg.change  = MPP_ENC_JPEG_CFG_CHANGE_QP;
        codec_cfg->jpeg.quant   = 10;
    } break;
    case MPP_VIDEO_CodingHEVC : {
        codec_cfg->h265.change = MPP_ENC_H265_CFG_INTRA_QP_CHANGE;
        codec_cfg->h265.intra_qp = 26;
        //if(streamID==3){
            // codec_cfg->h265.qp_init=26;
            // codec_cfg->h265.max_qp=33;
            // codec_cfg->h265.min_qp=22;
            if(streamID==3){
                int qp_set=track_state_mechine.delay_count;
                codec_cfg->h265.qp_init=qp_set;
                codec_cfg->h265.max_qp=qp_set;
                codec_cfg->h265.min_qp=qp_set;
                codec_cfg->h265.max_i_qp=qp_set;
                codec_cfg->h265.min_i_qp=qp_set;
            }
    
        if(gs_vedio_info.minimum_quality == 1 && streamID == 1)
        {
            codec_cfg->h265.qp_init=40;
            codec_cfg->h265.max_qp=51;
            codec_cfg->h265.min_qp=51;
            codec_cfg->h265.max_i_qp=51;
            codec_cfg->h265.min_i_qp=51;
            codec_cfg->h265.level=40;
        }
            // codec_cfg->h265.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
            //                      MPP_ENC_H264_CFG_CHANGE_ENTROPY |
            //                      MPP_ENC_H264_CFG_CHANGE_TRANS_8x8|
            //                      MPP_ENC_H264_CFG_CHANGE_QP_LIMIT;
       // }
    } break;
    case MPP_VIDEO_CodingVP8 :
    default : {
        mpp_err_f("support encoder coding type %d\n", codec_cfg->coding);
    } break;
    }
    ret = mpi->control(ctx, MPP_ENC_SET_CODEC_CFG, codec_cfg);
    if (ret) {
        mpp_err("mpi control enc set codec cfg failed ret %d\n", ret);
        goto RET;
    ret = mpi->control(ctx, MPP_ENC_GET_CODEC_CFG, codec_cfg);
    if (ret) {
        mpp_err("mpi control enc set codec cfg failed ret %d\n", ret);
        goto RET;
    }
        // printf("%d %d %d %d %d %d %d\n", codec_cfg->h264.qp_init,
        //        codec_cfg->h264.intra_refresh_arg,
        //        codec_cfg->h264.intra_refresh_mode,
        //        codec_cfg->h264.qp_max_i,
        //        codec_cfg->h264.qp_min_i,
        //        codec_cfg->h264.qp_max_step,
        //        codec_cfg->h264.qp_delta_ip);
    }

    /* optional */
    p->sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
    ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->sei_mode);
    if (ret) {
        mpp_err("mpi control enc set sei cfg failed ret %d\n", ret);
        goto RET;
    }

RET:
    return ret;
}

/*
 * write header here
 */
MPP_RET test_mpp_preprare(MpiEncData *p)
{
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;
    MppPacket packet = NULL;

    if (NULL == p)
        return MPP_ERR_NULL_PTR;

    mpi = p->mpi;
    ctx = p->ctx;
    ret = mpi->control(ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
    if (ret) {
        mpp_err("mpi control enc get extra info failed\n");
        goto RET;
    }

    /* get and write sps/pps for H.264 */
    if (packet) {
        void *ptr   = mpp_packet_get_pos(packet);
        size_t len  = mpp_packet_get_length(packet);

        // if (p->fp_output)
        //     fwrite(ptr, 1, len, p->fp_output);

        packet = NULL;
    }
RET:
    return ret;
}

MPP_RET test_mpp_deinit(MpiEncData *p)
{
    if (p->ctx) {
        mpp_destroy(p->ctx);
        p->ctx = NULL;
    }

    return MPP_OK;
}

unsigned char  stream_out_buff[1920*1080]={0};
unsigned char  KLV_out_buff[1920*1080]={0};
unsigned char  head[1920*1080]={0};
static int head_len;
static unsigned int rtsp_frame_cnt;
static MPP_RET mpp_enc_run(void)
{
    int number = 0;
    FILE *fp = NULL;
    static double t0,t1,t2,t20;
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;    
    MpiEncData *p = gs_enc_data;
    
#ifdef TS_OUTPUT_ENABLE  
    unsigned long long ts_pts =  0; 
    SetEncType(0,0);
#endif
    struct timespec start_tm,end_tm;
    int tsCount=0;
	int pts_num_tmp=1000000/gs_vedio_info.vo_fps;
	data_stream_t *data_stream = data_stream_get();
    image_frame_info_t frame_info; 
    rtsp_frame_cnt = 0;
    if (NULL == p) 
        return MPP_ERR_NULL_PTR;

    mpi = p->mpi;
    ctx = p->ctx;
    int i = 0;
    if ((p->type == MPP_VIDEO_CodingAVC) || (p->type == MPP_VIDEO_CodingHEVC)) {
        MppPacket packet = NULL;
        ret = mpi->control(ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
        if (ret) {
            SLOGI("mpi control enc get extra info failed\n");
            goto RET;
        }

        /* get and write sps/pps for H.264 */
        if (packet) {
            printf("head save sucussee -----------------------\n");
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);
            // send video 
#ifdef RTSP_OUTPUT_ENABLE
            if(1) {
                //rtsp_server_push_video(0,ptr,len);
                happytime_rtsp_server_data_input(0,ptr,len);
                memcpy(head,ptr,len);
                head_len=len;
                printf("this is ok \n");
            }
#endif
            //save_image_data(ptr,len,"head.h264");

// #ifdef TS_OUTPUT_ENABLE         
//             if(gs_ts_udp_info.open_udpts_flag){
//                 memcpy(head,ptr,len);
//                 head_len=len;
//                 ts_pts+=pts_num_tmp;
//                 tsCount =  ConverES2TS(0,stream_out_buff,ptr,0,gs_vedio_info.vo_fps, len, ts_pts);
//                 int size_klv=injectKLV(0,tsCount * 188,stream_out_buff,KLV_out_buff);
//                 QueuePush(&tsQueue,KLV_out_buff,size_klv); 
//             } 
// #endif                       
            packet = NULL;
        }
    }

    while (!p->pkt_eos) {
        MppFrame frame = NULL;
        MppPacket packet = NULL;
        i=p->frame_count%4;
        void *buf = mpp_buffer_get_ptr(p->frm_buf[i]);
        /*if(buf){*/
            /*if(data_stream->yuv_handle_frame_count<2){*/
                /*fill_image(buf, p->width, p->height, p->hor_stride,\*/
                                    /*p->ver_stride, p->fmt, p->frame_count);*/
                /*msleep(15);    */
            /*}else{*/
                // printf("p->width, p->height, p->hor_stride,p->ver_stride %d %d %d %d",p->width, p->height, p->hor_stride,p->ver_stride);
                encode_data_wait_and_get(buf,p->width, p->height, p->hor_stride,p->ver_stride);
            /*}*/
        // if(gs_vedio_info.low_rtsp_fps == 1)
        // {
        //     rtsp_frame_cnt++;
        //     if (rtsp_frame_cnt % 2 == 0)
        //         continue;
        // }
            
        //}
        t0=what_time_is_it_now();
        ret = mpp_frame_init(&frame);
        if (ret) {
            SLOGI("mpp_frame_init failed\n");
            goto RET;
        }
        
        mpp_frame_set_width(frame, p->width);
        mpp_frame_set_height(frame, p->height);
        mpp_frame_set_hor_stride(frame, p->hor_stride);
        mpp_frame_set_ver_stride(frame, p->ver_stride);
        mpp_frame_set_fmt(frame, p->fmt);
        mpp_frame_set_eos(frame, p->frm_eos);

        if (p->fp_input && feof(p->fp_input))
        {
            mpp_frame_set_buffer(frame, NULL);
        }
        else
        {
            mpp_frame_set_buffer(frame, p->frm_buf[i]);
        }
       
        ret = mpi->encode_put_frame(ctx, frame);
        if (ret) 
        {
            printf("mpp encode put frame failed\n");
            goto RET;
        }
       
        ret = mpi->encode_get_packet(ctx, &packet);
        if (ret) {
            SLOGI("mpp encode get packet failed\n");
            goto RET;
        }

        mpp_assert(packet);
  

        if (packet) {
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);
            if(len>1024*1024){
                len=1024*1024;
            }
            p->pkt_eos = mpp_packet_get_eos(packet);
#ifdef RTSP_OUTPUT_ENABLE
            if (1) {
                // rtsp_server_push_video(0,ptr,len);
                unsigned char *tmp = ptr;
                unsigned char head_idr_code = 0;
                if ( 1 ){
                    head_idr_code= ((tmp[4]&0x1f)==0x6) ? 1 : 0 ;   
                }else{
                    head_idr_code= (((tmp[4]&0x7e)>>1)==0x27) ? 1 : 0 ;
                }
                if (head_idr_code) { 
                    happytime_rtsp_server_data_input(0, head, head_len);
                    // happy_time_sps_flag=1;
                } 
                happytime_rtsp_server_data_input(0, ptr, len);
            }
        #endif

// #ifdef TS_OUTPUT_ENABLE            
//             if(gs_ts_udp_info.open_udpts_flag){
//                 unsigned char *tmp=ptr;
//                 ts_pts+=pts_num_tmp;
//                 int flag=1;
//                 if((tmp[4]&0x1f)==0x6){
//                     flag=0;
//                     memcpy(head+head_len,ptr,len);
//                     tsCount =  ConverES2TS(0,stream_out_buff,head,flag,gs_vedio_info.vo_fps, len+head_len, ts_pts);
//                 }else{
//                     tsCount =  ConverES2TS(0,stream_out_buff,ptr,flag,gs_vedio_info.vo_fps, len, ts_pts);
//                 }
//                 int size_klv=injectKLV(p->frame_count%16,tsCount * 188,stream_out_buff,KLV_out_buff);
//                 // if(p->frame_count>=500 && p->frame_count<=510){
//                 //     char file_path[256]={0};
//                 //     sprintf(file_path,"/tmp/ts_%d",p->frame_count);
//                 //     FILE *fp_ts=fopen(file_path,"wb");
//                 //     if(fp_ts!=NULL){
//                 //         fwrite(stream_out_buff,1,tsCount * 188,fp_ts);
//                 //         fflush(fp_ts);
//                 //         fclose(fp_ts);
//                 //     }
//                 // }
//                 QueuePush(&tsQueue,KLV_out_buff,size_klv);              
//             }
// #endif           
            // t1=what_time_is_it_now();
// #ifdef MPP_ENC_TIME_LOG
//             t1=what_time_is_it_now();
//             t20+=(t1-t0);
//             if(p->frame_count%100 == 88){
//                 SLOGI("mpp_ec %f %fms %d\n",t20*10,(t1-t2)*1000,i);
//                 t20=0;
//             }
//             t2=t1;
// #endif
           
            mpp_packet_deinit(&packet);

            p->stream_size += len;
            p->frame_count++;

            if (p->pkt_eos) {
                mpp_log("found last packet\n");
                mpp_assert(p->frm_eos);
            }
        }

        if (p->num_frames && p->frame_count >= p->num_frames) {
            break;
        }
        if (p->frm_eos && p->pkt_eos)
        {
            break;
        }
            
    }
RET:

    return ret;
}
unsigned char  head_video[1920*1080]={0};
int head_len_video;
unsigned char  stream_out_buff2[1920*1080]={0};
unsigned char  KLV_out_buff2[1920*1080]={0};

static MPP_RET mpp_enc_run2(void)
{
    // cpu_set_t mask;
	// int cpuid = 2;
	// CPU_ZERO(&mask);
	// CPU_SET(cpuid, &mask);
	// if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
	// {
	// 	printf("rga_handle_thread init error\n");
	// }
    static double t0,t1,t2,t20;
    int i=0;
    unsigned long long ts_pts =  0;
    int tsCount=0;
	int pts_num_tmp=1000000/gs_vedio_info.vo_fps;

    SetEncType(1,0);
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;    
    MpiEncData *p = gs_enc_data2;
    FILE *pFile;
	data_stream_t *data_stream = data_stream_get();
    image_frame_info_t frame_info; 
    if (NULL == p) 
        return MPP_ERR_NULL_PTR;

    mpi = p->mpi;
    ctx = p->ctx;

    if ((p->type == MPP_VIDEO_CodingAVC) || (p->type == MPP_VIDEO_CodingHEVC)) {
        MppPacket packet = NULL;
        ret = mpi->control(ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
        if (ret) {
            mpp_err("mpi control enc get extra info failed\n");
            goto RET;
        }

        /* get and write sps/pps for H.264 | H.265*/
//         if (packet) {
//             void *ptr = mpp_packet_get_pos(packet);
//             size_t len = mpp_packet_get_length(packet);

//              init_video_original_file_head(ptr,len);
//             // if(gs_vedio_info.output_type==TYPE_H264)
//             // {
//             //     pFile = fopen(INIT_RECORD_INFO_H264_PATH, "wb");
//             // }
//             // else {
//             //     pFile = fopen(INIT_RECORD_INFO_H265_PATH, "wb");
//             // }

//             // if (!pFile)
//             // {
//             //     printf("open file err!\n");
//             //     return MPP_ERR_NULL_PTR;
//             // }
//             // if(len>0){
//             //     fwrite(ptr,len, 1, pFile);
//             //     fflush(pFile);
//             //     memcpy(head_video,ptr,len);
//             //     head_len_video=len;
//             //     // for (int i = 0; i<10; ++i) {
//             //     //     for (int j = 0; j < 10; j++) {
//             //     //         printf("%x ", head_video[10*i+j]);
//             //     //     }
//             //     //     printf("\n");
//             //     // }
//             //     logger_info("record", "head length  : %d", len);
// #ifdef H264_VIDEO_TYPE         
//             if(gs_ts_udp_info.open_udpts_flag){
//                 ts_pts+=pts_num_tmp;
//                 tsCount =  ConverES2TS(1,stream_out_buff2,ptr,0,gs_vedio_info.vo_fps, len, ts_pts);
//                     int size_klv=injectKLV(p->frame_count%16,tsCount * 188,stream_out_buff2,KLV_out_buff2);
//             } 
// #endif  
//             }

//             //fclose(pFile);
//            // char * tmp_i=ptr;
//             //SLOGI("init_video_original_file_head %x %x %x\n", getVopType(tmp_i,len),tmp_i[4],tmp_i[4]&0x1f);
//             packet = NULL;
//         }
    }

    while (!p->pkt_eos) {
        MppFrame frame = NULL;
        MppPacket packet = NULL;
        i=p->frame_count%4;
        void *buf = mpp_buffer_get_ptr(p->frm_buf[i]);
        //ret = fill_image(buf, p->width, p->height, p->hor_stride,
       //                      p->ver_stride, p->fmt, p->frame_count);
        video_data_wait_and_get(buf,p->width, p->height, p->hor_stride,p->ver_stride,p->frame_count);
        t0=what_time_is_it_now();
        ret = mpp_frame_init(&frame);
        if (ret) {
            mpp_err_f("mpp_frame_init failed\n");
            goto RET;
        }
        
        mpp_frame_set_width(frame, p->width);
        mpp_frame_set_height(frame, p->height);
        mpp_frame_set_hor_stride(frame, p->hor_stride);
        mpp_frame_set_ver_stride(frame, p->ver_stride);
        mpp_frame_set_fmt(frame, p->fmt);
        mpp_frame_set_eos(frame, p->frm_eos);

        if (p->fp_input && feof(p->fp_input))
            mpp_frame_set_buffer(frame, NULL);
        else
            mpp_frame_set_buffer(frame, p->frm_buf[i]);
       
        ret = mpi->encode_put_frame(ctx, frame);
        if (ret) {
            mpp_err("mpp encode put frame failed\n");
            goto RET;
        }
       
        ret = mpi->encode_get_packet(ctx, &packet);
        if (ret) {
            mpp_err("mpp encode get packet failed\n");
            goto RET;
        }
        mpp_assert(packet);
        if (packet) {
            // write packet to file here
            void *ptr = mpp_packet_get_pos(packet);
            size_t len = mpp_packet_get_length(packet);

            p->pkt_eos = mpp_packet_get_eos(packet);

            if (pFile && len > 0) {
                if (p->frame_count < 60) {
                    fwrite(ptr, len, 1, pFile);
                    fflush(pFile);
                } else if (p->frame_count == 60) {
                    fclose(pFile);
                    SLOGI("init h265 head file end \n");
                }
            }

            // if(gs_ts_udp_info.open_udpts_flag){
#if H264_VIDEO_TYPE
            unsigned char *tmp = ptr;
            ts_pts += pts_num_tmp;
            int flag = 1;
            unsigned char head_idr_code = 0;
            if(gs_vedio_info.output_type==TYPE_H264){
                head_idr_code= ((tmp[4]&0x1f)==0x6) ? 1 : 0 ;           
            }else{
                head_idr_code= (((tmp[4]&0x7e)>>1)==0x27) ? 1 : 0 ;
            }
            if (head_idr_code) {
                flag = 0;
                memcpy(head_video + head_len_video, ptr, len);
                tsCount = ConverES2TS(1, stream_out_buff2, head_video, flag, gs_vedio_info.vo_fps, len + head_len_video, ts_pts);
            } else {
                tsCount = ConverES2TS(1, stream_out_buff2, ptr, flag, gs_vedio_info.vo_fps, len, ts_pts);
            }
            if (tsCount > 0) {
                        //int size_klv=injectKLV(p->frame_count%16,tsCount * 188,stream_out_buff2,KLV_out_buff2);
                        //zm_video_data_input(0,KLV_out_buff2,size_klv);
                        zm_video_data_input(0,stream_out_buff2,tsCount * 188);
            }

                #else
                // SLOGI("flag : %x \n",flag);
                 zm_video_data_input(0,ptr,len);
                #endif
                //SLOGI("%d %d",len,tsCount);
                //if(tsCount>0){
                    //int size_klv=injectKLV(tsCount * 188,stream_out_buff2,KLV_out_buff2);
                   // zm_video_data_input(0,KLV_out_buff2,size_klv);
               // }
           //}else{
               // zm_video_data_input(0,ptr,len);
          // }

        #ifdef MPP_ENC_TIME_LOG
            t1=what_time_is_it_now();
            t20+=(t1-t0);
            if(p->frame_count%100 == 88){
                SLOGI("mpp_ec2 %f %fms %d\n",t20*10,(t1-t2)*1000,i);
                t20=0;
            }
            t2=t1;
        #endif
            mpp_packet_deinit(&packet);
            //mpp_log_f("encoded frame %d size %d\n", p->frame_count, len);
            p->stream_size += len;
            p->frame_count++;
            if (p->pkt_eos) {
                mpp_log("found last packet\n");
                mpp_assert(p->frm_eos);
            }
        }
        if (p->num_frames && p->frame_count >= p->num_frames) {
            mpp_log_f("encode max %d frames", p->frame_count);
            break;
        }
        if (p->frm_eos && p->pkt_eos)
            break;
    }
RET:

    return ret;
}


static MPP_RET mpp_enc_run1(void)
{
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;    
    MpiEncData *p = gs_enc_data1;

    if (NULL == p) 
        return MPP_ERR_NULL_PTR;

    mpi = p->mpi;
    ctx = p->ctx;
    int i=0;
    while (!p->pkt_eos) {
        MppFrame frame = NULL;
        MppPacket packet = NULL;
        i=p->frame_count%4;
        void *buf = mpp_buffer_get_ptr(p->frm_buf[i]);

        jpeg_data_wait_and_get(buf,p->width, p->height, p->hor_stride,
                             p->ver_stride);

        ret = mpp_frame_init(&frame);
        if (ret) {
            mpp_err_f("mpp_frame_init failed\n");
            goto RET;
        } 
        mpp_frame_set_width(frame, p->width);
        mpp_frame_set_height(frame, p->height);
        mpp_frame_set_hor_stride(frame, p->hor_stride);
        mpp_frame_set_ver_stride(frame, p->ver_stride);
        mpp_frame_set_fmt(frame, p->fmt);
        mpp_frame_set_eos(frame, p->frm_eos);

        if (p->fp_input && feof(p->fp_input))
            mpp_frame_set_buffer(frame, NULL);
        else
            mpp_frame_set_buffer(frame, p->frm_buf[i]);
   
        ret = mpi->encode_put_frame(ctx, frame);
        if (ret) {
            mpp_err("mpp encode put frame failed\n");
            goto RET;
        }

        ret = mpi->encode_get_packet(ctx, &packet);
        if (ret) {
            mpp_err("mpp encode get packet failed\n");
            goto RET;
        }

        mpp_assert(packet);
  

        if (packet) {
            // write packet to file here
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);

            p->pkt_eos = mpp_packet_get_eos(packet);

            if(gs_record_info.sdcard_stat==1){
                char file_path[256];
                char cmd[256];
                if(gs_time_info.year!=0){
                        sprintf(cmd,"mkdir -p /mnt/sdcard/%04d-%02d-%02d/photo",gs_time_info.year,gs_time_info.month,gs_time_info.day);
                        sprintf(file_path,"/mnt/sdcard/%04d-%02d-%02d/photo/%04d-%02d-%02dT%02d-%02d-%02d-%03d.jpeg",\
                        gs_time_info.year,gs_time_info.month,gs_time_info.day,\
                        gs_time_info.year,gs_time_info.month,gs_time_info.day,gs_time_info.hour,\
                        gs_time_info.min,gs_time_info.sec,gs_time_info.m_second);
                 }else{
                      sprintf(cmd,"mkdir -p /mnt/sdcard/photo");
                      sprintf(file_path,"/mnt/sdcard/photo/Pic_Snap%05d.jpeg",gs_photo_info.photo_total);
                      gs_photo_info.photo_total++;
                 }  
                SLOGI("cmd %s\n",cmd);
                system(cmd);
                ret=af_save_jpeg(file_path,ptr,len,1920,1080);
                if(!ret)
                {
                    logger_info("af_save_jpeg", "save done!\n");
                }
                else
                {
                    logger_info("af_save_jpeg", "save err!\n");
                }

                sprintf(cmd,"sync");
                system(cmd);
                sleep(1);
                SLOGI("photo %d  ret %d \n",gs_photo_info.photo_total,ret);
                //save_image_data(ptr,len,file_path);
            } 

            mpp_packet_deinit(&packet);

            //mpp_log_f("encoded frame %d size %d\n", p->frame_count, len);
            p->stream_size += len;
            p->frame_count++;

            if (p->pkt_eos) {
                mpp_log("found last packet\n");
                mpp_assert(p->frm_eos);
            }
        }

        // if (p->num_frames && p->frame_count >= p->num_frames) {
        //     mpp_log_f("encode max %d frames", p->frame_count);
        //     break;
        // }
        if (p->frm_eos && p->pkt_eos)
            break;
    }
RET:

    return ret;
}

static void *yuv_encode_and_send_video_thread1(void* parameter)
{
    MPP_RET ret = MPP_OK;
    MpiEncCmd *cmd = &gs_cmd_ctx;
    MpiEncData *p = NULL;
    MppPollType timeout = MPP_POLL_BLOCK;

    memset((void*)cmd, 0, sizeof(*cmd));
    if(1){
        cmd->width = VIDEO_FRAME_WIDTH;
        cmd->height = VIDEO_FRAME_HEIGHT;
    }else{
        cmd->width = VIDEO_RESIZE_WIDTH;
        cmd->height = VIDEO_RESIZE_HEIGHT;
    }
    //cmd->num_frames=0;
    gs_rtsp_info.venc_bitrate = 2 * 1024;

    cmd->target_bps=gs_rtsp_info.venc_bitrate *1024;
    
    cmd->format = MPP_FMT_YUV420SP; // MPP_FMT_YUV420SP;

    //if (gs_vedio_info.output_type == TYPE_H264) {
    if(1){
        cmd->type = MPP_VIDEO_CodingAVC; /* H.264 */
    }
    else {
        cmd->type = MPP_VIDEO_CodingHEVC; /* H.265 */   
    }

    ret = test_ctx_init(&p, cmd);
    if (ret) {
        SLOGI("test data init failed ret %d\n", ret);
        goto MPP_TEST_OUT0;
    }

    ret = test_res_init(p);
    if (ret) {
        printf("test resource init failed ret %d\n", ret);
        goto MPP_TEST_OUT0;
    }

    ret = test_mpp_init(p);
    if (ret) {
        printf("test mpp init failed ret %d\n", ret);
        goto MPP_TEST_OUT0;
    }
    
    ret = mpp_enc_setup(p,1);
    if (ret) {
        printf("mpp setup failed ret %d\n", ret);
        goto MPP_TEST_OUT0;
    }

    gs_enc_data = p;

    mpp_enc_run();	
    
    return RET_OK;

MPP_TEST_OUT0:
    test_mpp_deinit(p);
    test_res_deinit(p);
    test_ctx_deinit(&p); 
  		
    return NULL;
}

static void *yuv_encode_and_send_video_thread2(void* parameter)
{
    MPP_RET ret = MPP_OK;
    MpiEncCmd *cmd = &gs_cmd_ctx1;
    MpiEncData *p = NULL;
    MppPollType timeout = MPP_POLL_BLOCK;

    memset((void*)cmd, 0, sizeof(*cmd));
    
    cmd->width = VIDEO_FRAME_WIDTH;
    cmd->height = VIDEO_FRAME_HEIGHT;
    
    cmd->format = MPP_FMT_YUV420SP;
    cmd->type = MPP_VIDEO_CodingMJPEG; /* H.264 */
    //cmd->num_frames=1;
    cmd->target_bps=5 *1024*1024;

    ret = test_ctx_init(&p, cmd);
    if (ret) {
        mpp_err_f("test data init failed ret %d\n", ret);
        goto MPP_TEST_OUT1;
    }

    ret = test_res_init(p);
    if (ret) {
        mpp_err_f("test resource init failed ret %d\n", ret);
        goto MPP_TEST_OUT1;
    }

    SLOGI("mpi_enc_test encoder test start w %d h %d type %d\n",
            p->width, p->height, p->type);

    // encoder demo
    ret = test_mpp_init(p);
    if (ret) {
        mpp_err_f("test mpp init failed ret %d\n", ret);
        goto MPP_TEST_OUT1;
    }

    ret = mpp_enc_setup(p,2);
    if (ret) {
        mpp_err_f("mpp setup failed ret %d\n", ret);
        goto MPP_TEST_OUT1;
    }

    gs_enc_data1 = p;

    mpp_enc_run1();	
    SLOGI("%s OK\n",__func__);

    return RET_OK;

MPP_TEST_OUT1:
    test_mpp_deinit(p);
    test_res_deinit(p);
    test_ctx_deinit(&p); 
	
    return NULL;
}

static void *yuv_encode_and_send_video_thread3(void* parameter)
{
    // cpu_set_t mask;
	// int cpuid = 5;
	// CPU_ZERO(&mask);
	// CPU_SET(cpuid, &mask);
	// if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
	// {
	// 	printf("yuv_encode_and_send_video_thread1 init error\n");
	// }
    MPP_RET ret = MPP_OK;
    MpiEncCmd *cmd = &gs_cmd_ctx2;
    MpiEncData *p = NULL;
    MppPollType timeout = MPP_POLL_BLOCK;

    memset((void*)cmd, 0, sizeof(*cmd));
    
    cmd->width = VIDEO_FRAME_WIDTH;
    cmd->height = VIDEO_FRAME_HEIGHT;

    cmd->format = MPP_FMT_YUV420SP; // MPP_FMT_YUV420SP;
    if (gs_vedio_info.output_type == TYPE_H264) {
        cmd->type = MPP_VIDEO_CodingAVC; /* H.264 */
    }
    else {
        cmd->type = MPP_VIDEO_CodingHEVC; /* H.265 */   
    }

    cmd->target_bps = 5 * 1024 * 1024;
    ret = test_ctx_init(&p, cmd);
    if (ret) {
        mpp_err_f("test data init failed ret %d\n", ret);
        goto MPP_TEST_OUT2;
    }

    ret = test_res_init(p);
    if (ret) {
        mpp_err_f("test resource init failed ret %d\n", ret);
        goto MPP_TEST_OUT2;
    }

    SLOGI("mpi_enc_test encoder test start w %d h %d type %d\n",
            p->width, p->height, p->type);

    // encoder demo
    ret = test_mpp_init(p);
    if (ret) {
        mpp_err_f("test mpp init failed ret %d\n", ret);
        goto MPP_TEST_OUT2;
    }

    ret = mpp_enc_setup(p,3);
    if (ret) {
        mpp_err_f("mpp setup failed ret %d\n", ret);
        goto MPP_TEST_OUT2;
    }

    gs_enc_data2 = p;

    mpp_enc_run2();	
    SLOGI("%s OK\n",__func__);

    return RET_OK;

MPP_TEST_OUT2:
    test_mpp_deinit(p);
    test_res_deinit(p);
    test_ctx_deinit(&p); 
	
    return NULL;
}

int encode_frame_init(void)
{
    pthread_t tid_1,tid_2,tid_3;  
    MPP_RET ret = MPP_OK; 
#if defined RTSP_OUTPUT_ENABLE  || defined TS_OUTPUT_ENABLE 
    ret = pthread_create(&tid_1,NULL,&yuv_encode_and_send_video_thread1,gs_enc_data);;
    if (ret != 0) {
        printf("%s(Error): Create alg encode video thread1 failed %d\n",__func__,ret);
        return RET_ERR_FAILED;
    }
#endif

#ifdef PHOTO_SAVE_ENABLE    
//    ret = pthread_create(&tid_2,NULL,&yuv_encode_and_send_video_thread2,gs_enc_data1);;
//    if (ret != 0) {
//        printf("%s(Error): Create alg encode video thread2 failed %d\n",__func__,ret);
//        return RET_ERR_FAILED;
//    }
#endif 

#ifdef VIDEO_SAVE_ENABLE    
//    ret = pthread_create(&tid_3,NULL,&yuv_encode_and_send_video_thread3,gs_enc_data2);;
//    if (ret != 0) {
//        printf("%s(Error): Create alg encode video thread3 failed %d\n",__func__,ret);
//        return RET_ERR_FAILED;
//    }
#endif  
}

void encode_frame_deinit(void)
{
    MpiEncData *p = gs_enc_data;
    test_mpp_deinit(p);
    test_res_deinit(p);
    test_ctx_deinit(&p); 

    p = gs_enc_data1;

    test_mpp_deinit(p);
    test_res_deinit(p);
    test_ctx_deinit(&p); 

     p = gs_enc_data2;

    test_mpp_deinit(p);
    test_res_deinit(p);
    test_ctx_deinit(&p); 
     
}   
