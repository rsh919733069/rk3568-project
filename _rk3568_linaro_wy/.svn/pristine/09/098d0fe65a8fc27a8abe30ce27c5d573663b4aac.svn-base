


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WINDOWS
    #define FILE_ROOT                               "t:/"
#else
    #include <sched.h>
    #include <dlfcn.h>
    #define FILE_ROOT                               "/tmp/"
#endif // _WINDOWS

#include "hetero_interface.h"

#define TEST_SAMPLE_RGB_2_GRAY                      0
#define TEST_SAMPLE_VYUY_2_RGB_AND_GRAY             0
#define TEST_SAMPLE_RESIZE_THEN_DRAW                0
#define TEST_SAMPLE_OVERLAPPED_OSD                  0
#define TEST_SAMPLE_PERSTRANS                       0
#define TEST_SAMPLE_RESIZE                          0
#define TEST_SAMPLE_EXTRACT                         0
#define TEST_SAMPLE_DARK                            0
#define TEST_SAMPLE_HISTOGRAM                       0

static size_t i_load_test_data(const char *raw_file, unsigned char *rgba_buf, unsigned width, unsigned pitch)
{
    size_t rd = 0;
    FILE* raw = fopen(raw_file, "rb");

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
    {   // 否则就只能老老实实一行一行的处理
        unsigned height = (unsigned)len / width;
        for (unsigned i = 0; i < height; ++i)
            rd += fread(rgba_buf + i * pitch, 1, width, raw);
    }

    fclose(raw);
    return rd;
}

static size_t i_store_test_data(const char* raw_file, unsigned char* rgba_buf, unsigned width, unsigned pitch, size_t len)
{
    size_t wr = 0;
    FILE* raw = fopen(raw_file, "wb");

    if (raw == 0)
    {
        fprintf(stderr, "## What?! Why failed to create %s?\n", raw_file);
        return 0;
    }

    if (pitch == 0)
        // 对于无空隙的图像，可以一股脑直接干到底
        wr = fwrite(rgba_buf, 1, len, raw);
    else
    {   // 否则就只能老老实实一行一行的处理
        unsigned height =  (unsigned)len / width;
        for (unsigned i = 0; i < height; ++i)
            wr += fwrite(rgba_buf + i * pitch, 1, width, raw);
    }

    fclose(raw);
    return wr;
}

#if TEST_SAMPLE_RESIZE_THEN_DRAW

int main_test( )
{
    unsigned pitch;
    void* hetero_hdr;
    void* image_obj_i;
    void* image_obj_o;
    unsigned char* mapped;
    float algorithm_cost;

    const unsigned src_w = 640, src_h = 512;
    const unsigned dst_w = 1920, dst_h = 1080;

#ifdef _WINDOWS
    const pixel_channel_et fmt_chn_in  = e_pix_chn_4;
    const pixel_channel_et fmt_chn_out = e_pix_chn_4;
#else
    const pixel_channel_et fmt_chn_in  = e_pix_chn_3;
    const pixel_channel_et fmt_chn_out = e_pix_chn_3;
#endif

    const pixel_data_et fmt_dat_in = e_pix_data_norm_u08;
    const pixel_data_et fmt_dat_out = e_pix_data_norm_u08;

    const char* input_file_name[ ] = {
        "1920.1080.8.raw", "1920.1080.16.raw", "1920.1080.24.raw", "1920.1080.32.raw"
    };

    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void* libhetero = dlopen("libhetero.so", RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero = (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    const unsigned bwi = htr.hetero_calculate_pixel_width(fmt_chn_in, fmt_dat_in);
    const unsigned bwo = htr.hetero_calculate_pixel_width(fmt_chn_out, fmt_dat_out);

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    image_obj_i = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_general, src_w*src_h);
    if (image_obj_i == 0) return -1;

    image_obj_o = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_output, dst_w*dst_h*3);
    if (image_obj_o == 0) return -1;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, image_obj_i, e_mapping_write);
    if (mapped == 0) return -1;

    // 加载测试图像到映射区域，作为被缩小的图像
    if (i_load_test_data("640.512.8.raw", mapped, 0, 0) == 0)
        return -1;

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, image_obj_i, mapped);

    // // 映射出硬件输入缓冲
    // mapped = htr.hetero_buffer_map(hetero_hdr, image_obj_o, e_mapping_write);
    // if (mapped == 0) return -1;

    // // 加载测试图像到映射区域，虽然是同一幅图，但这次作为背景
    // if (i_load_test_data(input_file_name[fmt_chn_out], mapped,0, 0) == 0)
    //     return -1;

    // // 释放映射区域
    // htr.hetero_buffer_unmap(hetero_hdr, image_obj_o, mapped);

    // 完成缩放
    // if (htr.resize_then_draw_to(hetero_hdr, image_obj_i, image_obj_o, 300, 300, 640, 480, &algorithm_cost))
    //     return -1;
    // printf("// Resize then draw cost %f ms.\n", algorithm_cost);
     htr.extent_gray640x512_to_rgb1920x1072(hetero_hdr,image_obj_i,image_obj_o,&algorithm_cost);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, image_obj_o, e_mapping_read);
    if (mapped == 0) return -1;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "floating_1.raw", mapped,0, 0, dst_w * dst_h * 3);

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, image_obj_o, mapped);

    // 释放输出异构图像
    htr.hetero_buffer_release(hetero_hdr, image_obj_o);

    // 释放输入异构图像
    htr.hetero_buffer_release(hetero_hdr, image_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);

    return 0;
}

