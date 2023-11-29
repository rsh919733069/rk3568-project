/*
 * File      : GetSysInfo.h
 * This file is Get system information header
 * COPYRIGHT (C) 2019 zmvision
 *
 * Change Logs:
 * Date           Author
 * 2020-02-17     tiandiao
 */
#ifndef GET_SYS_INFO_H_
#define GET_SYS_INFO_H_


#ifdef __cplusplus
extern "C" {
#endif

#define tv_detect_open   (1<<1)
#define tv_trace_open   (1<<1)
#define tv_pip_open      (1<<6)



#define SEND_SERVO_MSG_LENGHT 40 //定义发送给伺服消息长度
#define RCV_SERVO_MSG_LENGHT 56 //定义从伺服接受消息长度

//飞机模式
#define MODE_M 0x01 //主惯导模式
#define MODE_S 0x02 //从惯导模式
#define MODE_SO 0x03 //从惯导组合模式


//激光状态
#define FLIGHT_1 0x01 //照明
#define FLIGHT_2 0x02 //照明2
#define FLIGHT_3 0x03 //照明3
#define FLIGHT_4 0x04 //照明4
#define FLIGHT_5 0x05 //照明5
#define FLIGHT_6 0x06 //照明6
#define FLIGHT_7 0x07 //照明7
#define FLIGHT_8 0x08 //照明8
#define NO_LASER 0x09 //无激光
#define NO_ECHO 0x0A //无回波
#define DEAD_ZONE 0x0B //盲区
#define RANGING 0x0C //测距
#define WAITING 0x0D //等待
#define READYED 0x0E //准备好

//系统状态
#define NO_CHANGE 0x00	//没有变化
#define INERTIA 0x01 //惯性
#define LOCK 0x02 //锁定
#define LOCK_CURRENT 0X03//锁定当前
#define ORIENTATION_SCAN 0x04 //方位扫描
#define PITCH_SCAN 0x05 //俯仰扫描
#define DIAXON_SCAN 0x06 //两轴扫描
#define RECLCLE 0x07 //回收
#define RESET 0x08//复位
#define TRACE_SEARCH 0x09 //跟踪/跟搜
#define TRACE_ 0x0A //跟踪
#define GEO_TRACE 0x0B //地理跟踪
#define PANORAMA_JOINT 0x0C//全景拼接
#define BAND_3 0x0D//3米条带
#define BAND_1 0x0E //1米条带
#define BAND_POINT_5 0x0F //0.5米条带
#define BAND_POINT_3 0x10 //0.3米条带
#define BAND_POINT_2 0x11 //0.2米条带
#define BEAMING 0x12 //聚束
#define WIDE_SLOW 0x13 //广域慢速
#define WIDE_FAST 0x14 //广域快速
#define REGION_SOLW 0x15 //区域慢速
#define REGION_FAST 0x16 //区域快速
#define AUTO_SCAN	0x17	//自动扫描
#define TRACE_ADJUST  0x18	//跟踪微调
#define DRIFT_ADJUST	0x19	//漂移修正
#define RADAR_WAIING	0x1A	//雷达待机
#define AUX_BUNCH		0x1B	//辅助聚束
#define AUX_GMTI		0x1C	//辅助GMTI
#define SELF_CHECK		0x1D	//自检
#define START			0x1E	//启动
#define TL_BIAOJIAO		0x51	//陀螺标校
#define DL_TRACE_        0x52	//地理跟踪

//记录仪状态
#define ERASURE		3	//擦除
#define PALYBACK 	2	//回放
#define RECORDER	1	//记录
#define READY		0	//就绪

//可见光控制指令
#define FIELD_PLUS 0x01 //视场+
#define FIELD_SUB 0x02 //视场-
#define FOCUS_PLUS 0x03 //焦距+
#define FOCUS_SUB 0x04 //焦距—
#define DISTANCE_FOCUS 0x05 //距离聚焦
#define MANUAL_FOCUS 0x06 //手动聚焦
#define AUTO_FOCUS 0x07 //自动聚焦
#define ONEKEY_FOCUS 0x08 //一键聚焦
#define NEARINFRARED_TRANSFOG 0x09 //近红外透雾
#define TRANSFOG_ON 0x0A //透雾开
#define TRANSFOG_OFF 0x0B //透雾关
#define ELE_MAGNIFICATION_ON 0x0C //电子变倍开
#define ELE_MAGNIFICATION_OFF 0x0D //电子变倍关
#define SINGLE_PICTURE 0x0E //单幅拍照
#define CONTINUOUS_PICTURE 0x0F //连续拍照
#define PICTURE_STOP 0x10 //停止连拍
#define ENHANCE_ON 0x11 //增强开
#define ENHANCE_OFF 0x12 //增强关
#define WIDE_DYNAMIC_ON 0x13 //宽动态开
#define WIDE_DYNAMIC_OFF 0x14 //宽动态关


//红外控制指令
#define INFRARED_FIELD_PLUS 0x01 //视场+
#define INFRARED_FIELD_SUB 0x02 //视场-
#define INFRARED_FOCUS_PLUS 0x03 //焦距+
#define INFRARED_FOCUS_SUB 0x04 //焦距—
#define TONEUP_PLIUS 0x05 //增益+
#define TONEUP_SUB 0x06 //增益-
#define BRIGHT_PLUS 0x07 //亮度+
#define BRIGHT_SUB 0x08 //亮度-
#define MANUAL 0x09 //手动
#define AUTO 0x0A //自动
#define ERECT_IMAGE 0x0B //正像
#define NEGATIVE_IMAGE 0x0C //负像
#define IMAGE_CALIBRATION 0x0D //图像校准
#define INFRARED_ENHANCE_ON 0x0E //增强开
#define INFRARED_ENHANCE_OFF 0x0F //增强关
#define INFRARED_ONEKEY_FOCUS 0x10 //一键聚焦

//激光控制指令
#define CONTINUOUS_RANGING 0x101 //连续测距
#define RANGING_OFF 0x102 //测距关
#define RANGING_1 0x103 //测距1次
#define RANGING_5 0x104 //测距5次
#define LASER_LIGHT 0x105 //激光照射
#define EMERGENCY_LIGHT 0x106 //应急照射
#define LIGHT_STOP 0x107 //照射停止

//平台控制指令
#define RISE 0x201 //上升
#define DOWN 0x202 //下降
#define GANGED_ON 0x203 //联动开
#define GANGED_OFF 0x204 //联动关
#define ADJUSTMENT 0x205 //微调

//跟踪模式
#define TRACE_MODE_1 0x01 //默认模式TM1
#define TRACE_MODE_2 0x02
#define TRACE_MODE_3 0X03

//字符控制模式
#define CROSS_CURVE 0x01	//十字线、跟踪框
#define ALL_SHOW 0x02	//全显示
#define ALL_BLANK 0x03	//全消隐
#define DEBUG 0x04	//调试

//波们大小调整
// #define AMPLIFY 0x01 //扩大
// #define DEFAULT 0x02//默认
// #define SHRINK	0x03	//缩小

typedef enum{
	CS_VIS = 0,		//彩色可见光
	ZB_INF,			//中波红外
	DB_INF,			//短波红外 字符显示 黑白可见光 2
	HB_VIS,			//黑白可见光1
}plat_light_info_en;

typedef struct SF_Plat_Info
{
	unsigned short plat_azimuth;	//方位角
	unsigned short plat_pitch;		//俯仰角
	int	target_longitude;			//目标经度
	int target_latitude;			//目标纬度
	short target_height;			//目标高度
	unsigned char target_speed;		//目标测速值
	unsigned char target_direct;	//目标方向
	unsigned char self_status;		//自检状态
	unsigned char inertnav_status;	//小惯导状态
	short fw_tl;	//方位陀螺
	short fy_tl;	//俯仰陀螺
	char  stop_flag;	//扫描停顿标志
}sf_plat_info_t;


typedef struct Sensor_Info
{
	char sensor_type[16];
	char sensor_type_infrared[16];
	float stand_angle;
	float vertical_angle;
	float coeff_irz;
	float stand_angle_vis;
	float vertical_angle_vis;
	float coeff_vis;
	float distance;
}sensor_info_t;

typedef struct Plane_Info
{
	int Mode;//0 主惯导 1 从惯导 2 从惯导组合
	double lon;
	double lat;
	float hight;
	float hdg_angle;//飞机航向角
	float pit_angle;//飞机俯仰角
	float rol_angle;//飞机横滚角
	unsigned int distance;// 距离
	unsigned char pod_mode;//吊舱模式
	unsigned char zoom;//变焦倍数
	char mgrs_s[20];
	double gauss_x;
	double gauss_y;
}plane_info_t;

typedef struct Time_Info
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int min;
	unsigned int sec;
	unsigned int m_second;
	unsigned int m_second0;
	double t0;
}time_info_t;


