


#ifndef HETERO_INTERFACE_202104151110_H
#define HETERO_INTERFACE_202104151110_H

#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#define restrict                                __restrict__
#endif

#define HETERO_INTERFACE_VER                    0x4210411221161752

#define C_PI                                    3.1415926535897932384626433832795f

/*
 * 和 FFTW 兼容的复数格式
 */
typedef struct
{
    float r;
    float i;
}
cxf32;

/*
 * 函数执行位标识
 */
typedef enum
{
    e_exc_none      = 0,
    e_exc_queue     = 1,
    e_exc_profile   = 2
}
hfc_flag_et;

/*
 * 透视变换四边形顶点定义
 */
typedef struct
{
    float v0x, v0y;     // 左上点
    float v1x, v1y;     // 右上点
    float v2x, v2y;     // 左下点
    float v3x, v3y;     // 右下点
}
quad_cord_st;

/*
 * 透视变换变换矩阵定义
 */
typedef struct
{
    float a00, a10, a20;
    float a01, a11, a21;
    float a02, a12, a22;
}
trans_mat_st;

/*
 * 由 u16 定义的区域
 *  (x1, y1) 为左上角
 *  (x2, y2) 为右下角
 * 注意这和一般的长宽表示法不相同
 */
typedef struct
{
    uint16_t x1, y1;
    uint16_t x2, y2;
}
area_u16_st;

typedef enum {
    e_image_type_input,             // 图像输入
    e_image_type_output,            // 图像输出
    e_image_type_general,           // 不同函数中可分别作为输入和输出
    e_image_type_no_access,         // 供异构算法内部缓存之用
    e_image_type_max
}
image_type_et;

typedef enum {
    e_buffer_type_input,            // 数据输入
    e_buffer_type_output,           // 数据输出
    e_buffer_type_general,          // 数据输入输出通用
    e_buffer_type_no_access,        // 供异构算法内部缓存之用
    e_buffer_type_max
}
buffer_type_et;

typedef enum {
    e_mapping_read,                 // GPU -> CPU
    e_mapping_write,                // CPU -> GPU
    e_mapping_max
}
memory_map_et;

/*
 * 图像通道组织
 */
typedef enum {
    e_pix_chn_1,
    e_pix_chn_2,
    e_pix_chn_3,
    e_pix_chn_4,
    e_pix_chn_max
}
pixel_channel_et;

/*
 * 图像数据格式
 */
typedef enum {
    e_pix_data_norm_u08,
    e_pix_data_norm_u16,
    e_pix_data_norm_s08,
    e_pix_data_norm_s16,
    e_pix_data_gene_u08,
    e_pix_data_gene_u16,
    e_pix_data_gene_u32,
    e_pix_data_gene_s08,
    e_pix_data_gene_s16,
    e_pix_data_gene_s32,
    e_pix_data_f16,
    e_pix_data_f32,
    e_pix_data_max
}
pixel_data_et;

/*
 * YUV 数据类型
 */
typedef enum {
    e_yuv_vyuy, // bit_0000
    e_yuv_uyvy, // bit_0001
    e_yuv_yvyu, // bit_0010
    e_yuv_yuyv, // bit_0011
    e_yuv_nv12, // bit_0100
    e_yuv_nv21  // bit_0101
}
e_yuv_format_et;

typedef int32_t (*hetero_so_puts_ft)(const char* out_str);

/*
 * 功能：
 *   创建异构缓冲
 *
 * 返回值：
 *   返回异构缓冲句柄
 */
void*
buffer_acquire(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 缓冲用途 */
    buffer_type_et buftp,
    /* 缓冲总字节大小 */
    uint64_t byte_size
);

/*
 * 功能：
 *   复制数据到异构缓冲。
 * 
 * 返回值：
 *   （将）实际复制字节数，参数错误时为负数。
 */
int64_t
buffer_copy_from(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构缓冲句柄 */
    void* buf_obj,
    /* 输入数据缓冲 */
    const void* restrict vm_addr,
    /* 数据字节容量 */
    uint64_t size,
    /* 执行标致 */
    hfc_flag_et flag
);

/*
 * 功能：
 *   从异构缓冲复制数据。
 * 
 * 返回值：
 *   （将）实际复制字节数，参数错误时为负数。
 */
