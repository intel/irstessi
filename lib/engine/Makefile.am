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
        ahci_multiplier_phy.cpp \
        ahci_multiplier_phy.h   \
        ahci_multiplier_port.cpp\
        ahci_multiplier_port.h  \
        ahci_phy.cpp            \
        ahci_phy.h              \
        ahci_port.cpp           \
        ahci_port.h             \
        ahci_raid_info.cpp      \
        ahci_raid_info.h        \
        ahci_tape.cpp           \
        ahci_tape.h             \
        array.cpp               \
        array.h                 \
        block_device.cpp        \
        block_device.h          \
        cache.cpp               \
        cache.h                 \
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
        exception.cpp           \
        exception.h             \
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
        isci_expander_phy.cpp   \
        isci_expander_phy.h     \
        isci_expander_port.cpp  \
        isci_expander_port.h    \
        isci_phy.cpp            \
        isci_phy.h              \
        isci_port.cpp           \
        isci_port.h             \
        isci_raid_info.cpp      \
        isci_raid_info.h        \
        isci_tape.cpp           \
        isci_tape.h             \
        list.h                  \
        multimedia_device.h     \
        nondisk_device.h        \
        object.cpp              \
        object.h                \
        pci_header.h            \
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
        stream_device.h         \
        string.cpp              \
        string.h                \
        unique_id_manager.cpp   \
        unique_id_manager.h     \
        utils.cpp               \
        utils.h                 \
        volume.cpp              \
        volume.h

libengine_la_CPPFLAGS =         \
        -I$(top_srcdir)         \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/lib
