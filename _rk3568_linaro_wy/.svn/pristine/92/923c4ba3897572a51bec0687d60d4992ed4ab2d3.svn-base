


#ifndef ZMV_INTERFACE_200707_H
#define ZMV_INTERFACE_200707_H

#define MAX_DMB_TARGETS                     32

#ifdef __cplusplus
extern "C" {
#endif

// 库接口版本即为代码发布时间戳，后四位保留为 0
#define TRACK_INTERFACE_VERSION             0x2022081515590000

typedef union
{
    unsigned char   uc[96];
    unsigned short  us[48];
    unsigned int    ui[24];
    int             si[24];
    float           sf[24];
    void *          pv[12];
    const void *    pc[12];
}
para_ut;

typedef struct
{
    /* 目标坐标信息 */
    float x;
    float y;
    float w;
    float h;

    /* 保留单元 */
    para_ut extra;
}
target_info_st;

typedef struct
{
    int has_info;				// 是否有额外信息
    int server_movement_x;		// 伺服器运动的x方向
    int server_movement_y;		// 伺服器运动的y方向
	float shelter_level;		// 遮挡系数
}
extra_info_st;

typedef struct
{
    float lost_threshs[4];
	float shelter_threshs[3];
    float find_obj_threshold;		// 跟踪框和检测框的iou高于此值则认定为是相同目标
    float server_movement_factor;	// 伺服器运动系数，根据此系数筛除掉跳变过大的目标
    float detect_scale_factor;		// 重捕获的搜索范围系数
    float refind_threshold;			// 重捕获阈值
    float max_score_threshold;		// 原来是放在内部的ECO参数，现在因为调试的需要而放在了这里，和内部的是同名的参数，内部的参数暂时不再使用

    // 以下是跟踪参数
    float eco_search_area_scale;	// ECO跟踪时的搜索范围
	float stc_search_area_scale;	// STC跟踪时的搜索范围
    float learning_rate;			// ECO学习率
	float scale_step;				// 尺度变化率
	float scale_max;				// 尺度上限
	float scale_min;				// 尺度下限

    // 以下是策略开关
	// 因为程序发生大改版，暂时不完善，所以use_obj_detect_to_refind和use_obj_detect_only暂时不可开
    int use_obj_detect_to_refind;	// 重捕获时将目标检测中最近的目标加入进来
	int use_obj_detect_only;		// 重捕获时仅使用目标检测的结果
    int use_forecast_pushing;		// 重捕获时是否使用软件预推
	int use_distance_punishment;	// 重捕获时是否使用距离惩罚
	int obj_refind_delay;			// 在延迟多少帧以后将检测结果加入重捕获，打开use_obj_detect_to_refind时才生效
	int use_shelter_modules;		// 是否使用遮挡模块，若不使用，则仅使用max_score进行丢失判断
    int use_track_when_multi_obj;	// 当有多个检测目标和跟踪目标重合，且检测目标也重合时，不使用检测框
	//int train_with_size;			// 更新模板时除使用检测位置以外，还使用检测尺寸
	//								// （这个可能会影响其它参数的设置，所以先用一个开关做控制）
	int print_info;					// 是否打印调试信息（便于出差时使用）
}
ShelterParamters;

typedef struct
{
    float x;
    float y;
}
point_info_st;

typedef struct
{
    float width;
    float height;
}
size_info_st;

typedef struct
{
    float x;
    float y;
    float width;
    float height;
}
rect_info_st;

typedef struct
{
    unsigned                            frames_;
    int                                 is_used_;
    int                                 is_success_;
    float                               max_score_;
    float                               score_ratio_;
    void *                              init_with_;
    point_info_st                       pos_;
    size_info_st                        target_sz_;
    size_info_st                        search_area_;
    size_info_st                        current_scale_factor_;
    rect_info_st                        last_roi_;
    rect_info_st                        result_rect_;
}
dmb_info_st;

typedef struct
{
    point_info_st                       last_global_pos;
    size_info_st                        template_size;
    size_info_st                        search_size;
    void *                              pa1;
    void *                              pa2;
    float                               sum_a2;
    unsigned                            input_w;
    unsigned                            input_h;
}
rematch_st;

typedef int (*so_puts_ft)(const char* out_str);

/*
 *  算法总初始化，仅能在程序初始化时调用一次
 *  
 *  该函数将创建所有工作线程，并初始化内存分配器以及计算所有预处理数据
 * 
 *  so_puts     : 调试输出函数回调指针，若为 NULL 则采用库内部处理
 */
void
tracker_total_prepare(so_puts_ft so_puts);

/*
 *  算法复位并设定目标
 *
 *  第一次调用将创建后台并发线程，后续调用则会同步所有线程；
 *  初始化是十分复杂的过程，涉及到 ECO/STC 协同初始化和后台任务同步；
 *  该调用耗时较多，可能超过一帧时间。
 *
 *  image       : 输入指针，原始图像数据缓冲区
 *  image_w     : 图像像素宽度
 *  image_h     ：图像像素高度
 *  img_type    : 图像类型，0 是单字节灰度图，1 是三字节彩图
 *  target      : 目标信息结构
 */
void
eco_reinit_target(
    const void *image, 
    int image_w, 
    int image_h, 
    int img_type, 
    const target_info_st *target
);

/*
 *  依据输入数据输出跟踪目标
 *
 *  image               : 输入指针，原始图像数据缓冲区
 *  target              : 输出四目标的数组，target[0]为终判目标信息，target[1]为eco原始输出，target[2]为stc原始输出，target[3]为当前检测框输出
 *  obj_detect          : 可为0，仅适用检测模式，当前帧是否有目标检测，0为无目标检测，1为有目标检测
 *  obj_detect_target   : 可为0，仅适用检测模式，输入的目标检测结果的指针，和开始目标检测的帧有3帧的帧差
 *  has_obj_detect		: 可为0，仅适用检测模式，当前帧是否有检测结果，不管是否有和跟踪框重合的
 *  extra_info          : 可为0，云台伺服机构的额外参数指针
 *  force_stc_reset     : 是否强制进行STC重新学习，1为强制进行，0为自动判断
 *
 *  跟踪成功返回0，跟踪暂时丢失返回1，学习失败必须重新初始化返回2。
 *  当target[3].extra.ui[0]为1时，是建议按此目标信息重新初始化目标，可获得更加匹配的跟踪+检测合成效果。
 */
int
eco_update_target(
    const void* image, 
    target_info_st* target, 
    int obj_detect, 
    target_info_st* obj_detect_target, 
    int target_number, 
    extra_info_st *extra_info, 
    int force_stc_reset
);

/*
 * 设置跟踪参数的档位
 * 档位越低，越偏向遮挡
 * 档位越高，越偏向形变
 */
typedef enum { 
    LEVEL_1, 
    LEVEL_2, 
    LEVEL_3, 
    LEVEL_4, 
    LEVEL_5
}
traker_level;

void
eco_set_level(traker_level lv);

/*
 * 设置遮挡参数的接口
 */
void
eco_set_shelter_parameter(ShelterParamters *param);

/*
 * 获取遮挡参数的接口
 */
void
eco_get_shelter_parameter(ShelterParamters *param);

/*
 * 设置STC参数的接口
 * 其中padding参数不会立即生效，需在init之前调用此接口修改
 */
void
eco_set_stc_parameter(float rho, float beta, float alpha, float sigma, float padding);

/*
 * 重新设置跟踪的位置
 */
void
eco_update_position(const target_info_st* refreshed_target);

/*
 * 动目标，完成关键数据结构的重新初始化
 */
void dmb_reset_all_targets( );

/*
 * 动目标，增加一个目标
 */
int dmb_add_target(
    const unsigned char *gray, 
    unsigned cols, 
    unsigned rows, 
    const rect_info_st* rect
);

/* 
 * 动目标，移除一个目标
 */
void dmb_remove_target(int idx);

/* 
 * 动目标，查询一个目标
 */
dmb_info_st* dmb_query_target(int idx);

/*
 * 动目标，更新所有目标
 */
int dmb_update_all_targets(
    const unsigned char *gray, 
    unsigned cols, 
    unsigned rows
);

/* 
 * 动目标，查询是否跟踪完成
 */
int dmb_update_finish( );

/*
 * 
 */
void rematch_init(
    rematch_st* rematch,
    unsigned input_w,
    unsigned input_h,
    unsigned char * img,
    rect_info_st* target,
    size_info_st* search_area
);

/*
 *
 */
void rematch_update_full(
    rematch_st* rematch,
    unsigned char * img,
    int is_update,
    point_info_st* ret_pos,
    float* max_score
);

/*
 *
 */
void rematch_update_part(
    rematch_st* rematch,
    unsigned char * img, 
    rect_info_st* target_rc,
    int is_update,
    int use_last_pos,
    point_info_st* ret_pos,
    float* max_score
);

/*
 * 导出内部矩阵数据，调试挂钩
 */
void debug_hook_of_mat(
    const void* dat,
    unsigned w,
    unsigned h,
    unsigned type,
    unsigned hint
);

typedef enum
{
    e_hook_mat
}
debug_hook_et;

/*
 * 注册调试挂钩
 */
void register_debug_hook(
    debug_hook_et e,
    const void * hook
);

typedef void            (*TRACKER_TOTAL_PREPARE)            (so_puts_ft);
typedef void            (*ECO_REINIT_TARGET)                (const void*, int, int, int, const target_info_st*);
typedef int             (*ECO_UPDATE_TARGET)                (const void*, target_info_st*, int, target_info_st*, int, extra_info_st*, int);
typedef void            (*ECO_SET_LEVEL)                    (traker_level);
typedef void            (*ECO_SET_SHELTER_PARAMETER)        (const ShelterParamters*);
typedef void            (*ECO_GET_SHELTER_PARAMETER)        (ShelterParamters*);
typedef void            (*ECO_SET_STC_PARAMETER)            (float, float, float, float, float);
typedef void            (*ECO_UPDATE_POSITION)              (const target_info_st*);
typedef void            (*DMB_RESET_ALL_TARGETS)            ( );
typedef int             (*DMB_ADD_TARGET)                   (const unsigned char*, unsigned, unsigned, const rect_info_st*);
typedef void            (*DMB_REMOVE_TARGET)                (int);
typedef dmb_info_st*    (*DMB_QUERY_TARGET)                 (int);
typedef int             (*DMB_UPDATE_ALL_TARGETS)           (const unsigned char*, unsigned, unsigned);
typedef int             (*DMB_UPDATE_FINISH)                ( );
typedef void            (*REGISTER_DEBUG_HOOK)              (debug_hook_et, const void *);
typedef void            (*REMATCH_INIT)                     (rematch_st*, unsigned, unsigned, unsigned char*, rect_info_st*, size_info_st*);
typedef void            (*REMATCH_UPDATE_FULL)              (rematch_st*, unsigned char*, int, point_info_st*, float*);
typedef void            (*REMATCH_UPDATE_PART)              (rematch_st*, unsigned char*, rect_info_st*, int, int, point_info_st*, float*);

typedef struct
{
    TRACKER_TOTAL_PREPARE                   tracker_total_prepare;
    // --- For ECO
    ECO_REINIT_TARGET                       eco_reinit_target;
    ECO_UPDATE_TARGET                       eco_update_target;
    ECO_SET_LEVEL                           eco_set_level;
    ECO_SET_SHELTER_PARAMETER               eco_set_shelter_parameter;
    ECO_GET_SHELTER_PARAMETER               eco_get_shelter_parameter;
    ECO_SET_STC_PARAMETER                   eco_set_stc_parameter;
    ECO_UPDATE_POSITION                     eco_update_position;
    // --- For DMB
    DMB_RESET_ALL_TARGETS                   dmb_reset_all_targets;
    DMB_ADD_TARGET                          dmb_add_target;
    DMB_REMOVE_TARGET                       dmb_remove_target;
    DMB_QUERY_TARGET                        dmb_query_target;
    DMB_UPDATE_ALL_TARGETS                  dmb_update_all_targets;
    DMB_UPDATE_FINISH                       dmb_update_finish;
    // --- For REMATCH
    REMATCH_INIT                            rematch_init;
    REMATCH_UPDATE_FULL                     rematch_update_full;
    REMATCH_UPDATE_PART                     rematch_update_part;
    // --- For DEBUG
    REGISTER_DEBUG_HOOK                     register_debug_hook;
}
tracker_reference_st;

/*
 * 跟踪库统一访问入口
 * 
 * 返回值为库接口版本
 */
unsigned long long
path_to_track_core(tracker_reference_st*);

/* 显示定义的入口函数 */
typedef void    (*PATH_TO_TRACK_CORE)                   (tracker_reference_st*);

#ifdef __cplusplus
}
#endif

#endif // ZMV_INTERFACE_200707_H
