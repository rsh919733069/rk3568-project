/*
 * GetSysInfo.c
 *
 *  Created on: Feb 28, 2020
 *      Author: qq
 */

#include "GetSysInfo.h"
#include "log.h"
#include "data_engine.h"

#include "video_common.h"
#include <math.h>

#define LOG_TAG "SysInfo"

sensor_info_t Sensor_info={0};//传感器信息
plane_info_t Plane_info={0};//飞机信息
time_info_t Time_info={0};//时间信息
laser_info_t Laser_info={0};//激光信息
plat_info_t Plat_info={0};//平台信息
target_info_t Target_info={0};//目标信息
stand_and_pitch_param sys_stand_and_pitch = {0};//水平和俯仰参数
scan_param sys_scan = {0};//扫描参数
int Trace_Mode = 0x01;//默认跟踪模式为TM_1
unsigned char trace_stat = 0;
int Gate_Size = 2;//默认框大小
char select_trace = 0xff;//选择跟踪参数
char estimate_frame_cnt =0;//预推帧数
float visible_focal = 0;//可见光焦距
float infrared_focal = 0;//红外焦距
unsigned char Is_compensate = 0;//是否开启地速补偿
int Fuse_Stat = 0;
float angle_north = 0;
float assign_angle = 0;//指定角度
unsigned char control_cmd;//图像控制类命令
unsigned char cmd_tye;//指令类型
unsigned char commit_cmd;//具体图像控制执行指令
unsigned char plat_stat = 0;
int PC_stat = 0;//通信状态
int show_signal_src = 0;//单路输出信号源，默认可见光
coordinate_info_t trace_posit;
coordinate_info_t trace_shizi_posit;


void my_sys_time_set(time_info_t *time_info_tmp){
	//struct tm tptr;
	//struct timeval tv;
	char  cmd[256]={0};
	sprintf(cmd,"date -s '%04d-%02d-%02d %02d:%02d:%02d' ",\
	time_info_tmp->year,time_info_tmp->month,time_info_tmp->day,\
	time_info_tmp->hour,time_info_tmp->min,time_info_tmp->sec);
	system(cmd);

	// tptr.tm_year=time_info_tmp->year;
	// tptr.tm_mon=time_info_tmp->month;
	// tptr.tm_mday=time_info_tmp->day;
	// tptr.tm_hour=time_info_tmp->hour;
	// tptr.tm_min=time_info_tmp->min;
	// tptr.tm_sec=time_info_tmp->sec;
	// tv.tv_sec=mktime(&tptr);
	// tv.tv_usec=0;
	// settimeofday(&tv,NULL);

}


//获取系统状态
void GetSystemStat(selfcheck_info_t *selfcheck_info);

void InitRS422Info()
{
	memset(&Sensor_info, 0, sizeof(sensor_info_t));
	memset(&Plane_info, 0, sizeof(plane_info_t));
	memset(&Time_info, 0, sizeof(time_info_t));
	memset(&Laser_info, 0, sizeof(laser_info_t));
	memset(&Plat_info, 0, sizeof(plat_info_t));
	memset(&Target_info, 0, sizeof(target_info_t));
	memset(&trace_posit, 0, sizeof(coordinate_info_t));

	unsigned char visible_cs[16] = "彩色可见光";
	memcpy(Sensor_info.sensor_type, visible_cs, sizeof(visible_cs));
}

void CreateServoMsg(unsigned char *msg_buf)
{
	int value_i = 0;
	short value_s = 0;

	if(Is_compensate == 0x02)
	{
		msg_buf[22] = 0x01;
	}
	else
	{
		msg_buf[22] = 0x00;
	}
	value_i = Target_info.lat *3600 /0.01;
	memcpy(&msg_buf[24],  &value_i,sizeof(int));
	value_i = Target_info.lon *3600 /0.01;
	memcpy(&msg_buf[28],  &value_i,sizeof(int));
	value_i = (short)Target_info.height;
	memcpy(&msg_buf[32],  &value_s,sizeof(short));

	if((Plat_info.sys_stat == TRACE_SEARCH) ||(Plat_info.sys_stat == TRACE_))
	{
		if((Plat_info.sys_stat == TRACE_SEARCH))
		{
			msg_buf[2] = 0x50;
		}
		else
		{
			msg_buf[2] = 0x20;
		}
		memcpy(&msg_buf[7],  &trace_posit,2*sizeof(short));
	}

}

