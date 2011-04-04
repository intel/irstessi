#!/bin/bash

# Stop all MD devices first.
( set -e; . ./stop.sh )

# The function detecting devices attached to the given adapter.
. adapter.sh

# Erase metadata from disks atached to AHCI adapter.
ahci_devices=`get_attached_devices ahci`
if [ "$ahci_devices" != "" ]; then
    mdadm --zero-superblock -q $ahci_devices
fi

# Erase metadata from disks attached to ISCI adapter.
isci_devices=`get_attached_devices isci`
if [ "$isci_devices" != "" ]; then
    mdadm --zero-superblock -q $isci_devices
fi

exit 0
