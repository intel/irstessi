#!/bin/bash -ex
origdir=`pwd`
cd /root/SSI_API
./autogen.sh --libdir='${prefix}/lib64'
CXXFLAGS=-g3 ./configure --libdir='${prefix}/lib64'
make
make install
#g++ -o ania ut_ania.cpp -g3 -L/usr/local/lib64 -lssi 
#echo aaa > /tmp/my.log
#./ania
#cat /tmp/my.log
cd ut
./ut_session
cd $origdir
