/*
 * Important:
 *
 * This file using "UTF-8 65001 with BOM" coding.
 * Choose editor coding if you could not see words below.
 *
 * “防控烦人的乱码，编辑器使用上述编码才看得到这句话！”
 * “这句就是故意给编辑器看的，它一般会识别头部，勿删。”
 * * * * * * * * * * * * * * * * * * * * * * * * * 
 * File      : jpeg_exif_impl.c
 * This file is JPEG EXIF impl file  
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-18     fengyong    first version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <libexif/exif-data.h>
#include "GetSysInfo.h"
#include "video_common.h"


static const unsigned char g_Make[16] = "AF";
static const unsigned char g_Model[16] = "CON612";
static const unsigned char g_Version[16] = "UAV_Release";

/* start of JPEG image data section */
static const unsigned int image_data_offset = 20;

/* raw EXIF header data */
static const unsigned char exif_header[] = {
 0xff, 0xd8, 0xff, 0xe1
};

/* GPS Version */
static const unsigned char g_GPSVersion[] = {
 0x02, 0x02, 0x00, 0x00
};

/* length of data in exif_header */
static const unsigned int exif_header_len = sizeof(exif_header);

/* byte order to use in the EXIF block */
#define FILE_BYTE_ORDER EXIF_BYTE_ORDER_INTEL

/* Get an existing tag, or create one if it doesn't exist */
static ExifEntry *init_tag(ExifData *exif, ExifIfd ifd, ExifTag tag)
{
    ExifEntry *entry;
    /* Return an existing tag if one exists */
    if (!((entry = exif_content_get_entry (exif->ifd[ifd], tag)))) {
        /* Allocate a new entry */
        entry = exif_entry_new ();
        assert(entry != NULL); /* catch an out of memory condition */
        entry->tag = tag; /* tag must be set before calling
        exif_content_add_entry */

        /* Attach the ExifEntry to an IFD */
        exif_content_add_entry (exif->ifd[ifd], entry);

        /* Allocate memory for the entry and fill with default data */
        exif_entry_initialize (entry, tag);

        /* Ownership of the ExifEntry has now been passed to the IFD.
        * One must be very careful in accessing a structure after
        * unref'ing it; in this case, we know "entry" won't be freed
        * because the reference count was bumped when it was added to
        * the IFD.
        */
        exif_entry_unref(entry);
    }

    return entry;
}

/* Create a brand-new tag with a data field of the given length, in the
 * given IFD. This is needed when exif_entry_initialize() isn't able to create
 * this type of tag itself, or the default data length it creates isn't the
 * correct length.
 */
static ExifEntry *create_tag(ExifData *exif, ExifIfd ifd, ExifTag tag, size_t len)
{
    void *buf;
    ExifEntry *entry;

    /* Create a memory allocator to manage this ExifEntry */
    ExifMem *mem = exif_mem_new_default();
    assert(mem != NULL); /* catch an out of memory condition */

    /* Create a new ExifEntry using our allocator */
    entry = exif_entry_new_mem (mem);
    assert(entry != NULL);

    /* Allocate memory to use for holding the tag data */
    buf = exif_mem_alloc(mem, len);
    assert(buf != NULL);

    /* Fill in the entry */
    entry->data = buf;
    entry->size = len;
    entry->tag = tag;
    entry->components = len;
    entry->format = EXIF_FORMAT_UNDEFINED;

    /* Attach the ExifEntry to an IFD */
    exif_content_add_entry (exif->ifd[ifd], entry);

    /* The ExifMem and ExifEntry are now owned elsewhere */
    exif_mem_unref(mem);
    exif_entry_unref(entry);

    return entry;
}