//获取系统状态
void GetSystemStat(selfcheck_info_t *selfcheck_info)
{
	memcpy(selfcheck_info, &Selfcheck_info,sizeof(selfcheck_info_t));
	return;
}

//左上角区域信息
void GetSensorInfo(sensor_info_t *sensor_info) //获取传感器信息
{
	memcpy(sensor_info, &Sensor_info,sizeof(sensor_info_t));
	return;
}

//中间区域
int GetConnectStat() //获取通信状态:0 通讯正常 1 通讯异常
{
	return PC_stat;
}

int GetTargetStat() //获取目标状态：0 目标正常 1 目标丢失
{
	return trace_stat == 0xaa ? 1 : 0;
}

//右上角区域
void GetFlightInfo(plane_info_t *plane_info) //获取飞行经纬度信息
{
	memcpy(plane_info, &Plane_info,sizeof(plane_info_t));
}

void GetCorrentTime(time_info_t *time_info)
{
	memcpy(time_info, &gs_time_info,sizeof(time_info_t));
	return;
}
void GetRecordTime(time_info_t *time_info)
{
	memcpy(time_info, &gs_rec_time_info,sizeof(time_info_t));
	return;
}

//左下角区域
void GetLaserInfo(laser_info_t *laser_info) //获取激光信息
{
	memcpy(laser_info, &Laser_info,sizeof(laser_info_t) );
	return;
}

//中下区域
void GetPlatInfo(plat_info_t *plat_info) //获取平台信息
{
	memcpy(plat_info, &Plat_info, sizeof(plat_info_t));
	return;
}

int IsGroundSpeedCompensaion() //是否打开地速补偿：1 开启
{
	return 1;
}

int IsTransfog() //是否处于透雾状态：1 开启
{
	return 1;
}

int IsNearInfrared() //是否处于近红外状态：1 开启
{
	return 1;
}

int IsTargetDetectStat() //是否处于目标检测状态：1 开启
{
	return 1;
}

//右下角区域
void GetTargetInfo(target_info_t *target_info) //获取目标信息
{
	memcpy(target_info,&Target_info,sizeof(target_info_t));
	return;
}

int GetControlCMD() //获取控制指令
{
	return ELE_MAGNIFICATION_ON;
}


//获取跟踪模式
int GetTraceMode()
{
	return Trace_Mode;
}

//波们大小调整
int GetPortDoor()
{
	return Gate_Size;
}

//获取可见光焦距
float GetVisibleFocal()
{
	return visible_focal;
}


//获取红外焦距
float GetInfraredFocal()
{
	return infrared_focal;
}

int GetLaserDistance()
{
	return Laser_info.distance;
}

int GetFuseStat()
{
	return Fuse_Stat;
}

int GetCmdType()
{
	return cmd_tye;//指令类型
}

int GetImgControlCmd()
{
	return control_cmd;//图像控制类命令
}

int GetImgCommitCmd()
{
	return commit_cmd;//具体图像控制执行指令
}

void GetTraceFrameInfo(coordinate_info_t *trace_info)//获取跟踪框或十字框坐标
{
	memcpy(trace_info, &trace_posit,sizeof(coordinate_info_t));
	return;
}

void GetTraceshiZiInfo(coordinate_info_t *trace_info)//获取十字框坐标
{
	memcpy(trace_info, &trace_shizi_posit,sizeof(coordinate_info_t));
	return;
}

int GetPlatStat()//获取平台状态
{
	return plat_stat;
}


char GetSelectTraceStat()
{
	return select_trace;
}

char GetEstimateFrameCnt()
{
	return estimate_frame_cnt;
}

void SetSelectTraceStat(char index)
{
	select_trace = index;
}

float GetAngleNorth()
{
	return angle_north;
}

float GetAssign_Angle()
{
	return assign_angle;
}

int GetOutPutSignalSrc()
{
	return show_signal_src;
}

void SetDrawTraceBlock(int x,int y)
{
	trace_posit.x = x;
	trace_posit.y = y;

}






