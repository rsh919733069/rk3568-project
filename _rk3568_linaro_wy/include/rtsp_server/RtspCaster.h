/*
 * RtspCaster.h
 *
 *  Created on: 2016年4月6日
 *      Author: terry
 */

#ifndef RTSPCASTER_H_
#define RTSPCASTER_H_


////////////////////////////////////////////////////////////////////////////

#ifdef WIN32

    #ifndef NOMINMAX
    #define NOMINMAX
    #endif //NOMINMAX

	#include <Windows.h>
#else

#endif //WIN32


////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
    typedef signed char     int8_t;
    typedef unsigned char   uint8_t;
    typedef short           int16_t;
    typedef unsigned short  uint16_t;
    typedef int             int32_t;
    typedef unsigned        uint32_t;
    typedef long long       int64_t;
    typedef unsigned long long   uint64_t;
#else
    #include <stdint.h>
    typedef void*   HANDLE;
#endif //_MSC_VER


///////////////////////////////////////////////////////////////////
#ifdef WIN32
    #ifndef DLLEXPORT
    #define DLLEXPORT __declspec(dllexport)
    #endif //DLLEXPORT
#else
    #define DLLEXPORT __attribute__ ((visibility ("default")))
#endif //WIN32

///////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

/////////////////////////////////////////////////////////////////////////////
#ifndef MKBETAG
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
#endif //MKBETAG


#ifndef CASTER_TYPE
#define	CASTER_TYPE


/// 编码 
enum MCodec
{
	MCODEC_NONE = 0,

	MCODEC_H264 = 28,
	MCODEC_HEVC = 174, /// H.265
	MCODEC_H265 = MCODEC_HEVC,

	MCODEC_G711U = 65542,
	MCODEC_G711A,

	MCODEC_MP3 = 0x15001,
	MCODEC_AAC = 0x15002,
	MCODEC_AC3 = 0x15003,
	MCODEC_VORBIS = 0x15005,
    MCODEC_OPUS = 0x1503d,
	MCODEC_RAW = 0x10101010

};

enum MType
{
	MTYPE_NONE = -1,
	MTYPE_VIDEO = 0,
	MTYPE_AUDIO,
	MTYPE_DATA,
};


/// 媒体格式 
typedef struct MFormat
{
	int codec;		/// 视频编码  @see MCodec
	int width;		/// 视频高 
	int height;		/// 视频宽 
	int framerate;		/// 帧率 
	int profile;
	int clockRate;  /// 时钟频率 

	int audioCodec;	/// 音频编码  @see MCodec
	int channels;	/// 通道数 
	int sampleRate;	/// 采样率 
	int audioProfile;	/// 档次 
	int audioRate;      /// 音频时钟频率 

	int vPropSize;		/// 视频解码参数, 对于H.264是sps+pps, 对于H.265是vps+sps+pps
	unsigned char* vProp;

	int configSize;		/// 音频解码参数, 如果是AAC编码, 则必须设置为AAC的config参数 
	unsigned char* config;

    int bitrate;    ///码率. 
    int audioBitrate;
}MFormat;



/// 媒体包 
typedef struct MPacket
{
	int type;       ///
	uint8_t* data;	/// 数据指针 
	int size;		/// 数据长度 
	int64_t pts;	/// 时间戳 
	int duration;	/// 时长 
	int flags;		/// 标识 
}MPacket;


enum CasterConst
{
	MAX_CASTER = 64,
	INVALID_CASTER = -1
};


typedef int		caster_t;


enum CasterEventType
{
	CASTER_SESSION_REQUEST = 1,
	CASTER_SESSION_CREATE,
	CASTER_SESSION_DESTROY,

    CASTER_CLIENT_SESSION_SETUP = 0x10
    
};

typedef struct CasterEvent
{
	int  type;
	caster_t  handle;	/// 通道句柄 
	char name[256];		/// 通道名称 
}CasterEvent;


/**
* 事件回调函数 
* @param event		事件 
* @param context	回调环境 
*/
typedef void(*CasterEventCallback)(const CasterEvent* event, void* context);


#endif //CASTER_TYPE


typedef CasterEvent	RtspCasterEvent;


