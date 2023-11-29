cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

#set(toolchain /home/calibrec/work/rk3566/gcc-buildroot-9.3.0-2020.03-x86_64_aarch64-rockchip-linux-gnu)
#set(toolchain /home/sailing/works/3566/3566_gnu)

set(CMAKE_C_COMPILER   aarch64-linux-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

project(rk3566_prj)

set(TARGETS "wyq_app")

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-O2 -std=c++11 -Wno-error")


if (XT_3566 STREQUAL 1)
	add_definitions(-DZM_MP5_XT)
	message("DZM_MP5_XT")
endif()

if (PL_3566 STREQUAL 1)
	add_definitions(-DZM_MP5_PL)
	message("DZM_MP5_PL")
endif()


add_definitions(-D_GNU_SOURCE -DLINUX -DRKISP -DMEDIA_LIVE -DEPOLL)

include_directories(
	${CMAKE_CURRENT_LIST_DIR}/include/rockchip/
	${CMAKE_CURRENT_LIST_DIR}/include/mpp/
	${CMAKE_CURRENT_LIST_DIR}/include/mpp/osal/
	${CMAKE_CURRENT_LIST_DIR}/include/drm/
	${CMAKE_CURRENT_LIST_DIR}/include/drm/freedreno/
	${CMAKE_CURRENT_LIST_DIR}/include/drm/libdrm/
	${CMAKE_CURRENT_LIST_DIR}/include/drm/libkms/
	${CMAKE_CURRENT_LIST_DIR}/include/rtsp_server/
	${CMAKE_CURRENT_LIST_DIR}/include/eco/  
	${CMAKE_CURRENT_LIST_DIR}/include/common/  
	${CMAKE_CURRENT_LIST_DIR}/include/nnie/
	${CMAKE_CURRENT_LIST_DIR}/lib/3rdparty/opencv/arm/include/  
	${CMAKE_CURRENT_LIST_DIR}/lib/rknn_api_0.95/include/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/interface/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/xcore/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/xcore/ia/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/ext/rkisp/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/plugins/3a/rkiq/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/modules/isp/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/rkisp/ia-engine/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/rkisp/ia-engine/include/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/rkisp/ia-engine/include/linux/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/rkisp/ia-engine/include/linux/media/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/rkisp/isp-engine/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/metadata/libcamera_client/include/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/metadata/libcamera_metadata/include/
	${CMAKE_CURRENT_LIST_DIR}/include/rkisp/metadata/header_files/include/system/core/include/
	${CMAKE_CURRENT_LIST_DIR}/include/usb_camera/
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/Interface
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/ucGUI/
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/ucGUI/Config/
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/ucGUI/GUI/Core/
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/ucGUI/GUI/MultiLayer/
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/ucGUI/GUI/Widget/
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/ucGUI/GUI/WM/
	${CMAKE_CURRENT_LIST_DIR}/include/libavformat/
	${CMAKE_CURRENT_LIST_DIR}/include/libavutil/
	${CMAKE_CURRENT_LIST_DIR}/include/libavcodec/
	${CMAKE_CURRENT_LIST_DIR}/include/
	${CMAKE_CURRENT_LIST_DIR}/include/openssl/
	${CMAKE_CURRENT_LIST_DIR}/include/mp4v2/
    ${CMAKE_CURRENT_LIST_DIR}/src/
	${CMAKE_CURRENT_LIST_DIR}/src/common/	
	${CMAKE_CURRENT_LIST_DIR}/src/log/	
	${CMAKE_CURRENT_LIST_DIR}/src/drm/
	${CMAKE_CURRENT_LIST_DIR}/src/gpu/	
	${CMAKE_CURRENT_LIST_DIR}/src/rga/
	${CMAKE_CURRENT_LIST_DIR}/src/mpp/
	${CMAKE_CURRENT_LIST_DIR}/src/rkisp/
    ${CMAKE_CURRENT_LIST_DIR}/src/rtsp/
	${CMAKE_CURRENT_LIST_DIR}/src/device/
	${CMAKE_CURRENT_LIST_DIR}/src/uvc/
	${CMAKE_CURRENT_LIST_DIR}/src/osd/
	${CMAKE_CURRENT_LIST_DIR}/src/protocol/
	${CMAKE_CURRENT_LIST_DIR}/src/can/
	${CMAKE_CURRENT_LIST_DIR}/src/algorithm/eco/
	${CMAKE_CURRENT_LIST_DIR}/src/algorithm/rknn/
	${CMAKE_CURRENT_LIST_DIR}/src/system/
	${CMAKE_CURRENT_LIST_DIR}/src/drm/
	${CMAKE_CURRENT_LIST_DIR}/src/rtsp_srv/bm/
	${CMAKE_CURRENT_LIST_DIR}/src/rtsp_srv/media/
	${CMAKE_CURRENT_LIST_DIR}/src/rtsp_srv/rtp/
	${CMAKE_CURRENT_LIST_DIR}/src/rtsp_srv/rtsp/

	${CMAKE_CURRENT_LIST_DIR}/src/rtmp_pusher/librtmp/
	${CMAKE_CURRENT_LIST_DIR}/src/rtmp_pusher/rtp/
	${CMAKE_CURRENT_LIST_DIR}/src/rtmp_pusher/rtmp/
	${CMAKE_CURRENT_LIST_DIR}/src/rtmp_pusher/media/
	${CMAKE_CURRENT_LIST_DIR}/src/record/
)

link_directories(
	${CMAKE_CURRENT_LIST_DIR}/env_lib/
	${CMAKE_CURRENT_LIST_DIR}/lib/rockchip/
	${CMAKE_CURRENT_LIST_DIR}/lib/rtsp/
	${CMAKE_CURRENT_LIST_DIR}/lib/rkisp/		
	${CMAKE_CURRENT_LIST_DIR}/lib/eco/
	${CMAKE_CURRENT_LIST_DIR}/lib/nnie/	
	${CMAKE_CURRENT_LIST_DIR}/lib/drm/	
	${CMAKE_CURRENT_LIST_DIR}/lib/3rdparty/opencv/arm/lib64/
	${CMAKE_CURRENT_LIST_DIR}/lib//rknn_api_0.95/lib64/
	${CMAKE_CURRENT_LIST_DIR}/lib/rkisp/
	${CMAKE_CURRENT_LIST_DIR}/lib/usb_camera/
	${CMAKE_CURRENT_LIST_DIR}/include/UCGUI/
	${CMAKE_CURRENT_LIST_DIR}/lib/common/
	${CMAKE_CURRENT_LIST_DIR}/lib/gxi_api/
)
aux_source_directory(./src DIR_SRCS)
aux_source_directory(./src/common DIR_SRCS_COMMON)
aux_source_directory(./src/log DIR_SRCS_LOG)
aux_source_directory(./src/drm DIR_SRCS_DRM)
aux_source_directory(./src/gpu DIR_SRCS_GPU)
aux_source_directory(./src/rga DIR_SRCS_RGA)
aux_source_directory(./src/mpp DIR_SRCS_MPP)
aux_source_directory(./src/rkisp DIR_SRCS_RKISP)
aux_source_directory(./src/rtsp DIR_SRCS_RTSP)
aux_source_directory(./src/device DIR_SRCS_DEVICE)
aux_source_directory(./src/uvc DIR_SRCS_DEVICE_UVC)
aux_source_directory(./src/osd DIR_SRCS_OSD)
aux_source_directory(./src/protocol DIR_SRCS_PROTOCOL)
aux_source_directory(./src/can DIR_SRCS_CAN)
aux_source_directory(./src/algorithm/eco DIR_SRCS_ECO)
aux_source_directory(./src/algorithm/rknn DIR_SRCS_RKNN)
aux_source_directory(./src/system DIR_SRCS_SYSTEM)
aux_source_directory(./src/rtsp_srv/bm DIR_SRCS_RTSP_SRV_BM)
aux_source_directory(./src/rtsp_srv/media DIR_SRCS_RTSP_SRV_MEDIA)
aux_source_directory(./src/rtsp_srv/rtp DIR_SRCS_RTSP_SRV_RTP)
aux_source_directory(./src/rtsp_srv/rtsp DIR_SRCS_RTSP_SRV_RTSP)
aux_source_directory(./src/rtmp_pusher/media DIR_SRCS_RTMP_MEDIA)
aux_source_directory(./src/rtmp_pusher/rtp DIR_SRCS_RTM_RTP)
aux_source_directory(./src/rtmp_pusher/rtmp DIR_SRCS_RTMP_RTMP)
aux_source_directory(./src/rtmp_pusher/librtmp DIR_SRCS_RTMP_LIB)
aux_source_directory(./src/record  DIR_SRCS_RECORD)

# Specify the build target
add_executable(${TARGETS}    ${DIR_SRCS}
							 ${DIR_SRCS_COMMON} 
							 ${DIR_SRCS_LOG}
							 ${DIR_SRCS_DRM} 
							 ${DIR_SRCS_GPU} 
							 ${DIR_SRCS_RGA} 
							 ${DIR_SRCS_MPP}
							 ${DIR_SRCS_RKISP}
							 ${DIR_SRCS_RTSP}
							 ${DIR_SRCS_DEVICE}
							 ${DIR_SRCS_DEVICE_UVC}
							 ${DIR_SRCS_OSD}
							 ${DIR_SRCS_PROTOCOL}
							 ${DIR_SRCS_ECO}
							 ${DIR_SRCS_RKNN}
							 ${DIR_SRCS_SYSTEM}
							 ${DIR_SRCS_RTSP_SRV_BM}	
							 ${DIR_SRCS_RTSP_SRV_MEDIA}					
							 ${DIR_SRCS_RTSP_SRV_RTP}	
							 ${DIR_SRCS_RTSP_SRV_RTSP}
							 ${DIR_SRCS_RTMP_MEDIA}	
							 ${DIR_SRCS_RTM_RTP}					
							 ${DIR_SRCS_RTMP_RTMP}	
							 ${DIR_SRCS_RTMP_LIB}
							 ${DIR_SRCS_RECORD}		
							)

target_link_libraries(${TARGETS}           
	    RtspCaster
		mp4v2
		rockchip_mpp
		GuideUSB2LiveStream
		usb-1.0
		rga
		drm  	
		pthread
		m
		rt			
		track_rk3566
		fftw3f
		cjson
		opencv_core	
		opencv_highgui
		opencv_imgcodecs	
		opencv_imgproc
		rknn_api
		gxiapi
		ucgui_rk3399
		avformat
		avcodec
		avutil
		swresample
		avdevice
		avfilter
		postproc
		swscale
		Tsmux
		exif
		z
		crypto
		ssl
		dl
)                            