/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2014-2020, Happytimesoft Corporation, all rights reserved.
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
#include "live_video.h"
#include "media_format.h"
#include "lock.h"

#ifndef MEDIA_LIVE
#define MEDIA_LIVE
#endif

#ifdef MEDIA_LIVE

#if __cplusplus
extern "C" {
#endif
#include "log/log.h"
#include "queue.h"

// int rtsp_srv_length_count[4] = {0};

extern QueueStruct rtsp_srv_ServerQueue[4];
// extern int rtsp_srv_length[4][1000];
#include "data_engine.h"
#include "sys_config.h"
#include "xutils.h"
#if __cplusplus
}
#endif

/**************************************************************************************/

CLiveVideo * CLiveVideo::m_pInstance[] = {NULL, NULL, NULL, NULL};
void * CLiveVideo::m_pInstMutex = sys_os_create_mutex();


/**************************************************************************************/

void * liveVideoThread(void * argv)
{
	CLiveVideo *capture = (CLiveVideo *)argv;

	capture->captureThread();

	return NULL;
}

/**************************************************************************************/

CLiveVideo::CLiveVideo()
{
	m_nDevIndex = 0;	
	m_nCodecId = VIDEO_CODEC_NONE;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nFramerate = 15;
	m_nBitrate = 0;

	m_pMutex = sys_os_create_mutex();
	
	m_bInited = FALSE;
	m_bCapture = FALSE;
	m_hCapture = 0;
	
	m_nRefCnt = 0;

	m_pCallbackMutex = sys_os_create_mutex();
    m_pCallbackList = h_list_create(FALSE);
}

CLiveVideo::~CLiveVideo()
{
	stopCapture();
	
	sys_os_destroy_sig_mutex(m_pMutex);

	h_list_free_container(m_pCallbackList);
	
	sys_os_destroy_sig_mutex(m_pCallbackMutex);
}

