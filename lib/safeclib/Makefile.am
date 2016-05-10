# Process this file with automake to create Makefile.in

noinst_LTLIBRARIES = libsafec.la

libsafec_la_SOURCES =		\
	safe_lib.h			\
	safeclib_private.h		\
	safe_lib_errno.h		\
	safe_str_constraint.h		\
	safe_str_lib.h			\
	safe_types.h			\
	safe_types.h.in			\
	snprintf_s.h			\
	safe_mem_constraint.h		\
	abort_handler_s.c		\
	ignore_handler_s.c		\
	memcmp_s.c			\
	memcpy_s.c			\
	memmove_s.c			\
	mem_primitives_lib.c		\
	mem_primitives_lib.h		\
	memset_s.c			\
	memzero_s.c			\
	safe_mem_constraint.c		\
	safe_mem_lib.h			\
	safe_str_constraint.c		\
	stpcpy_s.c			\
	stpncpy_s.c			\
	strcat_s.c			\
	strcmp_s.c			\
	strcpy_s.c			\
	strncat_s.c			\
	strncpy_s.c			\
	strnlen_s.c			\
	strstr_s.c			\
	snprintf_support.c		\
	strtok_s.c

libsafec_la_CPPFLAGS =		\
        -I$(top_srcdir)			\
        -I$(top_srcdir)/include		\
        -I$(top_srcdir)/lib		\
	-Wno-unused-variable
