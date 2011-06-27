#!/bin/bash
<<LICENSE_BSD

Copyright (c) 2011, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

LICENSE_BSD

driver_name=$1
raid_level=$2
chunk_size=$3
component_size=$4
devices_num=$5
raid_level_2=$6
chunk_size_2=$7
component_size_2=$8

# Import the function detecting devices attached to the given adapter.
. adapter.sh

# Get all devices attached to the specified adapter.
the_devices=`get_attached_devices $driver_name`

# Report an error if there are no devices attached to the give adapter.
if [ "$the_devices" == "" ]; then
    exit 1
fi

# Determine which disk or MD device holds the root file system.
the_system_device=`df -v | grep /$ | awk '{print $1}'`

# Filter out non-disk and non-storage devices attached to the adaper.
for device in `echo $the_devices`; do
    case $the_system_device in
    $device*) ;;
    *)  the_driver="/sys/block/`basename $device`/device/driver"
        if [ -h "$the_driver" ]; then
            the_driver=`( cd -P "$the_driver"; pwd )`
            if [ "$the_driver" == "/sys/bus/scsi/drivers/sd" ]; then
                the_available_devices="$the_available_devices $device"
            fi
        fi
    ;;
    esac
done

# Stop all MD devices before continue.
( set -e; . clean.sh )

# Erase metadata information from block devices attached to the given adapter.
mdadm --zero-superblock -q $the_available_devices

# Determine the number of disks attached to the given adapter.
disks_num=`echo "$the_available_devices" | wc -w`

# Report an error if there are less block devices available then the given volume requires.
if [ $disks_num -lt $devices_num ]; then
    exit 2
fi

the_list=""
for device in `seq $devices_num`; do
    the_list="$the_list `echo $the_available_devices | awk '{print $'$device'device}'`"
done

udevadm settle

for num in `seq -f %04.0f 0 9999`; do
    the_array=Imsm_$num
    if [ ! -h "/dev/md/$the_array" ]; then
        break;
    fi
done

for num in `seq -f %04.0f 0 9999`; do
    the_volume=Volume_$raid_level\_$num
    if [ ! -h "/dev/md/$the_volume" ]; then
        break
    fi
done

# Create and start the container (array)
mdadm -CR $the_array -amd -eimsm -n$devices_num $the_list
# Create and start the volume (subarray)
mdadm -CR $the_volume -amd -l$raid_level --chunk=$chunk_size --size=$component_size -n$devices_num $the_list

# Create and start the second volume (subarray) in the same container (array)
if [ "$chunk_size_2" != "" ]; then
    for num in `seq -f %04.0f 0 9999`; do
        the_volume_2=Volume_$raid_level_2\_$num
        if [ ! -h "/dev/md/$the_volume_2" ]; then
            break
        fi
    done
    mdadm -CR $the_volume_2 -amd -l$raid_level_2 --chunk=$chunk_size_2 --size=$component_size_2 -n$devices_num $the_list
fi

exit $?