#endif // TEST_SAMPLE_RESIZE_THEN_DRAW

#if TEST_SAMPLE_OVERLAPPED_OSD

int main_test( )
{
    void* hetero_hdr;
    void* buf_obj_i;
    void* buf_obj_o;
    unsigned char* mapped;
    float algorithm_cost;
    const unsigned dst_w = 1920, dst_h = 1080;

    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void* libhetero = dlopen("libhetero.so", RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero = (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    buf_obj_i = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_input, dst_w * dst_h * 2);
    if (buf_obj_i == 0) return -1;

    buf_obj_o = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_general, dst_w * dst_h * 3);
    if (buf_obj_i == 0) return -1;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_i, e_mapping_write);
    if (mapped == 0) return -1;

    // 加载测试 OSD 层到映射区域
    if (i_load_test_data("1920.1080.rgb565.raw", mapped, 0, 0) == 0)
        return -1;

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_i, mapped);

    // 映射出硬件输入缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_o, e_mapping_write);
    if (mapped == 0) return -1;

    // 加载测试图像到映射区域，作为背景
    if (i_load_test_data("1920.1080.24.raw", mapped, 0, 0) == 0)
        return -1;

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_o, mapped);

    // 完成格式转换和叠加
    if (htr.overlapped_osd_buf_layer(hetero_hdr, buf_obj_i, buf_obj_o, dst_w, dst_h, &algorithm_cost))
        return -1;
    printf("// Overlap cost %f ms.\n", algorithm_cost);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_o, e_mapping_read);
    if (mapped == 0) return -1;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "overlapped_osd.raw", mapped, 0, 0, dst_w * dst_h * 3);

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_o, mapped);

    // 释放输出异构图像
    htr.hetero_buffer_release(hetero_hdr, buf_obj_o);

    // 释放输入异构图像
    htr.hetero_buffer_release(hetero_hdr, buf_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);

    return 0;
}

#endif // TEST_SAMPLE_OVERLAPPED_OSD

#if TEST_SAMPLE_RGB_2_GRAY