typedef struct Laser_Info
{
	int distance;
	int state;//激光状态：
	int code;
	int recorder_stat;//记录仪状态
	int capacity;//容量
}laser_info_t;

typedef struct Plat_Info
{
	int sys_stat;
	float hdg_angle;//飞机航向角
	float plat_azimuth;//平台方位角
	float plat_pitch;//平台俯仰角
	int radar_stat;//雷达状态
	float azimuth_angle_speed;//方位角速度
	float pitch_angle_speed;//俯仰角速度
	float fw_tl;	//方位陀螺
	float fy_tl;	//俯仰陀螺
	int dspeed_flag;	//低速补偿开关
}plat_info_t;

typedef struct Plat_16_Info
{
	int plat_longitude;		//经度
	int plat_latitude;			//纬度
	short plat_gps_height;		//GPS高程
	unsigned short plat_sp_param;		//水平参数

	unsigned short plat_cz_param;		//垂直参数
	unsigned char plat_status;			//系统平台状态 （开关1）
	unsigned char vis_switch;			//可见光（开关2）

	unsigned char inf_switch;			//红外-共光路 （开关3）
	unsigned char img_switch;			//图像板（开关4）
	unsigned char recorder_switch;		//记录仪-编码板 （开关5）
	unsigned char osd_switch;			//字符显示，跟踪模式，升降机构控制 （开关6）
	unsigned char year;					//年
	unsigned char month;				//月
	unsigned char day;					//日

	unsigned char alg_light_source;		//跟踪光源
	unsigned char alg_sf_kz_flag;		//低速补偿 和 反镜
	short dx_height;					//地形高度
}plat_16_Info;

