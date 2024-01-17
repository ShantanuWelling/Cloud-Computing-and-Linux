#!/bin/bash

# Compile the ioctl driver
make

# Load the ioctl driver module
sudo insmod ioctl_mod.ko

# Run the user space application
gcc user.c -o user
./user

# Remove the ioctl driver module
sudo rmmod ioctl_example

# Clean up compiled files
make clean

# Remove user space executable
rm user
