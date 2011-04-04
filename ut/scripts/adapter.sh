#!/bin/bash

# The function returns the list of block devices attached to the given adapter.
# $1 the name of the adapter: ahci or isci
get_attached_devices() {
    local the_result=""
    local the_path="/sys/bus/pci/drivers/$1"
    [ -d $the_path ] && for th in `ls $the_path`; do
        if [ -h "$the_path/$th/driver" ]; then
            local the_real=( `cd -P "$the_path/$th"; pwd` )
            local the_back=( `cd -P "$the_path/$th/driver"; pwd` )
            if [ "$the_path" == "$the_back" ]; then
                for dev in `ls /sys/block`; do
                    local the_device=( `cd -P "/sys/block/$dev"; pwd` )
                    case "$the_device" in
                    "$the_real"*) the_result="/dev/$dev $the_result";;
                    esac
                done
            fi
        fi
    done
    echo "$the_result"
}
