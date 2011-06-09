# Process this file with automake to create Makefile.in

noinst_PROGRAMS = ut_volume ut_filesystem ut_session ut_string ut_info ut_markasspare

ACLOCAL_AMFLAGS = -I m4 nostdinc

ut_volume_SOURCES = \
	ut_volume.cpp

ut_volume_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_volume_LDADD = \
	../src/libssi.la

ut_session_SOURCES = \
	ut_session.cpp

ut_session_CPPFLAGS = \
	-g3 -I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_session_LDADD = \
	../src/libssi.la

ut_info_SOURCES = \
	ut_info.cpp

ut_info_CPPFLAGS = \
	-g3 -I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_info_LDADD = \
	../src/libssi.la

ut_markasspare_SOURCES = \
	ut_markasspare.cpp

ut_markasspare_CPPFLAGS = \
	-g3 -I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_markasspare_LDADD = \
	../src/libssi.la

ut_filesystem_SOURCES = \
	ut_filesystem.cpp \
	../lib/engine/string.cpp \
	../lib/engine/filesystem.cpp \
	../lib/engine/exception.cpp

ut_filesystem_CPPFLAGS = \
	-iquote $(top_srcdir)/lib/engine \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_string_SOURCES = \
	ut_string.cpp \
	../lib/engine/exception.cpp \
	../lib/engine/string.cpp


ut_string_CPPFLAGS = \
	-iquote $(top_srcdir)/lib/engine \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include

