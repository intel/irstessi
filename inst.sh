#!/bin/bash -ex
cd `pwd`/`dirname $0`
./autogen.sh --libdir='${prefix}/lib64'
CXXFLAGS=-g3 ./configure --libdir='${prefix}/lib64'
make
make install
cd ut
./ut_session
