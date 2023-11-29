#ifndef _XF_ROCKCHIP_RGA_H__
#define _XF_ROCKCHIP_RGA_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <linux/videodev2.h>

#include "rga.h"
#include "drmrga.h"

#define XF_RGA_ALIGN(x, a)         (((x) + (a) - 1) / (a) * (a))
#define xf_rga_debug(_RGA_, fmt, args...) //LOGD(fmt, ##args)
#define xf_rga_err(_RGA_, fmt, args...) printf(fmt, ##args)
#define xf_rga_warn(_RGA_, fmt, args...) printf(fmt, ##args)

static inline int
RockchipRgaCheckFormat(int foramt)
{
    switch(foramt) {
    case RK_FORMAT_RGBA_8888:
    case RK_FORMAT_RGBX_8888:
    case RK_FORMAT_RGB_888:
    case RK_FORMAT_BGRA_8888:
    case RK_FORMAT_RGB_565:
    case RK_FORMAT_RGBA_5551:
    case RK_FORMAT_RGBA_4444:
    case RK_FORMAT_BGR_888:
    case RK_FORMAT_YCbCr_422_SP:
    case RK_FORMAT_YCbCr_422_P:
    case RK_FORMAT_YCbCr_420_SP:
    case RK_FORMAT_YCbCr_420_P:
    case RK_FORMAT_YCrCb_422_SP:
    case RK_FORMAT_YCrCb_422_P:
    case RK_FORMAT_YCrCb_420_SP:
    case RK_FORMAT_YCrCb_420_P:
    case RK_FORMAT_BGRX_8888:
    case RK_FORMAT_YCbCr_420_SP_10B:
    case RK_FORMAT_YCrCb_420_SP_10B:
        return 1;
    default:
        return 0;
    }
}

struct _XfRockchipRga;
typedef struct _RgaOps {
    /* Init rga contex */
    void (*initCtx)(struct _XfRockchipRga *rga);

    void (*setRotate)(struct _XfRockchipRga *rga, int rotate);
    /* Set solid fill color, color:
     * Blue:  0xffff0000
     * Green: 0xff00ff00
     * Red:   0xff0000ff
     */
    void (*setFillColor)(struct _XfRockchipRga *rga, int color);

    /* Set source rect info */
    void (*setSrcRectInfo)(struct _XfRockchipRga *rga, int cropX, int cropY,
                   int cropW, int cropH,int width, int height,int format);

    /* Set destination rect info */
    void (*setDstRectInfo)(struct _XfRockchipRga *rga, int cropX, int cropY,
                   int cropW, int cropH,int width, int height,int format);

    /* Set source dma buffer file description */
    void (*setSrcBufferFd)(struct _XfRockchipRga *rga, int dmaFd);

    /* Set destination dma buffer file description */
    void (*setDstBufferFd)(struct _XfRockchipRga *rga, int dmaFd);

    /* Set source userspace data pointer */
    void (*setSrcBufferPtr)(struct _XfRockchipRga *rga, unsigned char *ptr);

    /* Set destination userspace data pointer */
    void (*setDstBufferPtr)(struct _XfRockchipRga *rga, unsigned char *ptr);

    int (*go)(struct _XfRockchipRga *rga);
} RgaOps;

typedef struct _XfRockchipRga {      
    rga_info_t src_info;
    rga_info_t dst_info;
    RgaOps *ops;    
    void *priv;
} XfRockchipRga;

#ifdef __cplusplus
extern "C" {
#endif

int rk_rga_init(void);
void rk_rga_deinit(void);

XfRockchipRga *RgaCreate(void);
void RgaDestroy(XfRockchipRga *rga);

#ifdef __cplusplus
}
#endif

#endif


