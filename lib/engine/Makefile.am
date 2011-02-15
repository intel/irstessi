# Process this file with automake to create Makefile.in

AUTOMAKE_OPTIONS = nostdinc

noinst_LTLIBRARIES = libengine.la

libengine_la_SOURCES =          \
        ../../include/ssi.h     \
        ../efi/efi.h            \
        ../log/log.h            \
        ../mpb/mpb.h            \
        ../orom/orom.h          \
        ../sgio/sgio.h          \
        ahci.cpp                \
        ahci.h                  \
        ahci_cdrom.cpp          \
        ahci_cdrom.h            \
        ahci_disk.cpp           \
        ahci_disk.h             \
        ahci_multiplier.cpp     \
        ahci_multiplier.h       \
        ahci_phy.cpp            \
        ahci_phy.h              \
        ahci_tape.cpp           \
        ahci_tape.h             \
        array.cpp               \
        array.h                 \
        block_device.h          \
        block_device.cpp        \
        cache.cpp               \
        cache.h                 \
        container.cpp           \
        container.h             \
        context_manager.cpp     \
        context_manager.h       \
        controller.cpp          \
        controller.h            \
        enclosure.cpp           \
        enclosure.h             \
        end_device.cpp          \
        end_device.h            \
        event.cpp               \
        event.h                 \
        event_manager.cpp       \
        event_manager.h         \
        exception.h             \
        exception.cpp           \
        filesystem.cpp          \
        filesystem.h            \
        isci.cpp                \
        isci.h                  \
        isci_cdrom.cpp          \
        isci_cdrom.h            \
        isci_disk.cpp           \
        isci_disk.h             \
        isci_expander.cpp       \
        isci_expander.h         \
        isci_phy.cpp            \
        isci_phy.h              \
        isci_tape.cpp           \
        isci_tape.h             \
        list.h                  \
        multimedia_device.cpp   \
        multimedia_device.h     \
        object.cpp              \
        object.h                \
        phy.cpp                 \
        phy.h                   \
        port.cpp                \
        port.h                  \
        raid_device.cpp         \
        raid_device.h           \
        raid_info.cpp           \
        raid_info.h             \
        remote_port.cpp         \
        remote_port.h           \
        routing_device.cpp      \
        routing_device.h        \
        session.cpp             \
        session.h               \
        session_manager.cpp     \
        session_manager.h       \
        storage_device.cpp      \
        storage_device.h        \
        stream_device.cpp       \
        stream_device.h         \
        string.cpp              \
        string.h                \
        unique_id_manager.cpp   \
        unique_id_manager.h     \
        volume.cpp              \
        volume.h

libengine_la_CPPFLAGS =         \
        -I$(top_srcdir)         \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/lib