int af_save_jpeg(char *file_path_name, unsigned char *frame_buf, 
    unsigned int frame_len, unsigned int width, unsigned int height)
{
    int ret = 0;
    FILE *fp = NULL;

    unsigned char *exif_data = NULL;
    unsigned int exif_data_len = 0; 
    ExifEntry *entry = NULL;
    ExifData *exif = exif_data_new();
    if (!exif) {
        ret = -1;
        goto ERR_EXIT;
    }
    /* Set the image options */
    exif_data_set_option(exif, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
    exif_data_set_data_type(exif, EXIF_DATA_TYPE_COMPRESSED);
    exif_data_set_byte_order(exif, FILE_BYTE_ORDER);

    /* Create the mandatory EXIF fields with default data */
    exif_data_fix(exif);

    /* All these tags are created with default values by exif_data_fix() */
    /* Change the data to the correct values for this image. */
    entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION);
    exif_set_long(entry->data, FILE_BYTE_ORDER, width);

    entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION);
    exif_set_long(entry->data, FILE_BYTE_ORDER, height);

    entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE);
    exif_set_short(entry->data, FILE_BYTE_ORDER, 1);

    entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_VERSION_ID, 4 * exif_format_get_size(EXIF_FORMAT_BYTE));
    entry->format = EXIF_FORMAT_BYTE;
    entry->components = 4;
    int i = 0;
    for (i = 0; i < 4; i++) {
        exif_set_sshort(entry->data + i, FILE_BYTE_ORDER, g_GPSVersion[i]);
    }


    plane_info_t plane_info_tmp;
	GetFlightInfo(&plane_info_tmp);



    // // 上海北纬 30度40分~31度53分 东经 120度51分~122度12分
    entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE_REF, 2 * exif_format_get_size(EXIF_FORMAT_ASCII));
    entry->format = EXIF_FORMAT_ASCII;
    entry->components = 2;

  
	int lat_du = 0;
	int lat_min = 0;
	float lat_second = 0;
	int coff_lat = 1;
	if(plane_info_tmp.lat < 0)
	{
		memcpy(entry->data,"S", 2);// N北纬/S南纬
		coff_lat = -1;
	}else{
		memcpy(entry->data, "N", 2);// N北纬/S南纬
	}
	lat_du = (int)(plane_info_tmp.lat);
	double value = ((plane_info_tmp.lat -lat_du)* 60) * coff_lat;
	lat_min = (int)(value);
	lat_second = (value-lat_min)* 60;

    entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE, 3 * exif_format_get_size(EXIF_FORMAT_RATIONAL));
    entry->format = EXIF_FORMAT_RATIONAL;
    entry->components = 3;
    ExifRational fir, mid, las;
    fir.numerator = lat_du;
    fir.denominator = 1;
    mid.numerator = lat_min;
    mid.denominator = 1;
    las.numerator = lat_second*10000000;
    las.denominator = 10000000;
    exif_set_rational(entry->data, FILE_BYTE_ORDER, fir);
    exif_set_rational(entry->data+8, FILE_BYTE_ORDER, mid);
    exif_set_rational(entry->data+16, FILE_BYTE_ORDER, las);

    entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE_REF, 2 * exif_format_get_size(EXIF_FORMAT_ASCII));
    entry->format = EXIF_FORMAT_ASCII;
    entry->components = 2;

   	int lon_du = 0;
	int lon_min = 0;
	float lon_second = 0;
	int coff_lon = 1;
	if(plane_info_tmp.lon < 0)
	{
		memcpy(entry->data, "W", 2);// N北纬/S南纬
		coff_lon = -1;
	}else{
		memcpy(entry->data, "E", 2);// N北纬/S南纬
	}
	lon_du = (int)(plane_info_tmp.lon);
	value = ((plane_info_tmp.lon - lon_du)* 60)*coff_lon;
	lon_min = (int)(value);
	lon_second = (value - lon_min) * 60;

    entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE, 3 * exif_format_get_size(EXIF_FORMAT_RATIONAL));
    entry->format = EXIF_FORMAT_RATIONAL;
    entry->components = 3;
    fir.numerator = lon_du;
    fir.denominator = 1;
    mid.numerator = lon_min;
    mid.denominator = 1;
    las.numerator = lon_second*10000000;
    las.denominator = 10000000;
    exif_set_rational(entry->data, FILE_BYTE_ORDER, fir);
    exif_set_rational(entry->data+8, FILE_BYTE_ORDER, mid);
    exif_set_rational(entry->data+16, FILE_BYTE_ORDER, las);
    
    // // 绝对-海平面
    // entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_ALTITUDE_REF, 1 * exif_format_get_size(EXIF_FORMAT_BYTE));
    // entry->format = EXIF_FORMAT_BYTE;
    // entry->components = 1;
    // exif_set_short(entry->data, FILE_BYTE_ORDER, 0);// 0-海面上 1-海面下

    // 高度
    entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_ALTITUDE, 1 * exif_format_get_size(EXIF_FORMAT_RATIONAL));
    entry->format = EXIF_FORMAT_RATIONAL;
    entry->components = 1;
    fir.numerator = plane_info_tmp.hight*10;
    fir.denominator =10;
    exif_set_rational(entry->data, FILE_BYTE_ORDER, fir);


    // //GPS速度单位K KM/H
    // entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_SPEED_REF, 2 * exif_format_get_size(EXIF_FORMAT_ASCII));
    // entry->format = EXIF_FORMAT_ASCII;
    // entry->components = 2;
    // memcpy(entry->data, "K", 2);

    // //GPS速度值
    // entry = create_tag(exif, EXIF_IFD_GPS, EXIF_TAG_GPS_SPEED, 1 * exif_format_get_size(EXIF_FORMAT_RATIONAL));
    // entry->format = EXIF_FORMAT_RATIONAL;
    // entry->components = 1;
    // fir.numerator = 50;
    // fir.denominator = 1;
    // exif_set_rational(entry->data, FILE_BYTE_ORDER, fir);

    // // 拍摄时间 
    // EXIF_TAG_SUB_SEC_TIME EXIF_TAG_SUB_SEC_TIME_ORIGINAL EXIF_TAG_SUB_SEC_TIME_DIGITIZED 毫秒时间不写入
    entry = create_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL, 20 * exif_format_get_size(EXIF_FORMAT_ASCII));
    entry->format = EXIF_FORMAT_ASCII;
    entry->components = 20;
    // time_t t = time(NULL);
    // struct tm st_time;
    // localtime_r(&t, &st_time); 
    char data_time[20]="2021:09:28 16:40:32";
    time_info_t time_info_tmp;
	GetCorrentTime(&time_info_tmp);
    sprintf(data_time,"%04d:%02d:%02d %02d:%02d:%02d",time_info_tmp.year,time_info_tmp.month,\
    time_info_tmp.day,time_info_tmp.hour, time_info_tmp.min,time_info_tmp.sec);
    data_time[19]=0;
    //printf("%s \n",data_time);
    memcpy(entry->data, data_time,20);

    // // 数字化时间
    // entry = create_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_DIGITIZED, 20 * exif_format_get_size(EXIF_FORMAT_ASCII));
    // entry->format = EXIF_FORMAT_ASCII;
    // entry->components = 20;
    // memcpy(entry->data, data_time, sizeof(data_time));

    // //制造商
    // entry = create_tag(exif, EXIF_IFD_0, EXIF_TAG_MAKE, sizeof(g_Make) * exif_format_get_size(EXIF_FORMAT_ASCII));
    // entry->format = EXIF_FORMAT_ASCII;
    // entry->components = sizeof(g_Make);
    // memcpy(entry->data, g_Make, sizeof(g_Make));

    // // 型号
    // entry = create_tag(exif, EXIF_IFD_0, EXIF_TAG_MODEL, sizeof(g_Model) * exif_format_get_size(EXIF_FORMAT_ASCII));
    // entry->format = EXIF_FORMAT_ASCII;
    // entry->components = sizeof(g_Model);
    // memcpy(entry->data, g_Model, sizeof(g_Model));

    // // // 修改时间
    // entry = create_tag(exif, EXIF_IFD_0, EXIF_TAG_DATE_TIME, 20 * exif_format_get_size(EXIF_FORMAT_ASCII));
    // entry->format = EXIF_FORMAT_ASCII;
    // entry->components = 20;
    // memcpy(entry->data, data_time, sizeof(data_time));

    // // 软件
    entry = create_tag(exif, EXIF_IFD_0, EXIF_TAG_SOFTWARE, sizeof(Selfcheck_info.soft_ver) * exif_format_get_size(EXIF_FORMAT_ASCII));
    entry->format = EXIF_FORMAT_ASCII;
    entry->components = sizeof(Selfcheck_info.soft_ver);
    memcpy(entry->data, Selfcheck_info.soft_ver, sizeof(Selfcheck_info.soft_ver));
    
    exif_data_save_data(exif, &exif_data, &exif_data_len);
    assert(exif_data != NULL);

    fp = fopen(file_path_name, "wb+");
    if (!fp) {
        ret = -1;
        goto ERR_EXIT;
    }

    /* Write EXIF header */
    if (fwrite(exif_header, exif_header_len, 1, fp) != 1) {
        ret = -1;
        goto ERR_EXIT;
    }
    /* Write EXIF block length in big-endian order */
    if (fputc((exif_data_len+2) >> 8, fp) < 0) {
        ret = -1;
        goto ERR_EXIT;
    }
    if (fputc((exif_data_len+2) & 0xff, fp) < 0) {
        ret = -1;
        goto ERR_EXIT;
    }
    /* Write EXIF data block */
    if (fwrite(exif_data, exif_data_len, 1, fp) != 1) {
        ret = -1;
        goto ERR_EXIT;
    }
    /* Write JPEG image data, skipping the non-EXIF header */
    if (fwrite(frame_buf + image_data_offset, frame_len - image_data_offset, 1, fp) != 1) {
        ret = -1;
        goto ERR_EXIT;
    }
    fflush_unlocked(fp);
ERR_EXIT:
    if (fp) {
        fclose(fp);
    }
    if (exif_data) {
        free(exif_data);
    }
    if (exif) {
        exif_data_unref(exif);
    }

    return ret;
}

