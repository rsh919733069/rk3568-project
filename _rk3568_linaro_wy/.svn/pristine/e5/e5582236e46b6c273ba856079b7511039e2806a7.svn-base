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

#include "media_util.h"


uint32 remove_emulation_bytes(uint8* to, uint32 toMaxSize, uint8* from, uint32 fromSize) 
{
	uint32 toSize = 0;
	uint32 i = 0;
	
	while (i < fromSize && toSize+1 < toMaxSize) 
	{
		if (i+2 < fromSize && from[i] == 0 && from[i+1] == 0 && from[i+2] == 3) 
		{
			to[toSize] = to[toSize+1] = 0;
			toSize += 2;
			i += 3;
		}
		else 
		{
			to[toSize] = from[i];
			toSize += 1;
			i += 1;
		}
	}

	return toSize;
}

static uint8 * avc_find_startcode_internal(uint8 *p, uint8 *end)
{
    uint8 *a = p + 4 - ((intptr_t)p & 3);

    for (end -= 3; p < a && p < end; p++) 
    {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
        {
        	return p;
		}          
    }

    for (end -= 3; p < end; p += 4) 
    {
        uint32 x = *(const uint32*)p;
        
        if ((x - 0x01010101) & (~x) & 0x80808080) 
        { // generic
            if (p[1] == 0) 
            {
                if (p[0] == 0 && p[2] == 1)
                {
                    return p;
                }
                
                if (p[2] == 0 && p[3] == 1)
                {
                    return p+1;
                }    
            }
            
            if (p[3] == 0) 
            {
                if (p[2] == 0 && p[4] == 1)
                {
                    return p+2;
                }
                
                if (p[4] == 0 && p[5] == 1)
                {
                    return p+3;
                }    
            }
        }
    }

    for (end += 3; p < end; p++) 
    {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
        {
        	return p;
        }
    }

    return end + 3;
}

uint8 * avc_find_startcode(uint8 *p, uint8 *end)
{
    uint8 *out = avc_find_startcode_internal(p, end);
    if (p<out && out<end && !out[-1])
    {
    	out--;
    }
    
    return out;
}

uint8 * avc_split_nalu(uint8 * e_buf, int e_len, int * s_len, int * d_len)
{
    uint8 *r, *r1 = NULL, *end = e_buf + e_len;

	*s_len = 0;
	*d_len = 0;

	r = avc_find_startcode(e_buf, end);
	if (r < end)
	{
		if (r[0] == 0 && r[1] == 0 && r[2] == 0 && r[3] == 1)
		{
			*s_len = 4;
		}
		else if (r[0] == 0 && r[1] == 0 && r[2] == 1)
		{
			*s_len = 3;
		}

		while (!*(r++));

		r1 = avc_find_startcode(r, end);

		*d_len = r1 - r + *s_len;
	}
	else
	{
		*d_len = e_len;
	}

	return r1;
}

uint8 avc_h264_nalu_type(uint8 * e_buf, int len)
{
    if (len > 4 && e_buf[0] == 0 && e_buf[1] == 0 && e_buf[2] == 0 && e_buf[3] == 1)
    {
        return (e_buf[4] & 0x1f);
    }
    else if (len > 3 && e_buf[0] == 0 && e_buf[1] == 0 && e_buf[2] == 1)
    {
        return (e_buf[3] & 0x1f);
    }
    else if (len > 0)
    {
        return (e_buf[0] & 0x1f);
    }

    return 0;
}

uint8 avc_h265_nalu_type(uint8 * e_buf, int len)
{
    if (len > 4 && e_buf[0] == 0 && e_buf[1] == 0 && e_buf[2] == 0 && e_buf[3] == 1)
    {
        return ((e_buf[4] >> 1) & 0x3f);
    }
    else if (len > 3 && e_buf[0] == 0 && e_buf[1] == 0 && e_buf[2] == 1)
    {
        return ((e_buf[3] >> 1) & 0x3f);
    }
    else if (len > 0)
    {
        return ((e_buf[0] >> 1) & 0x3f);
    }

    return 0;
}



