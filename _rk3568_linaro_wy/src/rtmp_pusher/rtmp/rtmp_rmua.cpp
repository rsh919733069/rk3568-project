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
#include "rtmp_rmua.h"
#include "ppstack.h"

/***********************************************************************/

PPSN_CTX *		rmpua_fl;           // rua free list
PPSN_CTX *		rmpua_ul;           // rua used list


/***********************************************************************/
void rmpua_proxy_init()
{
	rmpua_fl = pps_ctx_fl_init(MAX_NUM_RUA, sizeof(RMPUA), TRUE);
	rmpua_ul = pps_ctx_ul_init(rmpua_fl, TRUE);
}

void rmpua_proxy_deinit()
{
	if (rmpua_ul)
	{
	    pps_ul_free(rmpua_ul);
	    rmpua_ul = NULL;
	}

	if (rmpua_fl)
	{
	    pps_fl_free(rmpua_fl);
	    rmpua_fl = NULL;
	}
}

RMPUA * rmpua_get_idle_rua()
{
	RMPUA * p_rua = (RMPUA *)pps_fl_pop(rmpua_fl);
	if (p_rua)
	{
		memset(p_rua, 0, sizeof(RMPUA));
	}
	else
	{
		log_print(HT_LOG_ERR, "%s, don't have idle rtmp rmua!!!\r\n", __FUNCTION__);
	}

	return p_rua;
}

void rmpua_set_online_rua(RMPUA * p_rua)
{
	pps_ctx_ul_add(rmpua_ul, p_rua);
	p_rua->used_flag = 1;
}

void rmpua_set_idle_rua(RMPUA * p_rua)
{
	pps_ctx_ul_del(rmpua_ul, p_rua);

	memset(p_rua, 0, sizeof(RMPUA));
	
	pps_fl_push(rmpua_fl, p_rua);
}

RMPUA * rmpua_lookup_start()
{
	return (RMPUA *)pps_lookup_start(rmpua_ul);
}

RMPUA * rmpua_lookup_next(RMPUA * p_rua)
{
	return (RMPUA *)pps_lookup_next(rmpua_ul, p_rua);
}

void rmpua_lookup_stop()
{
	pps_lookup_end(rmpua_ul);
}

uint32 rmpua_get_index(RMPUA * p_rua)
{
	return pps_get_index(rmpua_fl, p_rua);
}

RMPUA * rmpua_get_by_index(uint32 index)
{
	return (RMPUA *)pps_get_node_by_index(rmpua_fl, index);
}




