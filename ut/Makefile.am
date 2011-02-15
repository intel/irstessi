# Process this file with automake to create Makefile.in

noinst_PROGRAMS = ut_scope_controller_direct ut_volume

ACLOCAL_AMFLAGS = -I m4

ut_scope_controller_direct_SOURCES =    \
        ut_scope_controller_direct.c

ut_scope_controller_direct_CFLAGS =     \
        -I$(top_srcdir)                 \
        -I$(top_srcdir)/include

ut_scope_controller_direct_LDADD =     \
        ../src/libssi.la

ut_volume_SOURCES = \
	ut_volume.c

ut_volume_CFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include

ut_volume_LDADD = \
	../src/libssi.la
