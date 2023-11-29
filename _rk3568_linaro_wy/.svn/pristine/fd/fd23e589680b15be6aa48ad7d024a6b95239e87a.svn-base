

#ifndef _TSMUX_H_
#define _TSMUX_H_

#ifdef __cplusplus
extern "C" {
#endif
/**************************
IN:
    iChannel: 0/1/2
	iEncType: 0--H264;1--H265;
OUT:
    1  - success
    -1 - fail
*****************************/
int SetEncType(int iChannel, int iEncType);

/**************************
IN:
    iChannel: 0/1/2
OUT:
	1  - success
	-1 - fail
*****************************/
int GetEncType(int iChannel);

/**************************
IN:
    iChannel: 0/1/2
	outBuffer: ts data buffer
	pData: es data(1 frame)
	videoframetype: 0:  IFRAME, 1: P FRAME, 2: B FRAME
	framerate: video frame rate
	len:pData length
	u64PTS:encode data PTS  (use VENC_PACK_S  u64PTS)
	
RETURN: ts data length
*****************************/
int ConverES2TS(int iChannel, unsigned char *outBuffer,unsigned char *pData,int videoframetype,int framerate, int len,unsigned long long u64PTS);//ES(from Temp) convert to TS ,then cpy to ringfifo.buffer

/**************************
IN:
    iChannel: 0/1/2
	cBufIn: input ts data buffer
	iLenIn: input ts length(Must be a multiple of 188,length=n*188)
	
RETURN: -2: The input ts length(iLenIn) isnot a multiple of 188.
		-1: Authentication failed,and test time is over. 
		 0: Frame is not complete,do not copy data,continue call this function
		 1: Get 1 frame,copy data from es_buf[0]
		 2: Get 2 frame,copy data from es_buf[0] & es_buf[1]
	    ...
		 5: Get 5 frame,copy data from es_buf[0] & es_buf[1] & es_buf[2] & es_buf[3] & es_buf[4]
*****************************/
int ConverTS2ES(int iChannel, char *cBufIn,int iLenIn);//TS convert to ES



#ifdef __cplusplus
}
#endif

#endif 
