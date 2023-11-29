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

#include "sys_inc.h"
#include "live_video_rtmp.h"
#include "media_format.h"
#include "lock.h"

#include "log.h"

//#ifdef MEDIA_LIVE
#if 1

/**************************************************************************************/

CLiveVideo_rtmp * CLiveVideo_rtmp::m_pInstance[] = {NULL, NULL, NULL, NULL};
void * CLiveVideo_rtmp::m_pInstMutex = sys_os_create_mutex();

#if __cplusplus
extern "C" {
#endif
#include "log/log.h"
#include "data_engine.h"
#include "sys_config.h"
#include "xutils.h"

#if __cplusplus
}
#endif
/**************************************************************************************/

void * liveVideoThread_rtmp(void * argv)
{
	CLiveVideo_rtmp *capture = (CLiveVideo_rtmp *)argv;

	capture->captureThread();

	return NULL;
}

/**************************************************************************************/

CLiveVideo_rtmp::CLiveVideo_rtmp()
{
	m_nStreamIndex = 0;	
	m_nCodecId = VIDEO_CODEC_NONE;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nFramerate = 25;
	m_nBitrate = 0;

	m_pMutex = sys_os_create_mutex();
	
	m_bInited = FALSE;
	m_bCapture = FALSE;
	m_hCapture = 0;
	
	m_nRefCnt = 0;

	m_pCallbackMutex = sys_os_create_mutex();
    m_pCallbackList = h_list_create(FALSE);
}

CLiveVideo_rtmp::~CLiveVideo_rtmp()
{
	stopCapture();
	
	sys_os_destroy_sig_mutex(m_pMutex);

	h_list_free_container(m_pCallbackList);
	
	sys_os_destroy_sig_mutex(m_pCallbackMutex);
}

CLiveVideo_rtmp * CLiveVideo_rtmp::getInstance(int idx)
{
	if (idx < 0 || idx >= MAX_LIVE_VIDEO_NUMS)
	{
		return NULL;
	}
	
	if (NULL == m_pInstance[idx])
	{
		sys_os_mutex_enter(m_pInstMutex);

		if (NULL == m_pInstance[idx])
		{
			m_pInstance[idx] = (CLiveVideo_rtmp *) new CLiveVideo_rtmp;
			if (m_pInstance[idx])
			{
				m_pInstance[idx]->m_nRefCnt++;
				m_pInstance[idx]->m_nStreamIndex = idx;
			}
		}
		
		sys_os_mutex_leave(m_pInstMutex);
	}
	else
	{
		sys_os_mutex_enter(m_pInstMutex);
		m_pInstance[idx]->m_nRefCnt++;
		sys_os_mutex_leave(m_pInstMutex);
	}

	return m_pInstance[idx];
}

void CLiveVideo_rtmp::freeInstance(int idx)
{
	if (idx < 0 || idx >= MAX_LIVE_VIDEO_NUMS)
	{
		return;
	}

	if (m_pInstance[idx])
	{
    	sys_os_mutex_enter(m_pInstMutex);
    	
    	if (m_pInstance[idx])
    	{
    		m_pInstance[idx]->m_nRefCnt--;

    		if (m_pInstance[idx]->m_nRefCnt <= 0)
    		{
    			delete m_pInstance[idx];
    			m_pInstance[idx] = NULL;
    		}
    	}

    	sys_os_mutex_leave(m_pInstMutex);
	}
}

int CLiveVideo_rtmp::getStreamNums()
{
	// todo : return the max number of streams supported, don't be more than MAX_LIVE_VIDEO_NUMS

	return 1;
}

BOOL CLiveVideo_rtmp::initCapture(int codec, int width, int height, double framerate, int bitrate)
{
	CLock lock(m_pMutex);
	
	if (m_bInited)
	{
		return TRUE;
	}

    m_nCodecId = codec;
	m_nWidth = width;
	m_nHeight= height;
	m_nFramerate = framerate;
	m_nBitrate = bitrate;

	// todo : here add your init code ... 

	m_bInited = TRUE;

	return TRUE;
}

char * CLiveVideo_rtmp::getAuxSDPLine(int rtp_pt)
{
	return NULL;
}

BOOL CLiveVideo_rtmp::startCapture()
{
	CLock lock(m_pMutex);
	
	if (m_hCapture)
	{
		return TRUE;
	}

	m_bCapture = TRUE;
	m_hCapture = sys_os_create_thread((void *)liveVideoThread_rtmp, this);

	return m_hCapture ? TRUE : FALSE;
}

void CLiveVideo_rtmp::stopCapture()
{
	m_bCapture = FALSE;
	
	while (m_hCapture)
	{
		usleep(10*1000);
	}

	// todo : here add your uninit code ...

	m_bInited = FALSE;
}

