#!/bin/csh
ip link set $1 up
sleep 0.5s
brctl addif bp_arunp $1
exit 0
