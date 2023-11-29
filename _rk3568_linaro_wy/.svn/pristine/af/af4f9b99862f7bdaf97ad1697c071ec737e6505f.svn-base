


#ifndef ZMV_CONFIG_HANDLER_210220_H
#define ZMV_CONFIG_HANDLER_210220_H

#ifdef __cplusplus
extern "C" {
#endif

#define LOAD_INIT_CONFIG                                            0
#define LOAD_CUSTOM_CONFIG                                          1

typedef enum
{
	e_device_softe_version,
	e_device_vis_model_version,
    e_device_irf_model_version,
	e_device_ir_version,
    e_device_input_type,
    e_device_eth0,
    e_device_eth1,
#ifdef ZM_MP5_PL
	e_device_ts_port,
	e_osd_srt_set,
	e_eth_rate,
#endif
    e_device_upward_can_id,
    e_device_track_frame_delay,
    e_device_tcp_ctl_port,
	e_device_fps,
#ifdef ZM_MP5_PL
	e_device_rknn_class,
#endif
	e_device_track_search_time,
    e_device_bitrate,
    e_rtsp_client_server_ip,
    e_rtsp_client_port,
    e_rtsp_client_name,
    e_rtsp_client_user_name,
    e_rtsp_client_password,
    e_rtsp_client_stream_type,
    e_rtsp_client_protocol,
    e_rtsp_server_state,
    e_rtsp_server_port,
    e_rtsp_server_stream_type,
    e_ts_send_state,
    e_ts_send_stream_type,
	e_ts_send_ip,
	
	#ifdef ZM_MP5_PL
	e_mask_ip,
	#endif
	
#ifdef ZM_MP5_PL
	e_ts_send_resolution,
#endif

    e_ts_send_port,
    e_ts_recv_input_device,
    e_ts_recv_stream_type,
    e_ts_recv_port,
    e_ts_rtsp_image,           //  对RTSP过图传的配置
	vis_lost_threshs0,
	vis_lost_threshs1,
	vis_lost_threshs2,
	vis_lost_threshs3,
	vis_shelter_threshs0,
	vis_shelter_threshs1,
	vis_shelter_threshs2,
	vis_find_obj_threshold,
	vis_server_movement_factor,
	vis_detect_scale_factor,
	vis_refind_threshold,
	vis_max_score_threshold,
	vis_eco_search_area_scale,
	vis_learning_rate,
	vis_scale_step,
	vis_scale_max,
	vis_scale_min,
	vis_use_obj_detect_to_refind,
	vis_use_obj_detect_only,
	vis_use_forecast_pushing,
	vis_use_distance_punishment,
	vis_obj_refind_delay,
	vis_use_shelter_modules,
	vis_use_track_when_multi_obj,
	vis_print_info,
	vis_Is_useServoInfo,
	ir_lost_threshs0,
	ir_lost_threshs1,
	ir_lost_threshs2,
	ir_lost_threshs3,
	ir_shelter_threshs0,
	ir_shelter_threshs1,
	ir_shelter_threshs2,
	ir_find_obj_threshold,
	ir_server_movement_factor,
	ir_detect_scale_factor,
	ir_refind_threshold,
	ir_max_score_threshold,
	ir_eco_search_area_scale,
	ir_learning_rate,
	ir_scale_step,
	ir_scale_max,
	ir_scale_min,
	ir_use_obj_detect_to_refind,
	ir_use_obj_detect_only,
	ir_use_forecast_pushing,
	ir_use_distance_punishment,
	ir_obj_refind_delay,
	ir_use_shelter_modules,
	ir_use_track_when_multi_obj,
	ir_print_info,
	width_boundary,
	high_boundary,
	padding_big,
	padding_small,
	e_member_max
}
config_member_et;

int
zmv_config_file_load(const char *file_name, int method);

const char *
zmv_config_get_value(config_member_et e);

void
zmv_config_set_value(config_member_et e, const char *new_val);

void
zmv_config_reset_to_default_value(config_member_et e);

int
zmv_config_file_generate(const char *file_name);

void
zmv_config_get_value_string(config_member_et e,char* value,int size);

int
zmv_config_get_value_int(config_member_et e);

float
zmv_config_get_value_float(config_member_et e);

#ifdef __cplusplus
}
#endif

#endif // ZMV_CONFIG_HANDLER_210220_H
