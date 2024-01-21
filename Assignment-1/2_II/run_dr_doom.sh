#!/bin/bash

## Building the applications
make > /dev/null 2>&1

## Build your ioctl driver and load it here

insmod ioctl_pid.ko

###############################################

# Launching the control station
./control_station &
c_pid=$!
echo "Control station PID: $c_pid"

# Launching the soldier
./soldier $c_pid &
echo "Soldier PID: $!"

sleep 2
kill -9 $c_pid

## Remove the driver here
rmmod ioctl_pid.ko
make clean > /dev/null 2>&1