int main_1( )
{
    void* hetero_hdr;
    void* buf_obj_i;
    void* img_obj_i;
    void* buf_obj_o;
    unsigned char* mapped;
    float algorithm_cost;
    const unsigned src_w = 1920, src_h = 1080;

#ifdef _WINDOWS
    const pixel_channel_et fmt_chn_in = e_pix_chn_4;
#else
    const pixel_channel_et fmt_chn_in = e_pix_chn_3;
#endif

    const pixel_data_et fmt_dat_in = e_pix_data_norm_u08;
    const char* input_file_name[ ] = {
        "1920.1080.8.raw", "1920.1080.16.raw", "1920.1080.24.raw", "1920.1080.32.raw"
    };

    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void* libhetero = dlopen("libhetero.so", RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero = (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    const unsigned bw = htr.hetero_calculate_pixel_width(fmt_chn_in, fmt_dat_in);

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    buf_obj_i = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_input, src_w * src_h * bw);
    if (buf_obj_i == 0) return -1;

    img_obj_i = htr.hetero_image_build_from_buffer(hetero_hdr, buf_obj_i, e_image_type_input, src_w, src_h,\
     fmt_chn_in, fmt_dat_in);
    if (img_obj_i == 0) return -1;

    buf_obj_o = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_output, src_w * src_h);
    if (buf_obj_o == 0) return -1;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_i, e_mapping_write);
    if (mapped == 0) return -1;

    // 加载测试图像到映射区域，下例中将用此数据执行两次不同的透视变换
    if (i_load_test_data(input_file_name[fmt_chn_in], mapped, 0, 0) == 0)
        return -1;

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_i, mapped);

    // 完成灰度计算
    if (htr.rgb_img_to_gray_buf(hetero_hdr, img_obj_i, buf_obj_o, &algorithm_cost))
        return -1;
    printf("// Color (img) to gray cost %f ms.\n", algorithm_cost);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_o, e_mapping_read);
    if (mapped == 0) return -1;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "gray_i.raw", mapped, 0, 0, src_w * src_h);

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_o, mapped);

    // WIN 不支持 GRB24，不运行下例
#ifndef _WINDOWS

    if (htr.rgb_buf_to_gray_buf(hetero_hdr, buf_obj_i, buf_obj_o, src_w, src_h, &algorithm_cost))
        return -1;
    printf("// Color (buf) to gray cost %f ms.\n", algorithm_cost);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_o, e_mapping_read);
    if (mapped == 0) return -1;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "gray_b.raw", mapped, 0, 0, src_w * src_h);

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_o, mapped);

#endif // _WINDOWS

    // 释放输出异构数据
    htr.hetero_buffer_release(hetero_hdr, buf_obj_o);

    // 释放输入异构图像
    htr.hetero_image_release(hetero_hdr, img_obj_i);

    // 释放输入异构数据
    htr.hetero_buffer_release(hetero_hdr, buf_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);

    return 0;
}

#endif // TEST_SAMPLE_RGB_2_GRAY

#if TEST_SAMPLE_VYUY_2_RGB_AND_GRAY

int main( )
{
    void* hetero_hdr;
    void* buf_obj_i;
    void* buf_obj_c;
    void* buf_obj_g;
    unsigned char* mapped;
    float algorithm_cost;
    const unsigned src_w = 640, src_h = 512;

    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void* libhetero = dlopen("libhetero.so", RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero = (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    buf_obj_i = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_input, src_w * src_h * 2);
    if (buf_obj_i == 0) return -1;

    buf_obj_c = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_output, src_w * src_h * 3);
    if (buf_obj_c == 0) return -1;

    buf_obj_g = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_output, src_w * src_h);
    if (buf_obj_g == 0) return -1;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_i, e_mapping_write);
    if (mapped == 0) return -1;

    // 加载测试图像到映射区域，下例中将用此数据执行两次不同的透视变换
    if (i_load_test_data("640.512.vyuy.raw", mapped, 0, 0) == 0)
        return -1;

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_i, mapped);

    // 完成色域转换计算
    if (htr.vyuy_buf_to_rgb_and_gray_buf(hetero_hdr, buf_obj_i, buf_obj_c, buf_obj_g, src_w, src_h, &algorithm_cost))
        return -1;
    printf("// YUV to RGB/GRAY cost %f ms.\n", algorithm_cost);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_c, e_mapping_read);
    if (mapped == 0) return -1;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "vyuy2rgb.24.raw", mapped, 0, 0, src_w * src_h * 3);

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_c, mapped);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buf_obj_g, e_mapping_read);
    if (mapped == 0) return -1;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "vyuy2gray.raw", mapped, 0, 0, src_w * src_h);

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buf_obj_g, mapped);

    // 释放输出异构数据
    htr.hetero_buffer_release(hetero_hdr, buf_obj_g);

    // 释放输入异构图像
    htr.hetero_buffer_release(hetero_hdr, buf_obj_c);

    // 释放输入异构数据
    htr.hetero_buffer_release(hetero_hdr, buf_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);

    return 0;
}

