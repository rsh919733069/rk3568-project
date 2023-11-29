#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sched.h>
#define FILE_ROOT "/tmp/"
#include "log.h"
#include "hetero_interface.h"

#include "data_engine.h"
#include "video_common.h"

static unsigned bwi;
static unsigned bo1;
size_t i_load_test_data(const char *raw_file, unsigned char *rgba_buf, unsigned width, unsigned pitch)
{
    size_t rd = 0;
    FILE *raw = fopen(raw_file, "rb");

    if (raw == 0)
    {
        fprintf(stderr, "## Could not open %s\n", raw_file);
        return 0;
    }

    fseek(raw, 0, SEEK_END);
    long len = ftell(raw);
    fseek(raw, 0, SEEK_SET);

    if (pitch == 0)
        // 对于无空隙的图像，可以一股脑直接干到底
        rd = fread(rgba_buf, 1, len, raw);
    else
    { // 否则就只能老老实实一行一行的处理
        unsigned height = (unsigned)len / width;
        for (unsigned i = 0; i < height; ++i)
            rd += fread(rgba_buf + i * pitch, 1, width, raw);
    }

    fclose(raw);
    return rd;
}

static size_t i_store_test_data(const char *raw_file, unsigned char *rgba_buf, unsigned width, unsigned pitch, size_t len)
{
    size_t wr = 0;
    FILE *raw = fopen(raw_file, "wb");

    if (raw == 0)
    {
        fprintf(stderr, "## What?! Why failed to create %s?\n", raw_file);
        return 0;
    }

    if (pitch == 0)
        // 对于无空隙的图像，可以一股脑直接干到底
        wr = fwrite(rgba_buf, 1, len, raw);
    else
    { // 否则就只能老老实实一行一行的处理
        unsigned height = (unsigned)len / width;
        for (unsigned i = 0; i < height; ++i)
            wr += fwrite(rgba_buf + i * pitch, 1, width, raw);
    }

    fclose(raw);
    return wr;
}

static size_t i_load_in_data(unsigned char *src_buf, unsigned char *rgba_buf, unsigned width, unsigned pitch, long len)
{

    if (pitch == 0)
        // 对于无空隙的图像，可以一股脑直接干到底
        memcpy(rgba_buf, src_buf, len);
    else
    { // 否则就只能老老实实一行一行的处理
        unsigned height = (unsigned)len / width;
        for (unsigned i = 0; i < height; ++i)
            // rd += fread(rgba_buf + i * pitch, 1, width, raw);
            memcpy(rgba_buf + i * pitch, src_buf + i * width, width);
    }
    return 1;
}
static size_t i_store_out_data(unsigned char *out_buf, unsigned char *rgba_buf, unsigned width, unsigned pitch, size_t len)
{

    if (pitch == 0)
        // 对于无空隙的图像，可以一股脑直接干到底
        // wr = fwrite(rgba_buf, 1, len, raw);
        memcpy(out_buf, rgba_buf, len);
    else
    { // 否则就只能老老实实一行一行的处理
        unsigned height = (unsigned)len / width;
        for (unsigned i = 0; i < height; ++i)
            //  wr += fwrite(rgba_buf + i * pitch, 1, width, raw);
            memcpy(out_buf + i * width, rgba_buf + i * pitch, width);
    }
    return 1;
}

