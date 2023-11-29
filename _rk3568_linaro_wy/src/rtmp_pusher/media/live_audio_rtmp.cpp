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
#include "live_audio_rtmp.h"
#include "lock.h"
#include "media_format.h"

#ifdef MEDIA_LIVE

/***************************************************************************************/

CLiveAudio_rtmp * CLiveAudio_rtmp::m_pInstance[] = {NULL, NULL, NULL, NULL};
void * CLiveAudio_rtmp::m_pInstMutex = sys_os_create_mutex();


/***************************************************************************************/

void * rtmp_liveAudioThread(void * argv)
{
	CLiveAudio_rtmp *capture = (CLiveAudio_rtmp *)argv;

	capture->captureThread();

	return NULL;
}

/***************************************************************************************/

CLiveAudio_rtmp::CLiveAudio_rtmp()
{ 
    m_nStreamIndex = 0;
    m_bCapture = FALSE;
    m_hCapture = 0;
    m_nCodecId = AUDIO_CODEC_NONE;
	m_nSampleRate = 8000;
	m_nBitrate = 0;
	m_nChannel = 1;
	
	m_pMutex = sys_os_create_mutex();	
	m_bInited = FALSE;	
	m_nRefCnt = 0;  

	m_pCallbackMutex = sys_os_create_mutex();
    m_pCallbackList = h_list_create(FALSE);
}

CLiveAudio_rtmp::~CLiveAudio_rtmp()
{ 
	stopCapture();
	
	sys_os_destroy_sig_mutex(m_pMutex);

	h_list_free_container(m_pCallbackList);
	
	sys_os_destroy_sig_mutex(m_pCallbackMutex);
}