/**
 * 初始化 
 * @param port	RTSP端口 
 * @param ip    在哪个地址监听, 可以取值 null, 0.0.0.0 表示监听所有地址 
 * @return 0 表示成功 
 */
DLLEXPORT int caster_init(int port, const char* ip);

/**
 * 反初始化 
 * @return
 */
DLLEXPORT int caster_quit();

/**
 * 设置端口范围 
 * @param minPort
 * @param maxPort
 * @return
 */
DLLEXPORT int caster_set_port_range(int minPort, int maxPort);

/**
 * 打开通道 
 * @param handle 返回的通道句柄 
 * @param name	通道名称 
 * @param fmt   媒体格式 
 * @return 0 表示成功 
 */
DLLEXPORT int caster_chl_open(HANDLE* handle, const char* name, const MFormat* fmt);

/**
 * 关闭通道 
 * @param handle 通道句柄 
 */
DLLEXPORT void caster_chl_close(HANDLE handle);

/**
 * 通道是否打开, 即判断有效 
 * @param handle 通道句柄 
 * @return > 0 表示有效 
 */
DLLEXPORT int caster_chl_is_open(HANDLE handle);

/**
 *
 * @param handle
 * @param prop
 * @param size
 * @return
 */
DLLEXPORT int caster_chl_set_video_prop(HANDLE handle, const uint8_t* prop, int size);

/**
 * 获取通道媒体格式 
 * @param handle   通道句柄 
 * @param fmt 媒体格式, fmt 中的指针不能保证永久有效. 
 * @return 0 表示成功 
 */
DLLEXPORT int caster_chl_get_format(HANDLE handle, MFormat* fmt);

/**
 * 写视频包 
 * @param handle  通道句柄 
 * @param pkt  包
 * @return 0 表示成功 
 */
DLLEXPORT int caster_chl_write_video(HANDLE handle, const MPacket* pkt);

/**
 * 写音频包 
 * @param handle
 * @param pkt
 * @return
 */
DLLEXPORT int caster_chl_write_audio(HANDLE handle, const MPacket* pkt);


/**
 * 更新通道媒体格式 
 * @param handle   通道句柄 
 * @param fmt 媒体格式 
 * @return 0 表示成功 
 */
DLLEXPORT int caster_chl_set_format(HANDLE handle, MFormat* fmt);



/**
 * 事件回调函数 
 * @param event		事件 
 * @param context	回调环境 
 */
typedef void (*RtspCasterEventCallback)(const RtspCasterEvent* event, void* context);

/**
 * 设置事件回调句柄 
 * @param cb		回调函数指针 
 * @param context	回调环境 
 */
DLLEXPORT void caster_set_event_callback(RtspCasterEventCallback cb, void* context);


/**
 * 启用日志 
 * @param filename	日志文件 
 */
DLLEXPORT void caster_enable_log(const char* filename);

/**
* 设置会话检测时间, 单位为秒, 默认为60
* 在初始化 caster_init 之前调用
* @param seconds > 0	
*/
DLLEXPORT void caster_setReclamation(int seconds);


/**
* 设置 socket 发送缓冲区大小, 默认为 1M
* @param seconds > 0
*/
DLLEXPORT void caster_setSendBufferSize(int bufferSize);

/**
 * 设置包缓冲区大小, 默认为1 M
 * @param size
 */
DLLEXPORT void caster_setMaxPacketBufferSize(int size);

/**
* 关闭对应通道的媒体会话
* @param handle
* @return 0 表示成功
*/
DLLEXPORT int caster_chl_close_session(HANDLE handle);

/**
* 设置用户认证文件.
* 用户认证文件为类似apache htpasswd 文件, 可以用 htpasswd这个工具生成.
* @param filepath	用户认证文件
* @return 0 表示成功
*/
DLLEXPORT int caster_set_auth_file(const char* filepath);

/**
 * TCP传输模式下, 强制发送完整的帧. 默认为false, 在带宽不足时会造成花屏
 * @param forced
 * @return 0 表示成功
 */
DLLEXPORT int caster_tcp_force_complete_frame(int forced);

/**
* 关闭对应通道的客户端会话
* @param handle
* @return 0 表示成功
*/
DLLEXPORT int caster_chl_close_client_session(HANDLE handle);
/////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif


#endif /* RTSPCASTER_H_ */
