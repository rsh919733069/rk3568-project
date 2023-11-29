


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "config_handler.h"

#define MAX_CONFIG_FILE_LEN                                     8192
#define MAX_CONFIG_LINE_LEN                                     ((128 - sizeof(void*) - sizeof(int)) / 2)

typedef enum
{
    e_class_device,
    e_class_rtsp_client,
    e_class_rtsp_server,
    e_class_ts_send,
    e_class_ts_recv,
	e_class_track_param,
    e_class_max
}
config_class_et;

typedef struct
{
    const char *class_name;
    config_member_et member_first;
    config_member_et member_last;
}
organization_st;

const static organization_st config_class[e_class_max] = 
{
    {"Device",              e_device_softe_version,             e_device_bitrate},
    {"RtspClient",          e_rtsp_client_server_ip,            e_rtsp_client_protocol},
    {"RtspServer",          e_rtsp_server_state,                e_rtsp_server_stream_type},
    {"TsSend",              e_ts_send_state,               		e_ts_send_port},
//#ifdef ZM_MP5_PL
    {"TsRcv",               e_ts_recv_input_device,        		e_ts_rtsp_image},
//#else
//    {"TsRcv",               e_ts_recv_input_device,        		e_ts_recv_port},
//#endif
	{"TrackParam",          vis_lost_threshs0,        padding_small}
};

typedef struct
{
    const char *porpotity_name;
    unsigned flag_overwrited;
    char value_str[MAX_CONFIG_LINE_LEN];
    char overwrite[MAX_CONFIG_LINE_LEN];
}
relation_st;

static relation_st config_member[e_member_max] = 
{
 	{"e_device_softe_version",},
    {"e_device_vis_model_version",},
    {"e_device_irf_model_version",},
    {"e_device_ir_version",},
    {"e_device_input_type",},
    {"e_device_eth0",},
    {"e_device_eth1",},
    #ifdef ZM_MP5_PL
    {"e_device_ts_port",},
    {"e_osd_srt_set",},
    {"e_eth_rate",},
    #endif
    {"e_device_upward_can_id",},
    {"e_device_track_frame_delay",},
    {"e_device_tcp_ctl_port",},
    {"e_device_fps",},
#ifdef ZM_MP5_PL
    {"e_device_rknn_class",},
#endif
	{"e_device_track_search_time",},
   { "e_device_bitrate",},
   { "e_rtsp_client_server_ip",},
   { "e_rtsp_client_port",},
   { "e_rtsp_client_name",},
   { "e_rtsp_client_user_name",},
   { "e_rtsp_client_password",},
   { "e_rtsp_client_stream_type",},
  {  "e_rtsp_client_protocol",},
  {  "e_rtsp_server_state",},
   { "e_rtsp_server_port",},
   { "e_rtsp_server_stream_type",},
  {  "e_ts_send_state",},
    {"e_ts_send_stream_type",},
 { "e_ts_send_ip",},

#ifdef ZM_MP5_PL
	{ "e_mask_ip",},
	#endif

#ifdef ZM_MP5_PL
    {"e_ts_send_resolution",},
#endif

   { "e_ts_send_port",},
   { "e_ts_recv_input_device",},
    { "e_ts_recv_stream_type",},
	
//#ifdef ZM_MP5_PL
    { "e_ts_recv_port",},
    {"e_ts_rtsp_image",},
// #else
//     { "e_ts_recv_port",},
// #endif
	{ "vis_lost_threshs0",},
	{ "vis_lost_threshs1",},
	{ "vis_lost_threshs2",},
	{ "vis_lost_threshs3",},
	{ "vis_shelter_threshs0",},
	{ "vis_shelter_threshs1",},
	{ "vis_shelter_threshs2",},
	{ "vis_find_obj_threshold",},
	{ "vis_server_movement_factor",},
	{ "vis_detect_scale_factor",},
	{ "vis_refind_threshold",},
	{ "vis_max_score_threshold",},
	{ "vis_eco_search_area_scale",},
	{ "vis_learning_rate",},
	{ "vis_scale_step",},
	{ "vis_scale_max",},
	{ "vis_scale_min",},
	{ "vis_use_obj_detect_to_refind",},
	{ "vis_use_obj_detect_only",},
	{ "vis_use_forecast_pushing",},
	{ "vis_use_distance_punishment",},
	{ "vis_obj_refind_delay",},
	{ "vis_use_shelter_modules",},
	{ "vis_use_track_when_multi_obj",},
	{ "vis_print_info",},
	{ "vis_Is_useServoInfo",},
	{ "ir_lost_threshs0",},
	{ "ir_lost_threshs1",},
	{ "ir_lost_threshs2",},
	{ "ir_lost_threshs3",},
	{ "ir_shelter_threshs0",},
	{ "ir_shelter_threshs1",},
	{ "ir_shelter_threshs2",},
	{ "ir_find_obj_threshold",},
	{ "ir_server_movement_factor",},
	{ "ir_detect_scale_factor",},
	{ "ir_refind_threshold",},
	{ "ir_max_score_threshold",},
	{ "ir_eco_search_area_scale",},
	{ "ir_learning_rate",},
	{ "ir_scale_step",},
	{ "ir_scale_max",},
	{ "ir_scale_min",},
	{ "ir_use_obj_detect_to_refind",},
	{ "ir_use_obj_detect_only",},
	{ "ir_use_forecast_pushing",},
	{ "ir_use_distance_punishment",},
	{ "ir_obj_refind_delay",},
	{ "ir_use_shelter_modules",},
	{ "ir_use_track_when_multi_obj",},
	{ "ir_print_info",},
    { "width_boundary",},
	{ "high_boundary",},
	{ "padding_big",},
	{ "padding_small",},
};