#endif // TEST_SAMPLE_VYUY_2_RGB_AND_GRAY

#if TEST_SAMPLE_PERSTRANS

int main( )
{
    unsigned pitch;
    void* hetero_hdr;
    void* image_obj_i;
    void* image_obj_o;
    void* perstrans_hdr;
    unsigned char* mapped;
    float algorithm_cost;

    const unsigned src_w = 1920, src_h = 1080;
    const unsigned dst_w = 1920, dst_h = 1080;

#ifdef _WINDOWS
    const pixel_channel_et fmt_chn_in = e_pix_chn_4;
    const pixel_channel_et fmt_chn_out = e_pix_chn_4;
#else
    const pixel_channel_et fmt_chn_in = e_pix_chn_3;
    const pixel_channel_et fmt_chn_out = e_pix_chn_3;
#endif

    const pixel_data_et fmt_dat_in = e_pix_data_norm_u08;
    const pixel_data_et fmt_dat_out = e_pix_data_norm_u08;

    const char *input_file_name[ ] = {
        "1920.1080.8.raw", "1920.1080.16.raw", "1920.1080.24.raw", "1920.1080.32.raw"
    };

    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void* libhetero = dlopen("libhetero.so", RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero = (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    const unsigned bwi = htr.hetero_calculate_pixel_width(fmt_chn_in, fmt_dat_in);
    const unsigned bwo = htr.hetero_calculate_pixel_width(fmt_chn_out, fmt_dat_out);

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    image_obj_i = htr.hetero_image_acquire(hetero_hdr, e_image_type_input, src_w, src_h, fmt_chn_in, fmt_dat_in);
    if (image_obj_i == 0) return -2;

    image_obj_o = htr.hetero_image_acquire(hetero_hdr, e_image_type_output, dst_w, dst_h, fmt_chn_out, fmt_dat_out);
    if (image_obj_o == 0) return -3;

    // 初始化透视变换计算单元
    perstrans_hdr = htr.perstrans_prepare(hetero_hdr, image_obj_i, image_obj_o);
    if (perstrans_hdr == 0) return -4;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_i, e_mapping_write, &pitch);
    if (mapped == 0) return -5;

    // 加载测试图像到映射区域，下例中将用此数据执行两次不同的透视变换
    if (i_load_test_data(input_file_name[fmt_chn_in], mapped, src_w * bwi, pitch) == 0)
        return -6;

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_i, mapped);

#if 0
    // 原始图像四边形顶点坐标
    quad_cord_st from = {
        0,      0,
        1919,   0,
        0,      1079,
        1919,   1079
    };

    // 目标图像四边形顶点坐标
    quad_cord_st to = {
        -10,    -10,
        2000,   -10,
        20,      1059,
        1919,   1079
    };

    for (int r = 0; r < 100; ++r)
    {
        mapped = htr.hetero_image_map(hetero_hdr, image_obj_i, e_mapping_write, &pitch);
        htr.hetero_image_unmap(hetero_hdr, image_obj_i, mapped);
        htr.perstrans_with_quadcord(hetero_hdr, perstrans_hdr, &from, &to, &algorithm_cost);
        mapped = htr.hetero_image_map(hetero_hdr, image_obj_o, e_mapping_read, &pitch);
        htr.hetero_image_unmap(hetero_hdr, image_obj_o, mapped);
    }

    // 映射出硬件输出缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_o, e_mapping_read, &pitch);
    if (mapped == 0) return -8;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "spdt.raw", mapped, dst_w * bwo, pitch, dst_w * dst_h * bwo);

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_o, mapped);
#else
    // 原始图像四边形顶点坐标
    quad_cord_st from = {
        0,      0,
        1919,   0,
        0,      1079,
        1919,   1079
    };

    // 目标图像四边形顶点坐标
    quad_cord_st to = {
        -450,   -300,
        2300,   0,
        200,    700,
        1619,   1079
    };

    // 第一次透视变换，使用四边形定义的方式
    if (htr.perstrans_with_quadcord(hetero_hdr, perstrans_hdr, &from, &to, &algorithm_cost))
        return -7;

    // 映射出硬件输出缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_o, e_mapping_read, &pitch);
    if (mapped == 0) return -8;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "quad.raw", mapped, dst_w * bwo, pitch, dst_w * dst_h * bwo);

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_o, mapped);

    trans_mat_st transmat = {
        .a00 = -989711.125f,
        .a01 = 909811.688f,
        .a02 = 714.010620f,
        .a10 = -494855.5f,
        .a11 = -2274529,
        .a12 = -723.01123f,
        .a20 = 445370048,
        .a21 = -409415264,
        .a22 = -1625927
    };

    // 第二次透视变换，使用变换矩阵的方式
    if (htr.perstrans_with_transmat(hetero_hdr, perstrans_hdr, &transmat, &algorithm_cost))
        return -9;

    // 映射出硬件输出缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_o, e_mapping_read, &pitch);
    if (mapped == 0) return -10;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "tmat.raw", mapped, dst_w * bwo, pitch, dst_w * dst_h * bwo);

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_o, mapped);
#endif

    // 释放透视变换计算单元
    htr.perstrans_finish(hetero_hdr, perstrans_hdr);

    // 释放输出异构图像
    htr.hetero_image_release(hetero_hdr, image_obj_o);

    // 释放输入异构图像
    htr.hetero_image_release(hetero_hdr, image_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);
    
    return 0;
}

