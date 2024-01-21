#!/bin/bash

## Building the applications
make 

## Build your ioctl driver and load it here

sudo insmod ioctl_pid.ko

###############################################

# Launching the control station
./control_station &
c_pid=$!
echo "Control station PID: $c_pid"

# Launching the soldier
sudo ./soldier $c_pid &
echo "Soldier PID: $!"

sleep 2
kill -9 $c_pid

## Remove the driver here
sudo rmmod ioctl_pid.ko


