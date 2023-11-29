#! /bin/sh

LIBPH=./lib

if [ ! -e $LIBPH ]
then
	tar -xvf lib.tar
fi



#ifconfig enx000ec679cfc2 192.168.2.119 up
REAL_LIB=./lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$REAL_LIB/common:$REAL_LIB/opencv:$REAL_LIB/rtsp:$REAL_LIB/zmvision:$REAL_LIB/web:$REAL_LIB/gxi_api


sysctl -w kernel.core_pattern=/tmp/core
ulimit -c unlimited

chmod +x wyq_app
./wyq_app 





