#!/bin/bash

# Compile the ioctl driver
make > /dev/null 2>&1

# Load the ioctl driver module
sudo insmod ioctl_mod.ko

# Run the user space application
gcc user.c -o user
./user

# Remove the ioctl driver module
sudo rmmod ioctl_mod.ko

# Clean up compiled files
make clean > /dev/null

# Remove user space executable
rm user
