/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2014-2021, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifndef RTMP_RMUA_H
#define RTMP_RMUA_H

#include "sys_inc.h"
#include "rtmp_cfg.h"
#include "hqueue.h"
#include "rtmp.h"

#ifdef MEDIA_FILE
#include "file_demux.h"
#endif

#ifdef MEDIA_DEVICE
#if __WINDOWS_OS__
#include "audio_capture_dsound.h"
#include "video_capture_win.h"
#include "screen_capture_win.h"
#elif defined(ANDROID)
#include "audio_capture_android.h"
#include "video_capture_qt.h"
#include "screen_capture.h"
#elif defined(IOS)
#include "audio_capture_mac.h"
#include "video_capture_mac.h"
#include "screen_capture_mac.h"
#elif __LINUX_OS__
#include "audio_capture_linux.h"
#include "video_capture_linux.h"
#include "screen_capture_linux.h"
#endif
#endif

#ifdef MEDIA_PROXY
#include "media_proxy.h"
#endif

#ifdef MEDIA_LIVE
#include "live_video_rtmp.h"
#include "live_audio_rtmp.h"

#endif

#ifdef DEMO
#define MAX_NUM_RUA			2
#else
#   ifdef MEDIA_LIVE
#define MAX_NUM_RUA         10
#   else
#define MAX_NUM_RUA			100
#   endif
#endif

#include "linked_list.h"

/***************************************************************************************/

#define MAX_LIVE_VIDEO_NUMS  4

typedef void (*LiveVideoDataCB_rtmp)(uint8 *data, int size, void *pUserdata);

typedef struct
{
	LiveVideoDataCB_rtmp pCallback;      // callback function pointer
	void *          pUserdata;      // user data
	BOOL            bFirst;         // first flag
} LiveVideoCB_rtmp;

/***************************************************************************************/


class CLiveVideo_rtmp
{
public:
    virtual ~CLiveVideo_rtmp();
    
    // get the support stream numbers
    static int      getStreamNums();
    
    static CLiveVideo_rtmp * getInstance(int idx);
    
    virtual void    freeInstance(int idx);

    virtual BOOL    initCapture(int codec, int width, int height, double framerate, int bitrate);
	virtual BOOL    startCapture();

    virtual void    addCallback(LiveVideoDataCB_rtmp pCallback, void * pUserdata);
    virtual void    delCallback(LiveVideoDataCB_rtmp pCallback, void * pUserdata);
    
    virtual char *  getAuxSDPLine(int rtp_pt);
    virtual BOOL    captureThread();

    void            procData(uint8 * data, int size);
    
protected:
    CLiveVideo_rtmp();
    CLiveVideo_rtmp(CLiveVideo_rtmp &obj);

    BOOL            isCallbackExist(LiveVideoDataCB_rtmp pCallback, void *pUserdata);
    
	virtual void    stopCapture();

public:
    int                     m_nStreamIndex;
    int                     m_nRefCnt;

    static void *           m_pInstMutex;
	static CLiveVideo_rtmp *     m_pInstance[MAX_LIVE_VIDEO_NUMS];
	
protected:
    int                     m_nCodecId;
	int                     m_nWidth;
	int	                    m_nHeight;
	double                  m_nFramerate;
    int                     m_nBitrate;
    
    void *                  m_pMutex;
    BOOL                    m_bInited;
    BOOL                    m_bCapture;
    pthread_t               m_hCapture;

    void *                  m_pCallbackMutex;
	LINKED_LIST *           m_pCallbackList;
};

#define MAX_LIVE_AUDIO_NUMS  4

typedef void (*LiveAudioDataCB_rtmp)(uint8 *data, int size, int nbsamples, void *pUserdata);

typedef struct
{
	LiveAudioDataCB_rtmp pCallback;    // callback function pointer
	void *          pUserdata;    // user data
	BOOL            bFirst;       // first flag
} LiveAudioCB_rtmp;

/***************************************************************************************/

class CLiveAudio_rtmp
{
public:
    virtual ~CLiveAudio_rtmp();
    
    // get the support stream numbers     
    static int      getStreamNums();

    // get single instance
	static CLiveAudio_rtmp * getInstance(int idx);
	
	// free instance
	virtual void    freeInstance(int idx);

    virtual BOOL    initCapture(int codec, int sampleRate, int channels, int bitrate);
	virtual BOOL    startCapture();
    
    virtual char *  getAuxSDPLine(int rtp_pt);
    
    virtual void    addCallback(LiveAudioDataCB_rtmp pCallback, void * pUserdata);
    virtual void    delCallback(LiveAudioDataCB_rtmp pCallback, void * pUserdata);