int64_t
buffer_copy_to(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构缓冲句柄 */
    const void* buf_obj,
    /* 输出数据缓冲 */
    void* restrict vm_addr,
    /* 数据字节容量 */
    uint64_t size,
    /* 执行标致 */
    hfc_flag_et flag
);

/*
 * 功能：
 *   将满足对齐要求虚拟地址上缓冲导入为异构计算可访问的对象，
 *   速度较原生异构缓冲区慢，但无需 map/unmap 过程。
 *
 * 返回值：
 *   失败时返回0，否则返回异构缓冲句柄。
 */
void*
buffer_import_virtual(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 缓冲用途 */
    buffer_type_et buftp,
    /* 缓冲字节大小 */
    uint64_t buffer_size,
    /* 拟导入的虚拟地址 */
    void* restrict aligned_buf_va
);

/*
 * 功能：
 *   映射异构缓冲。若映射读，则该函数返回前对端会确保完成输出同步。
 *
 * 返回值：
 *   映射出的虚拟地址，每次调用返回的虚拟地址 *不一定* 相同。
 */
void*
buffer_map(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构缓冲句柄 */
    void* buffer_obj,
    /* 映射功能，读或写 */
    memory_map_et mapping_type
);

/*
 * 功能：
 *   回收异构缓冲
 */
void
buffer_release(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构图像句柄 */
    void* buffer_obj
);

/*
 * 功能：
 *   撤销异构缓冲映射。若映射写，则该函数返回前对端会确保已完成输入同步。
 */
void
buffer_unmap(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构缓冲句柄 */
    void* buffer_obj,
    /* 输入，映射出的虚拟地址 */
    void* mapped
);

/*
 * 功能：
 *   耗时结束计量。
 *
 * 返回值：
 *   push 与匹配的 pop 区间的毫秒耗时。
 */
float
clock_stack_pop(
    /* 输入，异构环境句柄 */
    void* hetero_hdr
);

/*
 * 功能：
 *   耗时启动计量。push 与 pop 成对使用，返回该区间的耗时，单位毫秒。
 *   可多层嵌套，如
 *       push( ) ... push( ) ... pop( ) ... pop( )
 *   可返回两层各自的耗时。
 */
void
clock_stack_push(
    /* 输入，异构环境句柄 */
    void* hetero_hdr
);

/*
 * 功能：
 *   关联可用的异构计算装置，比如 GPU。初始化异构计算环境，
 *   所有的计算函数都需要在特定的异构环境中才能执行。
 *
 * 返回值：
 *   异构计算环境句柄
 */
void *
compute_acquire(
    /* 输入，调试输出函数回调指针，若为 NULL 则使用库默认输出 */
    hetero_so_puts_ft so_puts
);

/*
 * 功能：
 *   复制异构计算环境，以此方式可开启多路计算模式，
 *   各路分享相同设备，降低设备等待数据而浪费的时间。
 *
 * 返回值：
 *   异构计算环境句柄
 */
void *
compute_duplicate(
    /* 输入，异构计算环境句柄 */
    const void* hetero_hdr
);

/*
 * 功能：
 *   清理异构计算环境。
 */
void
compute_release(
    /* 异构计算环境句柄 */
    void* hetero_hdr
);

/*
 * 功能：
 *   NV12/NV21 Semi-Plane 半平面格式数据转 RGB24 格式数据。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
convert_420sp_to_rgb(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，原始尺寸的异构数据句柄，连续存放的 420SP 数据 */
    const void* buf_obj_i,
    /* 输出，目标尺寸的异构数据句柄，映射后为连续缓冲区，像素类型为 RGB24 彩色 */
    void* buf_obj_o,
    /* 输入数据的格式 */
    e_yuv_format_et format,
    /* 图像像素宽度，必须是 8 的整数倍 */
    uint32_t w,
    /* 图像像素高度 */
    uint32_t h,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   YUV422 格式数据转 RGB24 & GRAY 格式数据。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
