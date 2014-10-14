# Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = nostdinc

lib_LTLIBRARIES = libssi.la

libssi_la_CPPFLAGS =                    \
        -I$(top_srcdir)                 \
        -I$(top_srcdir)/include         \
        -I$(top_srcdir)/lib

libssi_la_LDFLAGS =                     \
        $(SGUTILS_LDFLAGS)              \
        -version-info 1:0:0

libssi_la_SOURCES =                     \
        ../include/ssi.h                \
        ../lib/engine/array.h           \
        ../lib/engine/container.h       \
        ../lib/engine/context_manager.h \
        ../lib/engine/controller.h      \
        ../lib/engine/enclosure.h       \
        ../lib/engine/end_device.h      \
        ../lib/engine/object.h          \
        ../lib/engine/phy.h             \
        ../lib/engine/port.h            \
        ../lib/engine/raid_info.h       \
        ../lib/engine/routing_device.h  \
        ../lib/engine/session.h         \
        ../lib/engine/storage_device.h  \
        ../lib/engine/volume.h          \
        array.cpp                       \
        controller.cpp                  \
        enclosure.cpp                   \
        end_device.cpp                  \
        event.cpp                       \
        initialize.cpp                  \
        phy.cpp                         \
        port.cpp                        \
        raid_info.cpp                   \
        raid_level.cpp                  \
        routing_device.cpp              \
        session.cpp                     \
        system.cpp                      \
	templates.h			\
        volume.cpp

libssi_la_LIBADD =                      \
        ../lib/efi/libefi.la            \
        ../lib/log/liblog.la            \
        ../lib/orom/liborom.la          \
        ../lib/mpb/libmpb.la            \
        ../lib/engine/libengine.la
