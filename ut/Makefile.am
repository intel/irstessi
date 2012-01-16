# Process this file with automake to create Makefile.in

noinst_PROGRAMS = ut_volume ut_filesystem ut_session ut_string ut_info ut_markasspare ut_migration ut_events ut_initialize_volume ut_phy

ACLOCAL_AMFLAGS = -I m4 nostdinc

ut_events_SOURCES = \
	ut_events.cpp

ut_events_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_events_LDADD = \
	../src/libssi.la

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

ut_migration_SOURCES = \
	ut_migration.cpp

ut_migration_CPPFLAGS = \
	-g3 -I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_migration_LDADD = \
	../src/libssi.la

ut_phy_SOURCES = \
	ut_phy.cpp

ut_phy_CPPFLAGS = \
	-g3 -I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_phy_LDADD = \
	../src/libssi.la

ut_initialize_volume_SOURCES = \
	ut_initialize_volume.cpp

ut_initialize_volume_CPPFLAGS = \
	-g3 -I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_initialize_volume_LDADD = \
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