    virtual BOOL    captureThread();    

    void            procData(uint8 * data, int size, int nbsamples);
    
protected:
    CLiveAudio_rtmp();
    CLiveAudio_rtmp(CLiveAudio_rtmp &obj);	
	
    virtual void    stopCapture();
    
    BOOL            isCallbackExist(LiveAudioDataCB_rtmp pCallback, void *pUserdata);
    
public:
    int                     m_nStreamIndex;
    int                     m_nRefCnt;      // single instance ref count

    static void *           m_pInstMutex;   // instance mutex
    static CLiveAudio_rtmp *     m_pInstance[MAX_LIVE_AUDIO_NUMS];
    
protected:    
    BOOL			        m_bCapture;     // capturing flag
    pthread_t               m_hCapture;

    int                     m_nCodecId;     // audio codec
    int                     m_nSampleRate;  // sample rate
    int                     m_nChannel;     // channel
    int                     m_nBitrate;     // bitrate
    
    void *                  m_pMutex;       // mutex
    BOOL                    m_bInited;      // inited flag

    void *                  m_pCallbackMutex;
	LINKED_LIST *           m_pCallbackList;
};


/***************************************************************************************/
typedef struct
{
	uint32		    has_video : 1;      // has video flag
	uint32		    has_audio : 1;      // has audio flag
	uint32		    is_file   : 1;      // is file
	uint32		    is_device : 1;      // is device
	uint32		    is_screen : 1;      // is screen live
	uint32          is_proxy  : 1;      // is proxy
	uint32          is_live   : 1;      // is live
	uint32		    reserved  : 25;
	
    uint8           v_index;            // video device index
    uint8           a_index;            // audio device index
    
	VIDEO_INFO      v_info;			    // video information
	AUDIO_INFO      a_info;			    // audio information

	HQUEUE *	    v_queue;            // video queue
	HQUEUE *	    a_queue;            // audio queue

	pthread_t	    v_thread;           // video thread
	pthread_t	    a_thread;           // audio thread

#ifdef MEDIA_PROXY
    CMediaProxy *   proxy;              // proxy object
#endif

#ifdef MEDIA_FILE
	CFileDemux *    file_demuxer;       // file demuxer object
#endif

#ifdef MEDIA_LIVE
    CLiveVideo_rtmp *    live_video;         // live video object
    CLiveAudio_rtmp *    live_audio;         // live audio object
#endif

#ifdef MEDIA_DEVICE
    CVideoCapture * video_capture;      // video capture
    CAudioCapture * audio_capture;      // audio capture
	CScreenCapture* screen_capture;     // screen capture
#endif
} RMUA_MEDIA_INFO;

typedef struct
{
    uint32          used_flag : 1;      // used flag
	uint32          rtp_tx    : 1;      // tx flag
	uint32          avcc_flag : 1;      // have SPS, PPS been sent? 
	uint32          aac_flag  : 1;      // have AAC spec been sent?
	uint32          iframe_tx : 1;      // already send i-frame
	uint32		    reserved  : 27;

	char		    src_url[256];	    // source url 
	char		    dst_url[256];	    // dest url

	char            user[100];          // the username for rtmp connection
	char            pass[100];          // the password for rtmp connection

	void *          pusher;             // pointer the pusher object

	pthread_t	    tid_pusher;		    // pusher thread id
	pthread_t       tid_audio;          // audio send thread id

	void *		    mutex;		        // rtmp mutex
	RTMP *		    rtmp;			    // rtmp object
    uint32          start_ts;           // start timestamp
    double          audio_ts;           // audio timestamp
    
    uint8           vps_buf[256];       // vps buf
    int             vps_len;            // vps buf length
	uint8		    sps_buf[256];       // sps buf
	int			    sps_len;            // sps buf length
	uint8		    pps_buf[256];       // pps buf
	int			    pps_len;            // pps buf length

    RMUA_MEDIA_INFO media_info;         // media information
} RMPUA;

#ifdef __cplusplus
extern "C" {
#endif

void    rmpua_proxy_init();
void    rmpua_proxy_deinit();
RMPUA * rmpua_get_idle_rua();
void    rmpua_set_online_rua(RMPUA * p_rua);
void    rmpua_set_idle_rua(RMPUA * p_rua);
RMPUA * rmpua_lookup_start();
RMPUA * rmpua_lookup_next(RMPUA * p_rua);
void    rmpua_lookup_stop();
uint32  rmpua_get_index(RMPUA * p_rua);
RMPUA * rmpua_get_by_index(uint32 index);

#ifdef __cplusplus
}
#endif


#endif