//伺服矫正
typedef struct Plat_16_sf_jiaozheng_Info
{
	double start_time;
	unsigned char flag;					//当前是否为定焦矫正
	unsigned char send_flag;			//是否发送锁定
}plat_16_sf_jiaozheng_Info;

typedef struct Plat_16_Trace_Plat_Info
{
	unsigned short laser_distance;			//激光测距值
	unsigned short vis_focus;				//可见光焦距
	unsigned short inf_focus;				//红外焦距
	unsigned char laser_details_status;		//激光照射状态细节
	unsigned char laser_self_status;		//激光自检状态
	unsigned char active_sensor_flag;		//当前活跃传感器标志
	unsigned char vis_check_flag;			//可见光自检状态
	unsigned char inf_check_flag;			//红外自检状态
	unsigned char elevator_check_flag;		//升降装置状态
	unsigned char recorder_check_flag;		//记录仪自检状态
	unsigned char system_check_flag;		//系统自检状态

}plat_16_trace_plat_info;


typedef struct Target_Info
{
	float lon;			//经度
	float lat;			//纬度
	float height;		//高度
	float HDG;			//目标方向
	float SPD;			//目标速度
	float BRG;
	float TWD;
	float ELV;			//目标高度
	char mgrs_s[20];
	double gauss_x;
	double gauss_y;
}target_info_t;

typedef struct
{
	unsigned int x;
	unsigned int y;
}coordinate_info_t;

typedef struct
{
	float x_angle;
	float y_angle;
	float angle;
	int off_x;
	int off_y;
}joint_info_t;


typedef struct
{
	int flag;
	int x;
	int y;
	int width;
	int height;
}move_target_t;

typedef struct
{
	short stand_param;
	short pitch_param;
}stand_and_pitch_param;

typedef struct
{
	short scan_range;
	short scan_speed;
	short pitch_drive;
}scan_param;




void InitRS422Info();



//左上角区域信息
void GetSensorInfo(sensor_info_t *sensor_info); //获取传感器信息

//中间区域
int GetConnectStat(); //获取通信状态:0 通讯正常 1 通讯异常

int GetTargetStat(); //获取目标状态：0 目标正常 1 目标丢失

//右上角区域
void GetFlightInfo(plane_info_t *plane_info); //获取飞行经纬度信息
void GetCorrentTime(time_info_t *time_info);
//左下角区域
void GetLaserInfo(laser_info_t *laser_info); //获取激光信息

//中下区域
void GetPlatInfo(plat_info_t *plat_info); //获取平台信息

int IsTransfog(); //是否处于透雾状态：1 开启

int IsNearInfrared(); //是否处于近红外状态：1 开启

int IsTargetDetectStat(); //是否处于目标检测状态：1 开启

//右下角区域
void GetTargetInfo(target_info_t *target_info); //获取目标信息

int GetControlCMD(); //获取控制指令


//获取跟踪模式
int GetTraceMode();

//获取字符显示控制模式
int GetControlMode();

//波们大小调整
int GetPortDoor();

//获取可见光焦距
float GetVisibleFocal();


//获取红外焦距
float GetInfraredFocal();


int GetFuseStat();

int GetCmdType();//指令类型


int GetImgControlCmd();//图像控制类命令


int GetImgCommitCmd();//具体图像控制执行指令

void GetTraceFrameInfo(coordinate_info_t *trace_info);
void GetTraceshiZiInfo(coordinate_info_t *trace_info);

int GetPlatStat();//获取平台状态

char GetSelectTraceStat();//返回选择跟踪参数
void SetSelectTraceStat(char index);

float GetAngleNorth();
float GetAssign_Angle();

int GetOutPutSignalSrc();

void GetRecordTime(time_info_t *time_info);
//设置显示跟踪框坐标
void SetDrawTraceBlock(int x,int y);
void my_sys_time_set(time_info_t *time_info_tmp);

extern sensor_info_t Sensor_info;//传感器信息
extern plane_info_t Plane_info;//飞机信息
extern time_info_t Time_info;//时间信息
extern laser_info_t Laser_info;//激光信息
extern plat_info_t Plat_info;//平台信息
extern target_info_t Target_info;//目标信息

#ifdef __cplusplus
}
#endif


#endif //GET_SYS_INFO_H_