convert_422_to_gray_rgb(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，原始尺寸的异构数据句柄，连续存放的 VYUY 数据 */
    const void* buf_obj_i,
    /* 输出，目标尺寸的异构数据句柄，映射后为连续缓冲区，像素类型为 RGB24 彩色 */
    void* buf_obj_c,
    /* 输出，目标尺寸的异构数据句柄，映射后为连续缓冲区，像素类型为 UCHAR 灰度 */
    void* buf_obj_g,
    /* 输入数据的格式 */
    e_yuv_format_et format,
    /* 图像像素宽度，必须是 8 的整数倍 */
    uint32_t w,
    /* 图像像素高度 */
    uint32_t h,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   RGB24 格式数据转 NV12/NV21 Semi-Plane 半平面格式数据。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
convert_rgb_to_420sp(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，原始尺寸的异构数据句柄，映射后为连续缓冲区，像素类型为 RGB24 彩色 */
    const void* buf_obj_i,
    /* 输出，目标尺寸的异构数据句柄，连续存放的 420SP 数据 */
    void* buf_obj_o,
    /* 输出数据的格式 */
    e_yuv_format_et format,
    /* 图像像素宽度，必须是 8 的整数倍 */
    uint32_t w,
    /* 图像像素高度 */
    uint32_t h,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   RGB24 格式数据转 GRAY 格式数据。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
convert_rgb_to_gray(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，原始尺寸的异构数据句柄，连续存放的 RGB24 数据 */
    const void* buf_obj_i,
    /* 输出，目标尺寸的异构数据句柄，映射后为连续缓冲区，像素类型为 UCHAR */
    void* buf_obj_o,
    /* 图像像素宽，必须是 8 的倍数 */
    uint32_t w,
    /* 图像像素高 */
    uint32_t h,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   暗通道极值卷积。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
dark_filter_with_image(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构图像句柄，需要 e_pix_data_gene_u08 数据类型，反置暗图 */
    const void* img_obj_i,
    /* 输出，暗通道卷积后的异构图像句柄，需要 e_pix_data_gene_u08 数据类型 */
    void* img_obj_o,
    /* 卷积半轴长度 */
    uint32_t range,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   一维 FFT 逆计算。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
fft_1d_backword(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，FFT 计算句柄 */
    void* fft_hdr,
    /* 输入，数据句柄 */
    const void* buf_obj_i,
    /* 输出，数据句柄 */
    void* buf_obj_o,
    /* 单行数据的数据个数 */
    uint64_t size,
    /* 数据行数 */
    uint64_t lines,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   一维 FFT 正计算。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
fft_1d_forword(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，FFT 计算句柄 */
    void* fft_hdr,
    /* 输入，数据句柄 */
    const void* buf_obj_i,
    /* 输出，数据句柄 */
    void* buf_obj_o,
    /* 单行数据的数据个数 */
    uint64_t size,
    /* 数据行数 */
    uint64_t lines,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   回收 FFT 计算句柄
 */
void
fft_finish(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，FFT 计算句柄 */
    void* fft_hdr
);

/*
 * 功能：
 *   初始化 FFT 计算。
 *
 * 返回值：
 *   FFT 计算句柄，在配置不变的条件下可反复调用该算法。
 */
void*
fft_prepare(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 一维 FFT 计算的最大尺寸，必须是 2^N */
    uint64_t max_size
);

/*
 * 功能：
 *   计算指定格式的像素字节宽度。
 *
 * 返回值：
 *   计算所需的像素字节宽度
 */
uint32_t
get_pixel_width(
    /* 通道类型 */
    pixel_channel_et chn, 
    /* 数据格式 */
    pixel_data_et fmt
);

/*
 * 功能：
 *   创建异构图像。图像对象是一种复杂组织的数据，区别于普通的缓冲区，
 *   专门用于像素浮点计算，比如含有插值、变形等要求的高精度计算。
 * 
 * 返回值：
 *   异构图像句柄
 */
void *
image_acquire(
    /* 输入，异构环境句柄 */
    void* hetero_hdr, 
    /* 图像用途 */
    image_type_et imgtp, 
    /* 像素宽度 */
    uint32_t img_w, 
    /* 像素高度 */
    uint32_t img_h, 
    /* 像素格式 */
    pixel_channel_et pix_fmt, 
    /* 数据格式 */
    pixel_data_et dat_c
);

/*
 * 功能：
 *   从已创建的异构缓冲对象中创建异构图像，两者共用同一片内存区间，
 *   可同时使用两者的操作函数，从而避免数据搬移。
 *   该尺寸的图像对象的行填充必须为 0 否则会出现错误。
 *
 * 返回值：
 *   异构图像句柄
 */
void *
image_build_from_buffer(
    /* 输入，异构环境句柄 */
    void* hetero_hdr, 
    /* 输入，异构缓冲句柄 */
    void* buf_obj, 
    /* 图像用途 */
    image_type_et imgtp, 
    /* 像素宽度 */
    uint32_t img_w, 
    /* 像素高度 */
    uint32_t img_h, 
    /* 像素格式 */
    pixel_channel_et pix_fmt, 
    /* 数据格式 */
    pixel_data_et dat_c
);

/*
 * 功能：
 *   复制数据到异构图像。
 *
 * 返回值：
 *   （将）实际复制字节数，参数错误时为负数。
 */
int64_t
image_copy_from(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构图像句柄 */
    void* img_obj,
    /* 输入数据缓冲 */
    const void* restrict vm_addr,
    /* 缓冲去字节容量 */
    uint64_t buf_size,
    /* 输入缓冲区每行字节数，用于行填充区的处理，可以为 0 取像素字节数与行宽之积 */
    uint64_t row_pitch,
    /* 执行标致 */
    hfc_flag_et flag
);

/*
 * 功能：
 *   从异构图像复制数据。
 *
 * 返回值：
 *   （将）实际复制字节数，参数错误时为负数。
 */
int64_t
image_copy_to(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构图像句柄 */
    const void* img_obj,
    /* 输出数据缓冲 */
    void* restrict vm_addr,
    /* 缓冲去字节容量 */
    uint64_t buf_size,
    /* 输出缓冲区每行字节数，用于行填充区的处理，可以为 0 取像素字节数与行宽之积 */
    uint64_t row_pitch,
    /* 执行标致 */
    hfc_flag_et flag
);

/*
 * 功能：
 *   映射异构图像。若映射读，则函数返回前对端会确保完成输出同步。
 * 
 * 返回值：
 *   映射出的虚拟地址，每次调用返回的虚拟地址 *不一定* 相同。
 */
void *
image_map(
    /* 输入，异构环境句柄 */
    void *hetero_hdr,
    /* 输入，异构图像句柄 */
    void *image_obj,
    /* 映射功能 */
    memory_map_et mapping_type,
    /* 输出，为 0 表示映射出的空间无间隙，否则其数值表示实际行宽字节数 */
    uint32_t *pitch
);

/*
 * 功能：
 *   回收异构图像
 */
void
image_release(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构图像句柄 */
    void* image_obj
);

/*
 * 功能：
 *   撤销异构图像映射。若映射写，则函数返回前对端会确保完成输入同步。
 */
void
image_unmap(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 异构图像句柄 */
    void* image_obj,
    /* 输入，映射出的虚拟地址 */
    void* mapped
);

/*
 * 功能：
 *   将相同像素尺寸的字符层和图像层（BUF格式）进行叠加。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
overlapped_osd_buf_layer(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构数据对象，作为字符，使用 RGB565 格式 */
    const void* buf_obj_i,
    /* 输出，异构数据对象，作为背景 */
    void* buf_obj_o,
    /* 图像像素宽，必须是 8 的倍数 */
    uint32_t w,
    /* 图像像素高 */
    uint32_t h,
    /* 透明色，buf_obj_i 中为该数值的像素将变为透明 */
    uint32_t pat,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   通过四边形坐标完成透视变换。
 * 
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
perstrans_with_quadcord(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，透视算法输入的异构图像句柄，需要 e_pix_data_norm_u08 数据类型 */
    void* img_obj_i,
    /* 输入，透视算法输出的异构图像句柄，需要 e_pix_data_norm_u08 数据类型 */
    void* img_obj_o,
    /* 输入，原始四边形，没有边沿限制，坐标可以是任意点，比如负数或超过图像宽度与高度 */
    const quad_cord_st* from,
    /* 输入，目标四边形，没有边沿限制，坐标可以是任意点，比如负数或超过图像宽度与高度 */
    const quad_cord_st* to,
    /* 执行特性标志位 */
    hfc_flag_et flags
);

/*
 * 功能：
 *   通过转换矩阵完成透视变换。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
perstrans_with_transmat(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，透视算法输入的异构图像句柄，需要 e_pix_data_norm_u08 数据类型 */
    void* img_obj_i,
    /* 输入，透视算法输出的异构图像句柄，需要 e_pix_data_norm_u08 数据类型 */
    void* img_obj_o,
    /* 输入，3x3 变换矩阵 */
    const trans_mat_st* transmat,
    /* 执行特性标志位 */
    hfc_flag_et flags
);

/*
 * 功能：
 *   定制的四倍 GRAY 图像放大至 RGB 图，目标图长宽尺寸需要大于原图两倍。
 *   采用 1:4 像素放大填充，非插值计算。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
quad_extent_gray_to_rgb(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构数据对象，GRAY 图 */
    const void* buf_obj_i,
    /* 输出，异构数据对象，RGB24 图 */
    void* buf_obj_o,
    /* 原图像素宽，必须是 8 整数倍 */
    uint32_t sw,
    /* 原图像素高 */
    uint32_t sh,
    /* 目标像素宽，必须是 16 整数倍 */
    uint32_t dw,
    /* 目标像素高 */
    uint32_t dh,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   定制的四倍 RGB 图像放大至 RGB 图，目标图长宽尺寸需要大于原图两倍。
 *   采用 1:4 像素放大填充，非插值计算。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
quad_extent_rgb_to_rgb(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，异构数据对象，RGB24 图 */
    const void* buf_obj_i,
    /* 输出，异构数据对象，RGB24 图 */
    void* buf_obj_o,
    /* 原图像素宽，必须是 8 整数倍 */
    uint32_t sw,
    /* 原图像素高 */
    uint32_t sh,
    /* 目标像素宽，必须是 16 整数倍 */
    uint32_t dw,
    /* 目标像素高 */
    uint32_t dh,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   截取区域并缩放图像。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
resize_from_extracted(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，原始尺寸的异构图像句柄，需要 e_pix_chn_3/4 通道类型和 e_pix_data_norm_u08 数据类型 */
    const void* img_obj_i,
    /* 输出，目标尺寸的异构数据句柄，映射后为连续缓冲区，像素类型为 RGB888 */
    void* buf_obj_rgba_o,
    /* 截取框左上角 X 坐标，可为负数表示在图像外截取黑边 */
    float ex,
    /* 截取框左上角 Y 坐标，可为负数表示在图像外截取黑边 */
    float ey,
    /* 截取框像素宽，超出图像区域采用黑边填充 */
    float ew,
    /* 截取框像素高，超出图像区域采用黑边填充 */
    float eh,
    /* 缩放后的像素宽 */
    uint32_t dw,
    /* 缩放后的像素高 */
    uint32_t dh,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   缩放图像 A 并填充至图像 B 指定位置。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
resize_then_draw_to(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，被缩放的异构图像句柄，需要 e_pix_chn_3/4 通道类型和 e_pix_data_norm_u08 数据类型 */
    const void* img_obj_i,
    /* 输出，被填充的异构数据句柄 */
    void* img_obj_o,
    /* 填充区域左上角 X 坐标 */
    uint32_t dx,
    /* 填充区域左上角 Y 坐标 */
    uint32_t dy,
    /* 填充区域的像素宽 */
    uint32_t dw,
    /* 填充区域的像素高 */
    uint32_t dh,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   缩放图像。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
resize_with_image(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，原始尺寸的异构图像句柄 */
    const void* img_obj_i,
    /* 输出，目标尺寸的异构图像句柄，彩图，为 NULL 时忽略 */
    void* img_obj_rgba_o,
    /* 输出，目标尺寸的异构图像句柄，灰图，需要与 image_obj_rgba_o 相同图像尺寸，为 NULL 时忽略 */
    void* img_obj_gray_o,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   缩放图像，反置暗通道专用。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
resize_with_image_dark(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，原始尺寸的异构图像句柄，需要 e_pix_chn_3/4 通道类型和 e_pix_data_norm_u08 数据类型 */
    const void* img_obj_i,
    /* 输出，目标尺寸的异构图像句柄，需要 e_pix_data_gene_u08 数据类型，反置暗图 */
    void* img_obj_dark_o,
    /* 输出，目标尺寸的异构图像句柄，需要 e_pix_data_norm_u08 数据类型，灰图，需要与 image_obj_dark_o 相同图像尺寸 */
    void* img_obj_gray_o,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   进行 Shi-Tomas 特征点计算。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
shitomas_corners(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，Shi-Tomas 计算对象句柄 */
    void* shitomas_hdr,
    /* 输入，异构数据对象，单字节灰图 */
    const void* buf_obj_i,
    /* 输出，返回有效顶点的坐标，Y 为高 16 位，X 为低 16 位 */
    uint32_t* point_xy,
    /* 上述顶点坐标的数量 */
    uint32_t* valid_sz,
    /* 排除区域列表，可用于筛掉输入中的背景字符和目标框等干扰，可为 0 */
    const area_u16_st* except_list,
    /* 上述列表项数 */
    uint32_t except_num,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   用于 Shi-Tomas 算法的清理工作。
 */
void
shitomas_finish(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，Shi-Tomas 算法句柄 */
    void* shitomas_hdr
);

/*
 * 功能：
 *   用于 Shi-Tomas 算法的准备工作。
 *
 * 返回值：
 *   Shi-Tomas 计算句柄，在配置不变的条件下可反复调用该算法。
 */
void*
shitomas_prepare(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 图像像素宽 */
    uint32_t w,
    /* 图像像素高 */
    uint32_t h,
    /* 图像有效区域划定，可用于排除边框等无效区域，为 0 使用默认值 */
    const area_u16_st* enabled_area,
    /* 和计算相关的调优参数，为 0 使用默认值 */
    uint32_t hint,
    /* 最大顶点数 */
    uint32_t max_corners,
    /* 质量参数，需满足 0 < quality_level < 1 条件 */
    float quality_level,
    /* 顶点最小间距 */
    float min_distance
);

/*
 * 功能：
 *   用于 Wavelet-Fuse 算法的清理工作。
 */
void
wavelet_finish(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，Wavelet-Fuse 算法句柄 */
    void* wavelet_hdr
);

/*
 * 功能：
 *   进行 Wavelet-Fuse 计算。
 *
 * 返回值：
 *   0 表示成功，负数表示失败。
 */
int32_t
wavelet_fuse(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，Wavelet-Fuse 计算对象句柄 */
    void* wavelet_hdr,
    /* 输出，单位为毫秒的耗时测量输出，为 NULL 时不测量 */
    float* cost
);

/*
 * 功能：
 *   专用于 Wavelet-Fuse 算法的准备工作，红外与可见光 1:1 融合版。
 *
 * 返回值：
 *   Wavelet-Fuse 计算句柄，在配置不变的条件下可反复调用该算法。
 */
void *
wavelet_prepare_fixed(
    /* 输入，异构环境句柄 */
    void *hetero_hdr,
    /* 输入，红外异构数据句柄，尺寸需要与可见光一致，格式为单字节灰度 */
    const void *buf_obj_ir,
    /* 输入输出数据句柄，输入时含可见光灰度数据，输出时更新为融合数据，格式为单字节灰度 */
    void *buf_obj_vf,
    /* 图像数据像素宽 */
    uint32_t w,
    /* 图像数据像素高 */
    uint32_t h
);

/*
 * 功能：
 *   用于 Wavelet-Fuse 算法的准备工作，红外缩放到可见光尺寸后再融合版。
 *
 * 返回值：
 *   Wavelet-Fuse 计算句柄，在配置不变的条件下可反复调用该算法。
 */
void *
wavelet_prepare_ratio(
    /* 输入，异构环境句柄 */
    void* hetero_hdr,
    /* 输入，红外异构图像句柄，将被缩放到可见光相同大小后融合 */
    const void* img_obj_ir,
    /* 输入输出数据句柄，输入时含可见光灰度数据，输出时更新为融合数据，格式为单字节灰度 */
    void* buf_obj_vf,
    /* 可见光图像数据像素宽 */
    uint32_t w,
    /* 可见光图像数据像素高 */
    uint32_t h
);

/* 定义集体访问指针 */
typedef void *      (BUFFER_ACQUIRE)(void*, buffer_type_et, uint64_t);
typedef int64_t     (BUFFER_COPY_FROM)(void*, void*, const void* restrict, uint64_t, hfc_flag_et);
typedef int64_t     (BUFFER_COPY_TO)(void*, const void*, void* restrict, uint64_t, hfc_flag_et);
typedef void *      (BUFFER_IMPORT_VIRTUAL)(void*, buffer_type_et, unsigned, void* restrict);
typedef void *      (BUFFER_MAP)(void*, void*, memory_map_et);
typedef void        (BUFFER_RELEASE)(void*, void*);
typedef void        (BUFFER_UNMAP)(void*, void*, void*);
typedef float       (CLOCK_STACK_POP)(void *);
typedef void        (CLOCK_STACK_PUSH)(void *);
typedef void*       (COMPUTE_ACQUIRE)(hetero_so_puts_ft);
typedef void*       (COMPUTE_DUPLICATE)(const void*);
typedef void        (COMPUTE_RELEASE)(void*);
typedef int32_t     (CONVERT_420SP_TO_RGB)(void*, const void*, void*, e_yuv_format_et, int, int, float*);
typedef int32_t     (CONVERT_422_TO_GRAY_RGB)(void*, const void*, void*, void*, e_yuv_format_et, uint32_t, uint32_t, float*);
typedef int32_t     (CONVERT_RGB_TO_420SP)(void*, const void*, void*, e_yuv_format_et, int, int, float*);
typedef int32_t     (CONVERT_RGB_TO_GRAY)(void*, const void*, void*, int, int, float*);
typedef int32_t     (DARK_FILTER_WITH_IMAGE)(void *, const void *, void *, int, float *);
typedef int32_t     (FFT_1D_CAL)(void *, void *, const void *, void *, uint64_t, uint64_t, float *);
typedef void        (FFT_FINISH)(void *, void *);
typedef void *      (FFT_PREPARE)(void *, uint64_t);
typedef uint32_t    (GET_PIXEL_WIDTH)(pixel_channel_et, pixel_data_et);
typedef void *      (IMAGE_ACQUIRE)(void *, image_type_et, unsigned, unsigned, pixel_channel_et, pixel_data_et);
typedef void *      (IMAGE_BUILD_FROM_BUFFER)(void *, void *, image_type_et, unsigned, unsigned, pixel_channel_et, pixel_data_et);
typedef int64_t     (IMAGE_COPY_FROM)(void*, void*, const void* restrict, uint64_t, uint64_t, hfc_flag_et);
typedef int64_t     (IMAGE_COPY_TO)(void*, const void*, void* restrict, uint64_t, uint64_t, hfc_flag_et);
typedef void *      (IMAGE_MAP)(void*, void*, memory_map_et, unsigned*);
typedef void        (IMAGE_RELEASE)(void *, void *);
typedef void        (IMAGE_UNMAP)(void *, void *, void *);
typedef int32_t     (OVERLAPPED_OSD_BUF_LAYER)(void*, const void*, void*, int, int, int, float*);
typedef int32_t     (PERSTRANS_WITH_QUADCORD)(void*, void*, void*, const quad_cord_st*, const quad_cord_st*, hfc_flag_et);
typedef int32_t     (PERSTRANS_WITH_TRANSMAT)(void*, void*, void*, const trans_mat_st*, hfc_flag_et);
typedef int32_t     (QUAD_EXTENT_GRAY_TO_RGB)(void *, const void *, void *, int, int, int, int, float *);
typedef int32_t     (QUAD_EXTENT_RGB_TO_RGB)(void *, const void *, void *, int, int, int, int, float *);
typedef int32_t     (RESIZE_FROM_EXTRACTED)(void *, const void *, void *, float, float, float, float, uint32_t, uint32_t, float *);
typedef int32_t     (RESIZE_THEN_DRAW_TO)(void *, const void *, void *, uint32_t, uint32_t, uint32_t, uint32_t, float *);
typedef int32_t     (RESIZE_WITH_IMAGE)(void *, const void *, void *, void *, float *);
typedef int32_t     (RESIZE_WITH_IMAGE_DARK)(void *, const void *, void *, void *, float *);
typedef int32_t     (SHITOMAS_CORNERS)(void *, void *, const void *, unsigned *, unsigned *, const area_u16_st *, unsigned, float *);
typedef void        (SHITOMAS_FINISH)(void *, void *);
typedef void *      (SHITOMAS_PREPARE)(void *, unsigned, unsigned, const area_u16_st *, unsigned, unsigned, float, float);
typedef void        (WAVELET_FINISH)(void *, void *);
typedef int32_t     (WAVELET_FUSE)(void *, void *, float *);
typedef void *      (WAVELET_PREPARE_FIXED)(void *, const void *, void *, unsigned, unsigned);
typedef void *      (WAVELET_PREPARE_RATIO)(void *, const void *, void *, unsigned, unsigned);

typedef struct
{
    // 非排序固定部分
    uint64_t                                library_version;
    void *                                  reserved_pointer[7];
    // 排序列表部分
    BUFFER_ACQUIRE *                        buffer_acquire;
    BUFFER_COPY_FROM *                      buffer_copy_from;
    BUFFER_COPY_TO *                        buffer_copy_to;
    BUFFER_IMPORT_VIRTUAL *                 buffer_import_virtual;
    BUFFER_MAP *                            buffer_map;
    BUFFER_RELEASE *                        buffer_release;
    BUFFER_UNMAP *                          buffer_unmap;
    CLOCK_STACK_POP *                       clock_stack_pop;
    CLOCK_STACK_PUSH *                      clock_stack_push;
    COMPUTE_ACQUIRE *                       compute_acquire;
    COMPUTE_DUPLICATE *                     compute_duplicate;
    COMPUTE_RELEASE *                       compute_release;
    CONVERT_420SP_TO_RGB *                  convert_420sp_to_rgb;
    CONVERT_422_TO_GRAY_RGB *               convert_422_to_gray_rgb;
    CONVERT_RGB_TO_420SP *                  convert_rgb_to_420sp;
    CONVERT_RGB_TO_GRAY *                   convert_rgb_to_gray;
    DARK_FILTER_WITH_IMAGE *                dark_filter_with_image;
    FFT_1D_CAL *                            fft_1d_backword;
    FFT_1D_CAL *                            fft_1d_forword;
    FFT_FINISH *                            fft_finish;
    FFT_PREPARE *                           fft_prepare;
    GET_PIXEL_WIDTH *                       get_pixel_width;
    IMAGE_ACQUIRE *                         image_acquire;
    IMAGE_BUILD_FROM_BUFFER *               image_build_from_buffer;
    IMAGE_COPY_FROM *                       image_copy_from;
    IMAGE_COPY_TO *                         image_copy_to;
    IMAGE_MAP *                             image_map;
    IMAGE_RELEASE *                         image_release;
    IMAGE_UNMAP *                           image_unmap;
    OVERLAPPED_OSD_BUF_LAYER *              overlapped_osd_buf_layer;
    PERSTRANS_WITH_QUADCORD *               perstrans_with_quadcord;
    PERSTRANS_WITH_TRANSMAT *               perstrans_with_transmat;
    QUAD_EXTENT_GRAY_TO_RGB *               quad_extent_gray_to_rgb;
    QUAD_EXTENT_RGB_TO_RGB *                quad_extent_rgb_to_rgb;
    RESIZE_FROM_EXTRACTED *                 resize_from_extracted;
    RESIZE_THEN_DRAW_TO *                   resize_then_draw_to;
    RESIZE_WITH_IMAGE *                     resize_with_image;
    RESIZE_WITH_IMAGE_DARK *                resize_with_image_dark;
    SHITOMAS_CORNERS *                      shitomas_corners;
    SHITOMAS_FINISH *                       shitomas_finish;
    SHITOMAS_PREPARE *                      shitomas_prepare;
    WAVELET_FINISH *                        wavelet_finish;
    WAVELET_FUSE *                          wavelet_fuse;
    WAVELET_PREPARE_FIXED *                 wavelet_prepare_fixed;
    WAVELET_PREPARE_RATIO *                 wavelet_prepare_ratio;
}
hetero_world_st;

/*
 * 功能：
 *   用于初始化时获取所有库函数。
 *
 * 返回值：
 *   0 表示成功，-1 表示版本不匹配。
 */
int32_t
path_to_hetero(
    /* 输入，库函数访问指针集 */
    hetero_world_st* phw
);

/* 定义显式加载时的动态库导出函数指针 */
typedef int32_t (PATH_TO_HETERO)(hetero_world_st *);

#ifdef __cplusplus
}
#endif

#endif // HETERO_INTERFACE_202104151110_H