int gpu_init(data_stream_t *data_stream)
{

    const unsigned src_w = VIDEO_FRAME_WIDTH, src_h = VIDEO_FRAME_HEIGHT_STRIDE;
    const unsigned dst_w = V5_RKNN_W, dst_h = V5_RKNN_H;

    const unsigned src_w_ir = IR_VIDEO_FRAME_WIDTH, src_h_ir = IR_VIDEO_FRAME_HEIGHT;

    const pixel_channel_et fmt_chn_in = e_pix_chn_3;
    const pixel_channel_et fmt_chn_out1 = e_pix_chn_1;

    const pixel_data_et fmt_dat_in = e_pix_data_norm_u08;
    const pixel_data_et fmt_dat_out1 = e_pix_data_norm_u08;

    int i = 0;
    PATH_TO_HETERO *f_path_to_hetero;
    void *libhetero = dlopen(RK_GPU_LIB_PATH, RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero = (PATH_TO_HETERO *)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 联通异构计算库
    if (f_path_to_hetero(&(data_stream->htr)) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    bwi = data_stream->htr.get_pixel_width(fmt_chn_in, fmt_dat_in);
    bo1 = data_stream->htr.get_pixel_width(fmt_chn_out1, fmt_dat_out1);
    // 获取异构计算环境句柄
    data_stream->hetero_hdr = data_stream->htr.compute_acquire(0);
    if (data_stream->hetero_hdr == 0)
        return -1;

    for (i = 0; i < DATA_STREAM_RGB_FRAME_MAX; i++)
    {
        data_stream->buf_obj_i_tv[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                       e_buffer_type_general, src_w * src_h * 3);
        if (data_stream->buf_obj_i_tv[i] == 0)
            return -2;

        data_stream->image_obj_i_tv[i] = data_stream->htr.image_build_from_buffer(data_stream->hetero_hdr,
                                                                                  data_stream->buf_obj_i_tv[i], e_image_type_general, src_w, src_h, fmt_chn_in, fmt_dat_in);
        if (data_stream->image_obj_i_tv[i] == 0)
            return -2;
    }

    for (i = 0; i < DATA_STREAM_RGB_FRAME_MAX; i++)
    {
        data_stream->buf_obj_i_ir[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                       e_buffer_type_general, src_w_ir * src_h_ir * 3);
        if (data_stream->buf_obj_i_ir[i] == 0)
            return -3;

        data_stream->image_obj_i_ir[i] = data_stream->htr.image_build_from_buffer(data_stream->hetero_hdr,
                                                                                  data_stream->buf_obj_i_ir[i], e_image_type_general, src_w_ir, src_h_ir, fmt_chn_in, fmt_dat_in);
        if (data_stream->image_obj_i_ir[i] == 0)
            return -3;
    }

    for (i = 0; i < DATA_STREAM_RGB_FRAME_MAX; i++)
    {
        data_stream->buf_obj_o_rgb1080[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                            e_buffer_type_general, src_w * src_h * 3);
        if (data_stream->buf_obj_o_rgb1080[i] == 0)
            return -3;

        data_stream->img_obj_o_rgb1080[i] = data_stream->htr.image_build_from_buffer(data_stream->hetero_hdr,
                                                                                     data_stream->buf_obj_o_rgb1080[i], e_image_type_general, src_w, src_h, fmt_chn_in, fmt_dat_in);
        if (data_stream->img_obj_o_rgb1080[i] == 0)
            return -3;
    }

    for (i = 0; i < DATA_STREAM_RGB_FRAME_MAX; i++)
    {
        data_stream->buf_obj_i_osd[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                        e_buffer_type_input, src_w * src_h * 2);
        if (data_stream->buf_obj_i_osd[i] == 0)
            return -4;
    }

    for (i = 0; i < DATA_STREAM_RGB_FRAME_MAX; i++)
    {
        data_stream->buf_obj_o_gray[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                         e_buffer_type_output, src_w * src_h);
        if (data_stream->buf_obj_o_gray[i] == 0)
            return -5;
    }

    for (i = 0; i < DATA_STREAM_RGB_FRAME_MAX; i++)
    {
        data_stream->ir_obj_o_gray[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                        e_buffer_type_general, src_w_ir * src_h_ir);
        if (data_stream->ir_obj_o_gray[i] == 0)
            return -5;

        data_stream->image_i_ir_gray[i] = data_stream->htr.image_build_from_buffer(data_stream->hetero_hdr,
                                                                                   data_stream->ir_obj_o_gray[i], e_image_type_general, src_w_ir, src_h_ir, fmt_chn_out1, fmt_dat_in);
        if (data_stream->image_i_ir_gray[i] == 0)
            return -3;
    }

    for (i = 0; i < DATA_STREAM_RGB_FRAME_MAX; i++)
    {
        data_stream->buf_obj_o_rgb416[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                           e_buffer_type_output, dst_w * dst_h * 3);
        if (data_stream->buf_obj_o_rgb416[i] == 0)
            return -6;
    }

    for (i = 0; i < DATA_STREAM_RGB_FRAME_MAX; i++)
    {

#ifdef CAMERA_INPUT_FORMAT_YVYU
        data_stream->buf_obj_i_vyuy[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                         e_buffer_type_input, src_w * src_h * 2);
        if (data_stream->buf_obj_i_vyuy[i] == 0)
            return -7;
#endif

#ifdef IR_INPUT_ENABLE

        data_stream->buf_obj_i_ir_tmp = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                        e_buffer_type_general, src_w_ir * src_h_ir * 3);
        if (data_stream->buf_obj_i_ir_tmp == 0)
            return -9;

        data_stream->img_obj_i_ir_tmp = data_stream->htr.image_build_from_buffer(data_stream->hetero_hdr,
                                                                                 data_stream->buf_obj_i_ir_tmp, e_image_type_general, src_w_ir, src_h_ir, fmt_chn_in, fmt_dat_in);
        if (data_stream->img_obj_i_ir_tmp == 0)
            return -9;

        data_stream->buf_obj_i_ir_vyuy[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                            e_buffer_type_input, src_w_ir * src_h_ir * 3);
        if (data_stream->buf_obj_i_ir_vyuy[i] == 0)
            return -7;
#endif

#ifdef VO_720P_ENABLE
        data_stream->buf_obj_o_rgb720[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                           e_buffer_type_general, VIDEO_RESIZE_WIDTH * VIDEO_RESIZE_HEIGHT * 3);
        if (data_stream->buf_obj_o_rgb720[i] == 0)
            return -8;

        data_stream->img_obj_o_rgb720[i] = data_stream->htr.image_build_from_buffer(data_stream->hetero_hdr,
                                                                                    data_stream->buf_obj_o_rgb720[i], e_image_type_general, VIDEO_RESIZE_WIDTH, VIDEO_RESIZE_HEIGHT, fmt_chn_in, fmt_dat_in);
        if (data_stream->img_obj_o_rgb720[i] == 0)
            return -8;

        data_stream->buf_obj_o_yuv720[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                           e_buffer_type_output, VIDEO_RESIZE_WIDTH * VIDEO_RESIZE_HEIGHT * 2);
        if (data_stream->buf_obj_o_yuv720[i] == 0)
            return -8;
#endif

        data_stream->buf_obj_o_yuv1080[i] = data_stream->htr.buffer_acquire(data_stream->hetero_hdr,
                                                                            e_buffer_type_output, 1920 * 1080 * 2);
        if (data_stream->buf_obj_o_yuv1080[i] == 0)
            return -8;
    }

    // data_stream->image_obj_i_ir_resize= data_stream->htr.hetero_image_acquire(data_stream->hetero_hdr, e_image_type_input, src_w, src_h, fmt_chn_in, fmt_dat_in);
    // if (data_stream->image_obj_i_ir_resize== 0) return -4;

    printf(" gpu init ok %d %d %x %x \n", bwi, bo1, data_stream->image_obj_i_tv[0], data_stream->buf_obj_o_gray[0]);
    return 0;
}

int gpu_extract_resize(unsigned char *src_buf, unsigned char *dst_buf, int times)
{
    // data_stream_t *data_stream = data_stream_get();
    // static unsigned pitch;
    // float algorithm_cost;
    // unsigned char* mapped;
    // // // 锁定线程的到大核
    // // cpu_set_t mask;
    // // CPU_ZERO(&mask);
    // // CPU_SET(5, &mask);
    // // sched_setaffinity(0, sizeof(mask), &mask);

    // data_stream->htr.clock_stack_push(data_stream->hetero_hdr);
    // // 映射出硬件输入缓冲
    // mapped = data_stream->htr.hetero_image_map(data_stream->hetero_hdr, data_stream->image_obj_i_tv[times], e_mapping_write, &pitch);
    // if (mapped == 0) return -6;
    // float t0=data_stream->htr.clock_stack_pop(data_stream->hetero_hdr);
    // // 加载测试图像到映射区域，下例中将用此数据执行两次不同的透视变换
    // // if (i_load_in_data(src_buf, mapped, src_w * bwi, pitch,src_w*src_h*3) == 0)
    // //     return -7;

    // data_stream->htr.clock_stack_push(data_stream->hetero_hdr);
    // rgb_copy_to_gpu(src_buf,mapped);
    // float t1=data_stream->htr.clock_stack_pop(data_stream->hetero_hdr);

    // data_stream->htr.clock_stack_push(data_stream->hetero_hdr);
    // // 释放映射区域
    // data_stream->htr.hetero_image_unmap(data_stream->hetero_hdr, data_stream->image_obj_i_tv[times], mapped);
    // float t2=data_stream->htr.clock_stack_pop(data_stream->hetero_hdr);

    // // 完成缩放
    // if (htr.resize_with_image(hetero_hdr, image_obj_i_tv[times%10], image_obj_o_rgb416[times%10], 0/*image_obj_g*/, &algorithm_cost))
    //     return -8;
    // SLOGI(" resize_from_extracted---- \n");
    // 完成截取缩放

    // // 完成反置暗通道缩放
    // if (htr.f_resize_with_image_dark(hetero_hdr, image_obj_i, image_obj_d, image_obj_g, &algorithm_cost))
    //     return -9;
    // printf("// Cost %f ms.\n", algorithm_cost);
    //     data_stream->htr.clock_stack_push(data_stream->hetero_hdr);
    //     // 映射出硬件输出缓冲
    //     mapped = data_stream->htr.hetero_image_map(data_stream->hetero_hdr, data_stream->image_obj_o_rgb416[times%10], e_mapping_read, &pitch);
    //     if (mapped == 0) return -10;
    //     //SLOGI("// Cost %f ms. %d \n", algorithm_cost,pitch);
    //     // 从输出缓冲写生成的图像数据
    //   //  i_store_test_data(FILE_ROOT "resize_c.raw", mapped, dst_w * bo1, pitch, dst_w * dst_h * bo1);
    //     float t3=data_stream->htr.clock_stack_pop(data_stream->hetero_hdr);

    //     data_stream->htr.clock_stack_push(data_stream->hetero_hdr);
    //     i_store_out_data(dst_buf, mapped, dst_w * bo1, pitch, dst_w * dst_h * bo1);
    //     float t4=data_stream->htr.clock_stack_pop(data_stream->hetero_hdr);

    // htr.clock_stack_push(hetero_hdr);
    // // 释放映射区域
    // htr.hetero_image_unmap(hetero_hdr, image_obj_o_rgb416[times%10], mapped);

    // float t5=htr.clock_stack_pop(hetero_hdr);
    // SLOGI(" %d map_i %f  copyi_to_gpu %f  unmap_i %f  algorithm_cost %f  map_c %f st_out %f unmap_c %f ms \n",\
    //  times,t0,t1,t2,algorithm_cost,t3,t4,t5);

    //  SLOGI(" %d map_i %f  copyi_to_gpu %f  unmap_i %f  algorithm_cost %f \n",times,t0,t1,t2,algorithm_cost);
    return 0;
}
int gpu_release()
{

    // int i=0;
    // for(i=0;i<DATA_STREAM_RGB_FRAME_MAX;i++){
    //      data_stream->hhtr.hetero_image_release(data_stream->hhetero_hdr,  image_obj_i_tv[i]);
    // }

    // for(i=0;i<DATA_STREAM_RGB_FRAME_MAX;i++){
    //     htr.hetero_image_release(hetero_hdr,  image_obj_i_ir[i]);
    // }

    // htr.hetero_image_release(hetero_hdr, image_obj_i_osd);

    // for(i=0;i<DATA_STREAM_RGB_FRAME_MAX;i++){
    //     htr.hetero_image_release(hetero_hdr, image_obj_o_gray[i]);
    // }

    // for(i=0;i<DATA_STREAM_RGB_FRAME_MAX;i++){
    //     htr.hetero_image_release(hetero_hdr, image_obj_o_rgb416[i]);
    // }

    // for(i=0;i<DATA_STREAM_RGB_FRAME_MAX;i++){
    //     htr.hetero_image_release(hetero_hdr, image_obj_o_rgb1080[i]);
    // }
    // // 释放异构计算环境句柄
    // htr.hetero_compute_release(hetero_hdr);
}