#endif // TEST_SAMPLE_PERSTRANS

#if TEST_SAMPLE_RESIZE

int main( )
{
    unsigned pitch;
    void* hetero_hdr;
    void* image_obj_i;
    void* image_obj_c;
    void* image_obj_g;
    void* image_obj_d;
    unsigned char* mapped;
    float algorithm_cost;

    const unsigned src_w = 1920, src_h = 1080;
    const unsigned dst_w = 1366, dst_h = 768;

#ifdef _WINDOWS
    const pixel_channel_et fmt_chn_in   = e_pix_chn_4;
    const pixel_channel_et fmt_chn_out1 = e_pix_chn_4;
    const pixel_channel_et fmt_chn_out2 = e_pix_chn_1;
    const pixel_channel_et fmt_chn_out3 = e_pix_chn_1;
#else
    const pixel_channel_et fmt_chn_in   = e_pix_chn_3;
    const pixel_channel_et fmt_chn_out1 = e_pix_chn_3;
    const pixel_channel_et fmt_chn_out2 = e_pix_chn_1;
    const pixel_channel_et fmt_chn_out3 = e_pix_chn_1;
#endif

    const pixel_data_et fmt_dat_in   = e_pix_data_norm_u08;
    const pixel_data_et fmt_dat_out1 = e_pix_data_norm_u08;
    const pixel_data_et fmt_dat_out2 = e_pix_data_norm_u08;
    const pixel_data_et fmt_dat_out3 = e_pix_data_gene_u08;

    const char* input_file_name[ ] = {
        "1920.1080.8.raw", "1920.1080.16.raw", "1920.1080.24.raw", "1920.1080.32.raw"
    };

    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void* libhetero = dlopen("libhetero.so", RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
}
    f_path_to_hetero = (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    const unsigned bwi = htr.hetero_calculate_pixel_width(fmt_chn_in, fmt_dat_in);
    const unsigned bo1 = htr.hetero_calculate_pixel_width(fmt_chn_out1, fmt_dat_out1);
    const unsigned bo2 = htr.hetero_calculate_pixel_width(fmt_chn_out2, fmt_dat_out2);
    const unsigned bo3 = htr.hetero_calculate_pixel_width(fmt_chn_out3, fmt_dat_out3);

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    image_obj_i = htr.hetero_image_acquire(hetero_hdr, e_image_type_input, src_w, src_h, fmt_chn_in, fmt_dat_in);
    if (image_obj_i == 0) return -2;

    image_obj_c = htr.hetero_image_acquire(hetero_hdr, e_image_type_output, dst_w, dst_h, fmt_chn_out1, fmt_dat_out1);
    if (image_obj_c == 0) return -3;

    image_obj_g = htr.hetero_image_acquire(hetero_hdr, e_image_type_output, dst_w, dst_h, fmt_chn_out2, fmt_dat_out2);
    if (image_obj_g == 0) return -4;

    image_obj_d = htr.hetero_image_acquire(hetero_hdr, e_image_type_output, dst_w, dst_h, fmt_chn_out3, fmt_dat_out3);
    if (image_obj_d == 0) return -5;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_i, e_mapping_write, &pitch);
    if (mapped == 0) return -6;

    // 加载测试图像到映射区域，下例中将用此数据执行两次不同的透视变换
    if (i_load_test_data(input_file_name[fmt_chn_in], mapped, src_w * bwi, pitch) == 0)
        return -7;

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_i, mapped);

    // 完成缩放
    if (htr.resize_with_image(hetero_hdr, image_obj_i, image_obj_c, image_obj_g, &algorithm_cost))
        return -8;
    printf("// Cost %f ms.\n", algorithm_cost);

    // 完成反置暗通道缩放
    if (htr.resize_with_image_dark(hetero_hdr, image_obj_i, image_obj_d, image_obj_g, &algorithm_cost))
        return -9;
    printf("// Cost %f ms.\n", algorithm_cost);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_c, e_mapping_read, &pitch);
    if (mapped == 0) return -10;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "resize_c.raw", mapped, dst_w * bo1, pitch, dst_w * dst_h * bo1);

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_c, mapped);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_g, e_mapping_read, &pitch);
    if (mapped == 0) return -11;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "resize_g.raw", mapped, dst_w * bo2, pitch, dst_w * dst_h * bo2);

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_g, mapped);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_d, e_mapping_read, &pitch);
    if (mapped == 0) return -12;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "resize_d.raw", mapped, dst_w * bo3, pitch, dst_w * dst_h * bo3);

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_d, mapped);

    // 释放输出异构图像
    htr.hetero_image_release(hetero_hdr, image_obj_c);
    htr.hetero_image_release(hetero_hdr, image_obj_g);
    htr.hetero_image_release(hetero_hdr, image_obj_d);

    // 释放输入异构图像
    htr.hetero_image_release(hetero_hdr, image_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);

    return 0;
}

