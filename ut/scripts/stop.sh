#!/bin/bash

# Stop all detected md devices (make three attempts)
for attempt in `echo 0 1 2`; do
    the_list=`cat /proc/mdstat | grep 'md' | awk '{print $1}' | grep 'md'`
    if [ "$the_list" == "" ]; then
        break;
    fi
    the_md_list=""
    for device in `echo $the_list`; do
        the_md_list="$the_md_list /dev/$device"
    done
    ( set -e; udevadm settle; mdadm -Sq $the_md_list )
done

exit 0
