#!/bin/bash

# The function detecting devices attached to the given adapter.
. adapter.sh

# Get the list of devices attached to the given storage adapter.
adap_devices=`get_attached_devices $1`

# Return the number of attached devices as the exit code of the script.
# 0 means no devices attached to the adapter or no such an adapter.
exit `echo $adap_devices | wc -w`