#endif // TEST_SAMPLE_RESIZE

#if TEST_SAMPLE_EXTRACT

int main( )
{
    unsigned pitch;
    void* hetero_hdr;
    void* image_obj_i;
    void* buffer_obj_o;
    unsigned char* mapped;
    float algorithm_cost;

    const unsigned src_w = 1920, src_h = 1080;
    const unsigned dst_w = 416, dst_h = 416;

#ifdef _WINDOWS
    const pixel_channel_et fmt_chn_in = e_pix_chn_4;
#else
    const pixel_channel_et fmt_chn_in = e_pix_chn_3;
#endif

    const pixel_data_et fmt_dat_in = e_pix_data_norm_u08;
    const char* input_file_name[] = {
        "1920.1080.8.raw", "1920.1080.16.raw", "1920.1080.24.raw", "1920.1080.32.raw"
    };

    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void* libhetero = dlopen("libhetero.so", RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero = (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    const unsigned bw = htr.hetero_calculate_pixel_width(fmt_chn_in, fmt_dat_in);

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    image_obj_i = htr.hetero_image_acquire(hetero_hdr, e_image_type_input, src_w, src_h, fmt_chn_in, fmt_dat_in);
    if (image_obj_i == 0) return -1;

    buffer_obj_o = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_output, dst_w * dst_h * 3);
    if (buffer_obj_o == 0) return -1;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_i, e_mapping_write, &pitch);
    if (mapped == 0) return -1;

    // 加载测试图像到映射区域，下例中将用此数据执行两次不同的透视变换
    if (i_load_test_data(input_file_name[fmt_chn_in], mapped, src_w * bw, pitch) == 0)
        return -1;

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_i, mapped);

    // 完成截取缩放
    if (htr.resize_from_extracted(hetero_hdr, image_obj_i, buffer_obj_o, 424, 4, 1072, 1072, dst_w, dst_h, &algorithm_cost))
        return -1;
    printf("// Extract & resize cost %f ms.\n", algorithm_cost);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buffer_obj_o, e_mapping_read);
    if (mapped == 0) return -1;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "resize_c.raw", mapped, 0, 0, dst_w * dst_h * 3);

    // 释放映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buffer_obj_o, mapped);

    // 释放输出异构数据
    htr.hetero_buffer_release(hetero_hdr, buffer_obj_o);

    // 释放输入异构图像
    htr.hetero_image_release(hetero_hdr, image_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);

    return 0;
}