BOOL CLiveVideo_rtmp::captureThread()
{
	// todo : when get the encoded data, call procData 

	static int number = 0;
	static FILE* fp = 0;
	int write_number = 0;

	unsigned char* data = NULL;
	SLOGI("CLiveVideo_rtmp :%d\n", m_bCapture ? 1:0);
	data_stream_t *data_stream=data_stream_get();
    ring_buffer_t *rb = data_stream->rtmp_data_rb;
	image_frame_info_t frame_info;
	struct timeval time;
	int i=0;
	unsigned int frame_size=1024;
	//int queue_ret=0;
	#ifdef RTMP_THREAD_TIME_LOG
	int rtsp_thread_cnt=0;  
	double t0,t1,t2,t20;
	#endif

	while (m_bCapture)
	{
        // todo : here add the capture handler ... 
		if(ringbuffer_is_empty(rb)) {
			time.tv_sec=0;
			time.tv_usec=(1*1000);
			select(0,NULL,NULL,NULL,&time); 
		    continue;
		}
		 //for(i=0;i<DATA_STREAM_YUV_FRAME_MAX && (!ringbuffer_is_empty(rb)) ; i++){
		   ringbuffer_get(rb, (uint8_t *)&frame_info, sizeof(image_frame_info_t));
		 //}   
		data= data_stream->rtsp_data+frame_info.frame_offset;
		frame_size=frame_info.frame_index;


	// if(number == 0)
    //  {
    //        fp = fopen("/rtmp.h264","w+");              
    //         if(!fp)            
    //         {              
    //             printf("open error mpp\n");        
    //         }
    //  }

          
    // if(number < 60)
    // {              
    //     //printf("data_stream->yuv_data+frame_info.frame_offset: %d \n",*(data_stream->yuv_data+frame_info.frame_offset));


    //     write_number = fwrite(data,1,frame_size,fp);


    //     printf("write_number:%d\n",write_number);

    //     number ++;				      
    // }

    // if(number == 60)
    // { 
    //     printf("i am close ------------------------------\n");
    //     fclose(fp);
    //     number++;
    // }

	#ifdef RTMP_THREAD_TIME_LOG  
		t0=what_time_is_it_now();
	#endif
		CLiveVideo_rtmp::procData(data,frame_size);
	#ifdef RTMP_THREAD_TIME_LOG  
		rtsp_thread_cnt++;
        t1=what_time_is_it_now();
        t20+=(t1-t0);
        if(rtsp_thread_cnt%100 == 88){
            SLOGI("procData %f %fms \n",t20*10,(t1-t2)*1000);
            t20=0;
        }
        t2=t1;
   #endif   
	}

	//QueueFree(&rtmp_srv_ServerQueue[m_nStreamIndex]);

	m_hCapture = 0;
	
	return TRUE;
}

BOOL CLiveVideo_rtmp::isCallbackExist(LiveVideoDataCB_rtmp pCallback, void *pUserdata)
{
	BOOL exist = FALSE;
	LiveVideoCB_rtmp * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB_rtmp *) p_node->p_data;
		if (p_cb->pCallback == pCallback && p_cb->pUserdata == pUserdata)
		{
			exist = TRUE;
			break;
		}
		
		p_node = h_list_lookup_next(m_pCallbackList, p_node);
	}
	h_list_lookup_end(m_pCallbackList);
	
	sys_os_mutex_leave(m_pCallbackMutex);

	return exist;
}

void CLiveVideo_rtmp::addCallback(LiveVideoDataCB_rtmp pCallback, void * pUserdata)
{
	if (isCallbackExist(pCallback, pUserdata))
	{
		return;
	}
	
	LiveVideoCB_rtmp * p_cb = (LiveVideoCB_rtmp *) malloc(sizeof(LiveVideoCB_rtmp));

	p_cb->pCallback = pCallback;
	p_cb->pUserdata = pUserdata;
	p_cb->bFirst = TRUE;

	sys_os_mutex_enter(m_pCallbackMutex);
	h_list_add_at_back(m_pCallbackList, p_cb);	
	sys_os_mutex_leave(m_pCallbackMutex);
}

void CLiveVideo_rtmp::delCallback(LiveVideoDataCB_rtmp pCallback, void * pUserdata)
{
	LiveVideoCB_rtmp * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB_rtmp *) p_node->p_data;
		if (p_cb->pCallback == pCallback && p_cb->pUserdata == pUserdata)
		{		
			free(p_cb);
			
			h_list_remove(m_pCallbackList, p_node);
			break;
		}
		
		p_node = h_list_lookup_next(m_pCallbackList, p_node);
	}
	h_list_lookup_end(m_pCallbackList);

	sys_os_mutex_leave(m_pCallbackMutex);
}

void CLiveVideo_rtmp::procData(uint8 * data, int size)
{
	LiveVideoCB_rtmp * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB_rtmp *) p_node->p_data;
		if (p_cb->pCallback != NULL)
		{		    
			p_cb->pCallback(data, size, p_cb->pUserdata);
		}
		
		p_node = h_list_lookup_next(m_pCallbackList, p_node);
	}
	h_list_lookup_end(m_pCallbackList);

	sys_os_mutex_leave(m_pCallbackMutex);
}

BOOL media_live_put_video(int idx, uint8 * data, int size)
{
    CLiveVideo_rtmp * live = CLiveVideo_rtmp::getInstance(idx);
    if (NULL == live)
    {
        return FALSE;
    }

    live->procData(data, size);
    
    live->freeInstance(idx);
    
    return TRUE;
}

#endif // MEDIA_LIVE


