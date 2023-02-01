#!/bin/bash


insmod  MyDeviceModule.ko
mknod --mode=666 /dev/mydevice0 c `grep MyDevice /proc/devices | awk '{print $1;}'` 0
mknod --mode=666 /dev/mydevice1 c `grep MyDevice /proc/devices | awk '{print $1;}'` 1
mknod --mode=666 /dev/mydevice2 c `grep MyDevice /proc/devices | awk '{print $1;}'` 2