CLiveAudio_rtmp * CLiveAudio_rtmp::getInstance(int idx)
{
	if (idx < 0 || idx >= MAX_LIVE_AUDIO_NUMS)
	{
		return NULL;
	}
	
	if (NULL == m_pInstance[idx])
	{
		sys_os_mutex_enter(m_pInstMutex);

		if (NULL == m_pInstance[idx])
		{
			m_pInstance[idx] = new CLiveAudio_rtmp;
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

void CLiveAudio_rtmp::freeInstance(int idx)
{
	if (idx < 0 || idx >= MAX_LIVE_AUDIO_NUMS)
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

char * CLiveAudio_rtmp::getAuxSDPLine(int rtp_pt)
{
    // For AAC, Get AAC-specification parameters from the hardware encoder
    //  Construct "a = fmt:96 xxx" SDP line
    //  Please reference the CAudioEncoder::getAuxSDPLine function in audio_encoder.cpp file

    if (AUDIO_CODEC_AAC != m_nCodecId)
    {
        return NULL;
    }

    int rate_idx = 0;
    
    switch (m_nSampleRate)
    {
    case 96000:
        rate_idx = 0;
        break;

    case 88200:
        rate_idx = 1;
        break;
        
    case 64000:
        rate_idx = 2;
        break;
        
    case 48000:
        rate_idx = 3;
        break;
        
    case 44100:
        rate_idx = 4;
        break;
        
    case 32000:
        rate_idx = 5;
        break;
        
    case 24000:
        rate_idx = 6;
        break;

    case 22050:
        rate_idx = 7;
        break;

    case 16000:
        rate_idx = 8;
        break;     

    case 12000:
        rate_idx = 9;
        break;

    case 11025:
        rate_idx = 10;
        break;

    case 8000:
        rate_idx = 11;
        break;

    case 7350:
        rate_idx = 12;
        break;
        
    default:
        rate_idx = 4;
        break;
    }

    uint8 config[2];
    uint8 audioObjectType = 2; // AAC-LC
    int   configSize = 2;
    
    config[0] = (audioObjectType << 3) | (rate_idx >> 1);
    config[1] = (rate_idx << 7) | (m_nChannel << 3);

    char const* fmtpFmt =
        "a=fmtp:%d "
    	"streamtype=5;profile-level-id=1;"
    	"mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;"
    	"config=";
	uint32 fmtpFmtSize = strlen(fmtpFmt)
	    + 3 /* max char len */
	    + 2 * configSize;

	char* fmtp = new char[fmtpFmtSize+1];
	memset(fmtp, 0, fmtpFmtSize+1);
	
	sprintf(fmtp, fmtpFmt, rtp_pt);
	char* endPtr = &fmtp[strlen(fmtp)];
	for (int i = 0; i < configSize; ++i) 
	{
		sprintf(endPtr, "%02X", config[i]);
		endPtr += 2;
	}

	return fmtp;
}

int CLiveAudio_rtmp::getStreamNums()
{
	// todo : return the max number of streams supported, don't be more than MAX_LIVE_AUDIO_NUMS
	
    return 1;
}

BOOL CLiveAudio_rtmp::initCapture(int codec, int samplerate, int channels, int bitrate)
{
	CLock lock(m_pMutex);
	
	if (m_bInited)
	{
		return TRUE;
	}

    m_nCodecId = codec;
	m_nSampleRate = samplerate;
	m_nChannel = channels;
	m_nBitrate = bitrate;

	// todo : here add your init code ... 
	

	m_bInited = TRUE;

	return TRUE;
}

BOOL CLiveAudio_rtmp::startCapture()
{
    CLock lock(m_pMutex);
	
	if (m_hCapture)
	{
		return TRUE;
	}

	m_bCapture = TRUE;
	m_hCapture = sys_os_create_thread((void *)rtmp_liveAudioThread, this);

	return m_hCapture ? TRUE : FALSE;
}

void CLiveAudio_rtmp::stopCapture(void)
{
	m_bCapture = FALSE;
	
	while (m_hCapture)
	{
		usleep(10*1000);
	}

	// todo : here add your uninit code ...

	

	m_bInited = FALSE;
}

BOOL CLiveAudio_rtmp::captureThread()
{
	// todo : when get the encoded data, call procData 

	while (m_bCapture)
	{
        // todo : here add the capture handler ... 


        usleep(10*1000);
	}

	m_hCapture = 0;
	
	return TRUE;
}

BOOL CLiveAudio_rtmp::isCallbackExist(LiveAudioDataCB_rtmp pCallback, void *pUserdata)
{
	BOOL exist = FALSE;
	LiveAudioCB_rtmp * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveAudioCB_rtmp *) p_node->p_data;
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

void CLiveAudio_rtmp::addCallback(LiveAudioDataCB_rtmp pCallback, void * pUserdata)
{
	if (isCallbackExist(pCallback, pUserdata))
	{
		return;
	}
	
	LiveAudioCB_rtmp * p_cb = (LiveAudioCB_rtmp *) malloc(sizeof(LiveAudioCB_rtmp));

	p_cb->pCallback = pCallback;
	p_cb->pUserdata = pUserdata;
	p_cb->bFirst = TRUE;

	sys_os_mutex_enter(m_pCallbackMutex);
	h_list_add_at_back(m_pCallbackList, p_cb);	
	sys_os_mutex_leave(m_pCallbackMutex);
}

void CLiveAudio_rtmp::delCallback(LiveAudioDataCB_rtmp pCallback, void * pUserdata)
{
	LiveAudioCB_rtmp * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveAudioCB_rtmp *) p_node->p_data;
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

void CLiveAudio_rtmp::procData(uint8 * data, int size, int nbsamples)
{
	LiveAudioCB_rtmp * p_cb = NULL;
	LINKED_NODE * p_node = NULL;
	
	sys_os_mutex_enter(m_pCallbackMutex);

	p_node = h_list_lookup_start(m_pCallbackList);
	while (p_node)
	{
		p_cb = (LiveAudioCB_rtmp *) p_node->p_data;
		if (p_cb->pCallback != NULL)
		{ 
			p_cb->pCallback(data, size, nbsamples, p_cb->pUserdata);
		}
		
		p_node = h_list_lookup_next(m_pCallbackList, p_node);
	}
	h_list_lookup_end(m_pCallbackList);

	sys_os_mutex_leave(m_pCallbackMutex);
}

BOOL media_live_put_audio(int idx, uint8 * data, int size, int nbsamples)
{
    CLiveAudio_rtmp * live = CLiveAudio_rtmp::getInstance(idx);
    if (NULL == live)
    {
        return FALSE;
    }

    live->procData(data, size, nbsamples);

    live->freeInstance(idx);
    
    return TRUE;
}

#endif // end of MEDIA_LIVE