static char text[MAX_CONFIG_FILE_LEN];

static int
i_mapping_class(const cJSON *json, const char *class_name, config_member_et b, config_member_et e, int method)
{
    const cJSON *class_obj = cJSON_GetObjectItemCaseSensitive(json, class_name);
    if (class_obj == 0)
    {
        if (method == LOAD_INIT_CONFIG)
        {
            fprintf(stderr, "## Class [%s] does not exist, already discarded ?\n", class_name);
            return -10;
        }
        else
            return 0;
    }
    else
    {
        int as = cJSON_GetArraySize(class_obj);
        printf("// Class [%s] have %d nodes.\n", class_name, as);
    }

    for (int i = b; i <= e; ++i)
    {
        relation_st *pr = &config_member[i];
        const cJSON *node = cJSON_GetObjectItemCaseSensitive(class_obj, pr->porpotity_name);
        pr->flag_overwrited = 0;

        if (node == 0 || !cJSON_IsString(node))
        {
            if (method == LOAD_INIT_CONFIG)
            {
                fprintf(stderr, "##   Node [%s] does not exist, maybe removed ?\n", pr->porpotity_name);
                pr->value_str[0] = '\0';
                pr->overwrite[0] = '\0';
            }
        }
        else
        {
            if (method == LOAD_CUSTOM_CONFIG)
            {
                strncpy(pr->overwrite, node->valuestring, MAX_CONFIG_LINE_LEN);
                pr->overwrite[MAX_CONFIG_LINE_LEN - 1] = '\0';
                printf("//   [OVERWRITED|%s] \"%s\" -> \"%s\"\n", pr->porpotity_name, pr->value_str, pr->overwrite);
                pr->flag_overwrited = 1;
            }
            else
            {
                strncpy(pr->value_str, node->valuestring, MAX_CONFIG_LINE_LEN);
                pr->value_str[MAX_CONFIG_LINE_LEN - 1] = '\0';
                printf("//   [%s]: \"%s\"\n", pr->porpotity_name, pr->value_str);
            }
        }
    }

    return 0;
}