#endif // TEST_SAMPLE_EXTRACT

#if TEST_SAMPLE_DARK

int main( )
{
    unsigned pitch;
    void* hetero_hdr;
    void* image_obj_i;
    void* image_obj_o;
    unsigned char* mapped;
    float algorithm_cost;

    const unsigned src_w = 1366, src_h = 768;
    const unsigned dst_w = 1366, dst_h = 768;

    const pixel_channel_et fmt_chn_in = e_pix_chn_1;
    const pixel_channel_et fmt_chn_out = e_pix_chn_1;

    const pixel_data_et fmt_dat_in = e_pix_data_gene_u08;
    const pixel_data_et fmt_dat_out = e_pix_data_gene_u08;

    const char* input_file_name = "1366.768.dark.raw";
    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void * libhetero = dlopen("libhetero.so", RTLD_NOW|RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero =  (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    const unsigned bwi = htr.hetero_calculate_pixel_width(fmt_chn_in, fmt_dat_in);
    const unsigned bo1 = htr.hetero_calculate_pixel_width(fmt_chn_out, fmt_dat_out);

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    image_obj_i = htr.hetero_image_acquire(hetero_hdr, e_image_type_input, src_w, src_h, fmt_chn_in, fmt_dat_in);
    if (image_obj_i == 0) return -2;

    image_obj_o = htr.hetero_image_acquire(hetero_hdr, e_image_type_output, dst_w, dst_h, fmt_chn_out, fmt_dat_out);
    if (image_obj_o == 0) return -3;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_i, e_mapping_write, &pitch);
    if (mapped == 0) return -4;

    // 加载测试图像到映射区域，下例中将用此数据执行两次不同的透视变换
    if (i_load_test_data(input_file_name, mapped, src_w * bwi, pitch) == 0)
        return -5;

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_i, mapped);

    // 暗黑过滤
    if (htr.dark_filter_with_image(hetero_hdr, image_obj_i, image_obj_o, 7, &algorithm_cost))
        return -6;
    printf("// Cost %f ms.\n", algorithm_cost);

    // 映射出硬件输出缓冲
    mapped = htr.hetero_image_map(hetero_hdr, image_obj_o, e_mapping_read, &pitch);
    if (mapped == 0) return -7;

    // 从输出缓冲写生成的图像数据
    i_store_test_data(FILE_ROOT "filter_d.raw", mapped, dst_w * bo1, pitch, dst_w * dst_h * bo1);

    // 释放映射区域
    htr.hetero_image_unmap(hetero_hdr, image_obj_o, mapped);

    // 释放输出异构图像
    htr.hetero_image_release(hetero_hdr, image_obj_o);

    // 释放输入异构图像
    htr.hetero_image_release(hetero_hdr, image_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);

    return 0;
}

#endif // TEST_SAMPLE_DARK

#if TEST_SAMPLE_HISTOGRAM

static unsigned result[256];
static unsigned char cccp[1920 * 1080];

static void test_histogram_out( )
{
    i_load_test_data("1920.1080.8.raw", cccp, 1920, 0);
    for (int x = 0; x < 1920 * 1080; ++x)
        ++result[cccp[x]];
}

