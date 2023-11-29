
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "RockchipRga.h"
#include "xf_rockchip_rga.h"

static RockchipRga s_rkRga;
static int s_rga_init_flag = 1;

int rk_rga_init()
{
    int ret = 0;
    ret = s_rkRga.RkRgaInit();
    s_rga_init_flag = 0;    
    return ret;
}

void rk_rga_deinit()
{
    s_rkRga.RkRgaDeInit();
    s_rga_init_flag = 1;
}

static size_t
RockchipRgaGetSize(int format, __u32 width, __u32 height)
{
    switch(format) {
    case RK_FORMAT_YCbCr_420_SP:    // YUV420SP
    case RK_FORMAT_YCbCr_420_P:
    case RK_FORMAT_YCrCb_420_SP:
    case RK_FORMAT_YCrCb_420_P:
        return XF_RGA_ALIGN(width, 16) * XF_RGA_ALIGN(height, 16) * 3 / 2;
    case RK_FORMAT_RGB_565:
    case RK_FORMAT_RGBA_5551:
    case RK_FORMAT_RGBA_4444:
    case RK_FORMAT_YCbCr_422_SP:
    case RK_FORMAT_YCbCr_422_P:
    case RK_FORMAT_YCrCb_422_SP:
    case RK_FORMAT_YCrCb_422_P:   
        return XF_RGA_ALIGN(width, 16) * XF_RGA_ALIGN(height, 16) * 2;
    case RK_FORMAT_RGB_888:
    case RK_FORMAT_BGR_888:
        return XF_RGA_ALIGN(width, 16) * XF_RGA_ALIGN(height, 16) * 3;
    case RK_FORMAT_RGBA_8888: 
    case RK_FORMAT_RGBX_8888: 
    case RK_FORMAT_BGRA_8888: 
    case RK_FORMAT_BGRX_8888: 
        return XF_RGA_ALIGN(width, 16) * XF_RGA_ALIGN(height, 16) * 4;
    default:
	assert(0);
	break;
    }

    return 0;
}

static void
RockchipRgaInitCtx(XfRockchipRga *rga)
{
    memset(&rga->src_info, 0, sizeof(rga->src_info));
    memset(&rga->dst_info, 0, sizeof(rga->dst_info));
}

static void
RockchipRgaSetRotate(XfRockchipRga *rga, int rotate)
{
    xf_rga_debug(rga, "SetRotate: rotate %d\n", rotate);
}

static void
RockchipRgaSetFillColor(XfRockchipRga *rga, int color)
{
    
}

static void
RockchipRgaSetSrcRectInfo(XfRockchipRga *rga, int cropX, int cropY,
                   int cropW, int cropH,int width, int height,int format)
{
    assert(RockchipRgaCheckFormat(format));
    assert((cropW > 0) && (cropH > 0) && (width > 0) && (height > 0));

    rga_set_rect(&rga->src_info.rect,cropX,cropY,cropW,cropH,width/*stride*/,height,format);

    xf_rga_debug(rga, "SetSrcRectInfo: cropX %d, cropY %d, cropW %d, cropH %d width %d,height %d\n",
		    cropX, cropY, cropW, cropH,width,height);
}

static void
RockchipRgaSetDstRectInfo(XfRockchipRga *rga, int cropX, int cropY,
                   int cropW, int cropH,int width, int height,int format)
{
    assert(RockchipRgaCheckFormat(format));
    assert((cropW > 0) && (cropH > 0) && (width > 0) && (height > 0));

    rga_set_rect(&rga->dst_info.rect,cropX,cropY,cropW,cropH,width/*stride*/,height,format);

    xf_rga_debug(rga, "SetDstRectInfo: cropX %d, cropY %d, cropW %d, cropH %d width %d,height %d\n",
		    cropX, cropY, cropW, cropH,width,height);
}

static void
RockchipRgaSetSrcBufferFd(XfRockchipRga *rga, int dmaFd)
{
    rga->src_info.fd = dmaFd;
}

static void
RockchipRgaSetDstBufferFd(XfRockchipRga *rga, int dmaFd)
{
    rga->dst_info.fd = dmaFd;
}

static void
RockchipRgaSetSrcBufferPtr(XfRockchipRga *rga, unsigned char *ptr)
{
    rga->src_info.fd = -1;
    rga->src_info.mmuFlag = 1;
    rga->src_info.virAddr = (void *)ptr;
}

static void
RockchipRgaSetDstBufferPtr(XfRockchipRga *rga, unsigned char *ptr)
{
    rga->dst_info.fd = -1;
    rga->dst_info.mmuFlag = 1;
    rga->dst_info.virAddr = (void *)ptr;
}

static int
RockchipRgaGo(XfRockchipRga *rga)
{
    int ret = -EINVAL;
    
    if (0 == s_rga_init_flag) {        
        ret = s_rkRga.RkRgaBlit(&rga->src_info, &rga->dst_info, NULL);
    }

    return ret;
}

static RgaOps rgaOps = {
    .initCtx         =  RockchipRgaInitCtx,
    .setRotate       =  RockchipRgaSetRotate,
    .setFillColor    =  RockchipRgaSetFillColor,
    
    .setSrcRectInfo  =  RockchipRgaSetSrcRectInfo,
    .setDstRectInfo  =  RockchipRgaSetDstRectInfo,
    .setSrcBufferFd  =  RockchipRgaSetSrcBufferFd,
    .setDstBufferFd  =  RockchipRgaSetDstBufferFd,
    .setSrcBufferPtr =  RockchipRgaSetSrcBufferPtr,
    .setDstBufferPtr =  RockchipRgaSetDstBufferPtr,
    
    .go = RockchipRgaGo,
};

XfRockchipRga *RgaCreate(void)
{
    XfRockchipRga *rga;
 
    rga = (XfRockchipRga *)calloc(1, sizeof(*rga));
    if(!rga) {
        xf_rga_err(rga, "No memory for xfrockchip rga\n");
        return NULL;
    }

    rga->ops = &rgaOps;

    return rga;
}

void RgaDestroy(XfRockchipRga *rga)
{
    free(rga);
}