int
zmv_config_file_load(const char *file_name, int method)
{
    FILE *cfg;
    cJSON *json;
    size_t read_bytes;

    if (file_name == 0)
        return -1;
    else
    {
        const char *str;
        if (method == LOAD_INIT_CONFIG)
            str = "as initial config";
        else
            str = "to overwrite current settings";
        printf("//\n// Load [%s] %s.\n", file_name, str);
    }

    cfg = fopen(file_name, "r");
    if (cfg == 0)
    {
        fprintf(stderr, "## Config file open failed.\n");
        return -2;
    }

    read_bytes = fread(text, 1, MAX_CONFIG_FILE_LEN, cfg);
    fclose(cfg);

    if (read_bytes <= 0)
    {
        fprintf(stderr, "## Config file read failed.\n");
        return -3;
    }

    json = cJSON_Parse(text);
    if (json == 0)
    {
        fprintf(stderr, "## Config file parse failed.\n");
        return -4;
    }

    for (int i = 0; i < e_class_max; ++i)
    {
        const organization_st *po = &config_class[i];
        i_mapping_class(json, po->class_name, po->member_first, po->member_last, method);
    }

    cJSON_Delete(json);    
    return 0;
}

const char *
zmv_config_get_value(config_member_et e)
{
    if (e < e_member_max)
    {
        const relation_st *pr = &config_member[e];
        return pr->flag_overwrited ? pr->overwrite : pr->value_str ;
    }
    else
        return 0;
}

void
zmv_config_set_value(config_member_et e, const char *new_val)
{
    if (e < e_member_max)
    {
        relation_st *pr = &config_member[e];
        strncpy(pr->overwrite, new_val, MAX_CONFIG_LINE_LEN);
        pr->overwrite[MAX_CONFIG_LINE_LEN - 1] = '\0';
        pr->flag_overwrited = 1;
    }
}

void
zmv_config_reset_to_default_value(config_member_et e)
{
    if (e < e_member_max)
    {
        config_member[e].flag_overwrited = 0;
        config_member[e].overwrite[0] = 0;
    }
}

int
zmv_config_file_generate(const char *file_name)
{
    FILE *cfg;
    cJSON *node, *cfg_class, *custom;
    
    if (file_name == 0)
        return -1;

    custom = cJSON_CreateObject( );
    if (custom == 0) goto err_0;

    for (int c = 0; c < e_class_max; ++c)
    {
        cfg_class = 0;
        const organization_st *po = &config_class[c];

        for (int m = po->member_first; m <= po->member_last; ++m)
        {
            relation_st *pr = &config_member[m];
            if (pr->flag_overwrited)
            {
                if (cfg_class == 0)
                {
                    cfg_class = cJSON_CreateObject( );
                    if (cfg_class == 0) goto err_1;
                }

                node = cJSON_CreateStringReference(pr->overwrite);
                if (node == 0) goto err_2;

                cJSON_AddItemToObject(cfg_class, pr->porpotity_name, node);
            }
        }

        if (cfg_class)
        {
            cJSON_AddItemToObject(custom, po->class_name, cfg_class);
        }
    }

    cJSON_PrintPreallocated(custom, text, MAX_CONFIG_FILE_LEN, 1);
    cJSON_Delete(custom);

    cfg = fopen(file_name, "w");
    if (cfg == 0)
    {
        fprintf(stderr, "## Could not create [%s]\n", file_name);
        return -2;
    }

    size_t cfg_bytes = strnlen(text, MAX_CONFIG_FILE_LEN);
    size_t write_out = fwrite(text, 1, cfg_bytes, cfg);

    if (write_out < cfg_bytes)
    {
        fprintf(stderr, "## Write new config file failed.\n");
        return -3;
    }

    fclose(cfg);
    return 0;

err_2:
    cJSON_Delete(cfg_class);
err_1:
    cJSON_Delete(custom);
err_0:
    fprintf(stderr, "## Generate new json object failed.\n");
    return -4;
}

void
zmv_config_get_value_string(config_member_et e,char* value,int size)
{
	strncpy(value,zmv_config_get_value(e),size);
}

int
zmv_config_get_value_int(config_member_et e)
{
	return atoi(zmv_config_get_value(e));
}

float
zmv_config_get_value_float(config_member_et e)
{
	return atof(zmv_config_get_value(e));
}