int main( )
{
    void* hetero_hdr;
    void* histogram_hdr;
    void* buffer_obj_i;
    void* buffer_obj_o;
    unsigned char* mapped;
    float algorithm_cost;
    const unsigned hist[2] = { 1920, 1080 };
    const char* input_file_name = "1920.1080.8.raw";
    hetero_world_st htr = { HETERO_INTERFACE_VER };
    PATH_TO_HETERO* f_path_to_hetero;

#ifdef _WINDOWS
    f_path_to_hetero = path_to_hetero;
#else
    void* libhetero = dlopen("libhetero.so", RTLD_NOW | RTLD_DEEPBIND);
    if (libhetero == 0)
    {
        printf("!! Could not find libhetero.so.\n");
        return -1;
    }
    f_path_to_hetero = (PATH_TO_HETERO*)dlsym(libhetero, "path_to_hetero");
    if (f_path_to_hetero == 0)
    {
        printf("!! Loading function failed.\n");
        return -1;
    }
    // 锁定线程的到大核
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
#endif // _WINDOWS

    // 联通异构计算库
    if (f_path_to_hetero(&htr) < 0)
    {
        printf("!! Library hetero interface not match.\n");
        return -1;
    }

    // 获取异构计算环境句柄
    hetero_hdr = htr.hetero_compute_acquire(0);
    if (hetero_hdr == 0) return -1;

    buffer_obj_i = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_input, hist[0] * hist[1]);
    if (buffer_obj_i == 0) return -1;

    buffer_obj_o = htr.hetero_buffer_acquire(hetero_hdr, e_buffer_type_output, 256 * sizeof(unsigned));
    if (buffer_obj_o == 0) return -1;

    histogram_hdr = htr.histogram_prepare(hetero_hdr, buffer_obj_i, buffer_obj_o, hist[0], hist[1]);
    if (histogram_hdr == 0) return -1;

    // 映射出硬件输入缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buffer_obj_i, e_mapping_write);
    if (mapped == 0) return -1;

    // 加载测试图像到映射区域，下例中将用此数据执行两次不同的透视变换
    if (i_load_test_data(input_file_name, mapped, hist[0], 0) == 0)
        return -1;

    // 撤销映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buffer_obj_i, mapped);

    // 直方图计算
    if (htr.histogram_calculate(hetero_hdr, histogram_hdr, &algorithm_cost))
        return -1;

    // 映射出硬件输出缓冲
    mapped = htr.hetero_buffer_map(hetero_hdr, buffer_obj_o, e_mapping_read);
    if (mapped == 0) return -1;

    htr.clock_stack_push(hetero_hdr);
    test_histogram_out( );
    printf("// CPU cost %f ms\n", htr.clock_stack_pop(hetero_hdr));

    // 对比输出数据
    if (memcmp(result, mapped, 256 * sizeof(unsigned)))
        printf("!! Histogram cal error!\n");
    else
    {
        // 输出直方图结果
        const unsigned* p = (unsigned*)mapped;
        for (int i = 0; i < 32; ++i)
        {
            printf("// [%02d] %7u %7u %7u %7u %7u %7u %7u %7u\n", i,
                p[8 * i + 0], p[8 * i + 1], p[8 * i + 2], p[8 * i + 3],
                p[8 * i + 4], p[8 * i + 5], p[8 * i + 6], p[8 * i + 7]);
        }
    }

    // 撤销映射区域
    htr.hetero_buffer_unmap(hetero_hdr, buffer_obj_o, mapped);

    // 释放直方图计算关联
    htr.histogram_finish(hetero_hdr, histogram_hdr);

    // 释放输出缓冲区
    htr.hetero_buffer_release(hetero_hdr, buffer_obj_o);

    // 释放输入缓冲区
    htr.hetero_buffer_release(hetero_hdr, buffer_obj_i);

    // 释放异构计算环境句柄
    htr.hetero_compute_release(hetero_hdr);

    return 0;
}

#endif // TEST_SAMPLE_HISTOGRAM
