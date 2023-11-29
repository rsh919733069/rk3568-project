/***************************************************************
* Copyright (C) 2011, Wuhan University
* All rights reserved.
*
* �ļ�����  Queue.c
* ����������һ������ѭ���ڴ�Ķ���
*
* ��ǰ�汾��1.0
* ��    �ߣ�������
* ������ڣ�2011��4��20��
*
***************************************************************/

//#include "terasic_includes.h"
#include <stdio.h>
#include <stdlib.h> // malloc, free
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>

#include "queue.h"

//�ͷ��ڴ�
int FreeMemory(void **p)
{
    if (NULL != *p)
    {
        free(*p);
    }
    *p = NULL;

    return 0;
}

//�ͷŷ����ڴ�
int QueueFree(QueueStruct *pQueue)
{
    FreeMemory((void **)&(pQueue->m_dataBuf));
    FreeMemory((void **)&(pQueue->m_tmpBuf));

    return 0;
}


//����ڴ���У������ͷ��ڴ�
int QueueReset(QueueStruct *pQueue)
{
    pQueue->m_incount  = 0;         //������ֽ���
    pQueue->m_outcount = 0;         //�ܳ����ֽ���
    pQueue->m_validNum = 0;         //��������Ч���ݸ���
    pQueue->m_curBlockPoint = 0;    //��ǰ���ݻ����ȡλ��

    if (pQueue->m_dataBuf)
    {
        memset(pQueue->m_dataBuf, 0, pQueue->m_length);
    }
    else
    {
        return -1;
    }

    return 0;
}


//�������ڴ��������帳ֵΪ_val
int QueueMemset(QueueStruct *pQueue,alt_u8 _val)
{
    if (pQueue->m_dataBuf)
    {
        memset(pQueue->m_dataBuf, _val, pQueue->m_length);
    }
    else
    {
        return -1;
    }

    return 0;
}


//��ȡ������ֽ���
long int QueueGetInCount(QueueStruct *pQueue) {return pQueue->m_incount;}

//��ȡ�ܳ����ֽ���
long int QueueGetOutCount(QueueStruct *pQueue) {return pQueue->m_outcount;}

//��ȡ��������ڴ�����
int QueueGetSpace(QueueStruct *pQueue) {return pQueue->m_length;}

//��ȡ������ʣ��ռ��С
int QueueGetLeavingSpace(QueueStruct *pQueue) {return (pQueue->m_length-pQueue->m_unitmaxsize)-(pQueue->m_incount-pQueue->m_outcount);}
    
    

//���ڴ��_buf���뻺�棬�ڴ���СΪ_size
int QueuePush(QueueStruct *pQueue,alt_u8 * _buf, int _size)
{
    if (NULL == pQueue->m_dataBuf)
    {
        return -1;
    }

    if (_size <= 0)
    {
        return -2;
    }

    if (pQueue->m_validNum+_size >= pQueue->m_length-pQueue->m_unitmaxsize)
    {
        return -3;
    }

    if (pQueue->m_incount%pQueue->m_length + _size <= pQueue->m_length)
    {
        memcpy(pQueue->m_dataBuf+pQueue->m_incount%pQueue->m_length, _buf, _size);
    }
    else
    {
        memcpy(pQueue->m_dataBuf + pQueue->m_incount%pQueue->m_length, _buf, pQueue->m_length-pQueue->m_incount%pQueue->m_length);
        memcpy(pQueue->m_dataBuf, _buf+pQueue->m_length-pQueue->m_incount%pQueue->m_length, _size-(pQueue->m_length-pQueue->m_incount%pQueue->m_length));
    }

    pQueue->m_incount += _size;
    pQueue->m_validNum += _size;

    return 0;
}


//��ȡ�����λ��_pos��ʼ��_size���ֽ�
alt_u8 * QueueGetBlockPointer(QueueStruct *pQueue,int _size)
{
    //_pos��_size�����ǵ�λ��С
    if (0 != pQueue->m_length%_size)
    {
        return NULL;
    }

    if ((pQueue->m_curBlockPoint > pQueue->m_incount-_size) || (_size> (pQueue->m_length)/2))
    {
        return NULL;
    }
    else
    {
        pQueue->m_curBlockPoint += _size;
        return pQueue->m_dataBuf + (pQueue->m_curBlockPoint-_size)%pQueue->m_length;
    }
}
