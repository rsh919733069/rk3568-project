#ifndef LIBGUIDEUSB2LIVESTREAM_H
#define LIBGUIDEUSB2LIVESTREAM_H

#ifdef __cplusplus
extern "C" 
{
#endif

typedef enum
{
    CLOSE            = 0,   //close log
    LOG_FATALEER     = 1,
    LOG_ERROR        = 3,
    LOG_WARN         = 7,
    LOG_INFO         = 15,
    LOG_TEST         = 31
}guide_usb_log_level_e;

typedef enum
{
    X16 = 0,                             //X16
    X16_PARAM = 1,                       //X16+参数行
    Y16 = 2,                             //Y16
    Y16_PARAM = 3,                       //Y16+参数行
    YUV = 4,                             //YUV
    YUV_PARAM = 5,                       //YUV+参数行
    Y16_YUV = 6,                         //Y16+YUV
    Y16_PARAM_YUV = 7                    //Y16+参数行+YUV
}guide_usb_video_mode_e;

typedef enum
{
    DEVICE_CONNECT_OK = 1,                //连接正常
    DEVICE_DISCONNECT_OK = -1             //断开连接
}guide_usb_device_status_e;

typedef enum
{
    ISOTHERM_MODE_RANGE_NONE = 0,
    ISOTHERM_MODE_RANGE_MIDDLE,
    ISOTHERM_MODE_RANGE_UP_DOWN
}guide_usb_isothermmode_e;

typedef enum
{
    GSDK_SUCCESS                             =  0,
    GSDK_ERROR_USB_INIT                      = -1,
    GSDK_ERROR_USB_DESCRIPTOR                = -2,
    GSDK_ERROR_USB_OPEN                      = -3,
    GSDK_ERROR_USB_DETACH_KERNEL             = -4,
    GSDK_ERROR_USB_CLAIM_INTERFACE           = -5,
    GSDK_ERROR_INIT_QUEUE                    = -6,
    GSDK_ERROR_SERIAL_THREAD_CREATE          = -7,
    GSDK_ERROR_RECEIVE_THREAD_CREATE         = -8,
    GSDK_ERROR_DEAL_THREAD_CREATE            = -9,
    GSDK_ERROR_SEND_COMMAND                  = -10,
    GSDK_ERROR_SEND_COMMAND_TIMEOUT          = -11,
    GSDK_ERROR_UPGRADE_VIDEO_ON              = -12,
    GSDK_ERROR_SERIAL_THREAD_EXIT            = -13,
    GSDK_ERROR_RELEASE_INTERFACE             = -14,
    GSDK_ERROR_PARAMLINE                     = -15,
    GSDK_ERROR_EMISS                         = -16,
    GSDK_ERROR_RELHUM                        = -17,
    GSDK_ERROR_DISTANCE                      = -18,
    GSDK_ERROR_REFLECTED_TEMPER              = -19,
    GSDK_ERROR_ATMOSPHERIC_TEMPER            = -20,
    GSDK_ERROR_MODIFY_K                      = -21,
    GSDK_ERROR_MODIFY_B                      = -22,
    GSDK_ERROR_SHUTTER                       = -23,
    GSDK_ERROR_NO_DEVICES                    = -24,
    GSDK_ERROR_FILE_NO_EXIST                 = -25,
    GSDK_ERROR_FILE_FORMAT                   = -26,
    GSDK_ERROR_FILE_OVER_SIZE                = -27
}gsdk_usb_ret_code_e;

typedef struct
{
    int width;                              //图像宽度
    int height;                             //图像高度
    guide_usb_video_mode_e video_mode;      //视频模式
}guide_usb_device_info_t;

typedef struct
{
    int frame_width;                        //图像宽度
    int frame_height;                       //图像高度
    unsigned char* frame_rgb_data;          //rgb数据
    int frame_rgb_data_length;              //rgb数据长度
    short* frame_src_data;                  //原始数据，x16/y16
    int frame_src_data_length;              //原始数据长度
    short* frame_yuv_data;                  //yuv数据
    int frame_yuv_data_length;              //yuv数据长度
    short* paramLine;                       //参数行
    int paramLine_length;                   //参数行长度
}guide_usb_frame_data_t;

typedef struct
{
    unsigned char* serial_recv_data;
    int serial_recv_data_length;
}guide_usb_serial_data_t;

typedef struct
{
    unsigned short emiss;
    unsigned short relHum;
    unsigned short distance;
    short reflectedTemper;
    short atmosphericTemper;
    unsigned short modifyK;
    short modifyB;
}guide_usb_measure_external_param_t;

typedef int (*OnDeviceConnectStatusCB)(guide_usb_device_status_e deviceStatus);
typedef int (*OnFrameDataReceivedCB)(guide_usb_frame_data_t *pVideoData);
typedef int (*OnSerialDataReceivedCB)(guide_usb_serial_data_t *pSerialData);

int guide_usb_initial();
int guide_usb_exit();
int guide_usb_openstream(guide_usb_device_info_t* deviceInfo,OnFrameDataReceivedCB frameRecvCB,OnDeviceConnectStatusCB connectStatusCB);//连接设备
int guide_usb_closestream();
int guide_usb_sendcommand(unsigned char* cmd, int length);
int guide_usb_upgrade(const char* file);
int guide_usb_opencommandcontrol(OnSerialDataReceivedCB serialRecvCB);
int guide_usb_closecommandcontrol();
int guide_usb_setloglevel(int level);
int guide_usb_upgradecolor(const char* file);//升级color
int guide_usb_upgradecurve(const char* file);//升级curve
int guide_usb_interpolate_zoom(int multiple,short* yuvSrc, short*yuvDst,int width, int height, int x, int y,int paletteIndex);
int guide_usb_isotherm(float temperal,float temperah,int* y16Data,short* yuvData,unsigned char* paramline,int width,int height,guide_usb_measure_external_param_t* pParamExt, guide_usb_isothermmode_e isothermmode,int paletteIndex,int devtype, int lenstype);
int guide_usb_measure_convertgray2temper(int devtype, int lenstype, short* pGray, unsigned char* pParamLine, int len, guide_usb_measure_external_param_t* pParamExt, float* pTemper);

#ifdef __cplusplus
}
#endif

#endif // LIBGUIDEUSB2LIVESTREAM_H