CLiveVideo * CLiveVideo::getInstance(int idx)
{
	if (idx < 0 || idx >= MAX_STREAM_NUMS)
	{
		return NULL;
	}
	
	if (NULL == m_pInstance[idx])
	{
		sys_os_mutex_enter(m_pInstMutex);

		if (NULL == m_pInstance[idx])
		{
			m_pInstance[idx] = (CLiveVideo *) new CLiveVideo;
			if (m_pInstance[idx])
			{
				m_pInstance[idx]->m_nRefCnt++;
				m_pInstance[idx]->m_nDevIndex = idx;
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

void CLiveVideo::freeInstance(int idx)
{
	if (idx < 0 || idx >= MAX_STREAM_NUMS)
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

int CLiveVideo::getStreamNums()
{
	// todo : return the max number of streams supported, don't be more than MAX_STREAM_NUMS

	
	return 1;
}

BOOL CLiveVideo::initCapture(int codec, int width, int height, int framerate, int bitrate)
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

char * CLiveVideo::getAuxSDPLine(int rtp_pt)
{
    // For H264, Get SPS, PPS parameters from the hardware encoder
    // For H265, Get VPS, SPS, PPS parameters from the hardware encoder
    //  Construct "a = fmt:96 xxx" SDP line
    //  Please reference the CVideoEncoder::getAuxSDPLine function in video_encoder.cpp file
    
	return NULL;
}

BOOL CLiveVideo::startCapture()
{
	CLock lock(m_pMutex);
	
	if (m_hCapture)
	{
		return TRUE;
	}

	m_bCapture = TRUE;
	m_hCapture = sys_os_create_thread((void *)liveVideoThread, this);

	return m_hCapture ? TRUE : FALSE;
}

void CLiveVideo::stopCapture()
{
	m_bCapture = FALSE;
	
	while (m_hCapture)
	{
		usleep(10*1000);
	}

	// todo : here add your uninit code ...

	

	m_bInited = FALSE;
}

BOOL CLiveVideo::captureThread()
{
	// todo : when get the encoded data, call procData 

	unsigned char* data = NULL;

	int number = 0;
	FILE * fp = NULL;


	// printf("m_nDevIndex = %d\n",m_nDevIndex);

	// int ret = QueueInitial(&rtsp_srv_ServerQueue[m_nDevIndex], 5 * 1024 * 1024, 2 * 1024 * 1024);
	// printf("QueueInitial return value:%d\n", ret);

	printf("m_bCapture:%d\n", m_bCapture ? 1:0);
	data_stream_t *data_stream=data_stream_get();
    ring_buffer_t *rb = data_stream->rtsp_data_rb;
	image_frame_info_t frame_info;
	struct timeval time;
	int i=0;
	unsigned int frame_size=1024;
	int rtsp_thread_cnt=0;
	//int queue_ret=0;
	#ifdef RTSP_THREAD_TIME_LOG  
	double t0,t1,t2,t20;
	#endif
	while (m_bCapture)
	{
		if(ringbuffer_is_empty(rb)) {
			time.tv_sec=0;
			time.tv_usec=(10*1000);
			select(0,NULL,NULL,NULL,&time); 
		    continue;
		}
		 //for(i=0;i<DATA_STREAM_YUV_FRAME_MAX && (!ringbuffer_is_empty(rb)) ; i++){
		   ringbuffer_get(rb, (uint8_t *)&frame_info, sizeof(image_frame_info_t));
		 //}   
		data= data_stream->rtsp_data+frame_info.frame_offset;
		frame_size=frame_info.frame_index;


		//  if(number == 0)           
		//   {                   
		// 	printf("   rtsp data    ------------------------------------------\n");     
		// 	fp = fopen("/1.h264","wb+");              
		// 	if(!fp)            
		// 	{              
		// 		printf("oepn error\n");        
		// 	}          
		// }           
		// if(number < 60)           
		// {              
		// 	fwrite(data_stream->rtsp_data+frame_info.frame_offset,1,frame_info.frame_index,fp);             
		// 	number ++;				
		// 	printf("frame_info.frame_index:%d \n",frame_info.frame_index);        
		// }
		// if(number == 60)
		// { 
		// 	fclose(fp);
		// 	number++;
		// }

		
        // todo : here add the capture handler ... 
		// int queue_using_space = QueueGetUsingSpace(&rtsp_srv_ServerQueue[m_nDevIndex]);
		// if(queue_using_space <= 0)
		// {
		// 	usleep(1000);
		// 	continue;
		// }
		//rtsp_srv_length_count[m_nDevIndex] %= 1000;
		//queue_ret=QueueFront(&rtsp_srv_ServerQueue[m_nDevIndex],&data, rtsp_srv_length[m_nDevIndex][rtsp_srv_length_count[m_nDevIndex]]);
		
		//if(queue_ret==0){
	#ifdef RTSP_THREAD_TIME_LOG  
		t0=what_time_is_it_now();
	#endif
		procData(data,frame_size);
		//}
		//SLOGI("procData %d",rtsp_srv_length[m_nDevIndex][rtsp_srv_length_count[m_nDevIndex]]);
	#ifdef RTSP_THREAD_TIME_LOG  
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

	// printf("free rtsp_srv_ServerQueue[%d]\n",m_nDevIndex);

	// QueueFree(&rtsp_srv_ServerQueue[m_nDevIndex]);

	m_hCapture = 0;
	
	return TRUE;
}

BOOL CLiveVideo::isCallbackExist(LiveVideoDataCB pCallback, void *pUserdata)
{
	BOOL exist = FALSE;
	LiveVideoCB * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB *) p_node->p_data;
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

void CLiveVideo::addCallback(LiveVideoDataCB pCallback, void * pUserdata)
{
	if (isCallbackExist(pCallback, pUserdata))
	{
		return;
	}
	
	LiveVideoCB * p_cb = (LiveVideoCB *) malloc(sizeof(LiveVideoCB));

	p_cb->pCallback = pCallback;
	p_cb->pUserdata = pUserdata;
	p_cb->bFirst = TRUE;

	sys_os_mutex_enter(m_pCallbackMutex);
	h_list_add_at_back(m_pCallbackList, p_cb);	
	sys_os_mutex_leave(m_pCallbackMutex);
}

void CLiveVideo::delCallback(LiveVideoDataCB pCallback, void * pUserdata)
{
	LiveVideoCB * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB *) p_node->p_data;
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

void CLiveVideo::procData(uint8 * data, int size)
{
	LiveVideoCB * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveVideoCB *) p_node->p_data;

		if (p_cb->bFirst)
		{
		    // todo : If the getAuxSDPLine interface is not implemented
		    //  For H264, send SPS, PPS data
            //  For H265, send VPS, SPS, PPS data
            
		    p_cb->bFirst = FALSE;
		}
		
		if (p_cb->pCallback != NULL)
		{		    
			//printf("diaoyong callback\n");
			p_cb->pCallback(data, size, p_cb->pUserdata);
		}
		
		p_node = h_list_lookup_next(m_pCallbackList, p_node);
	}
	h_list_lookup_end(m_pCallbackList);

	sys_os_mutex_leave(m_pCallbackMutex);
}


#endif // MEDIA_LIVE


