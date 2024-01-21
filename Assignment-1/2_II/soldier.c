#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

// Include header or define the IOCTL call interface and devide name

#define IOCTL_SET_PARENT_PID _IOW('q', 1, pid_t) // Macro to get parent PID

//**************************************************

int open_driver(const char* driver_name) {

    int fd_driver = open(driver_name, O_RDWR);
    if (fd_driver == -1) {
        perror("ERROR: could not open driver");
    }

	return fd_driver;
}

void close_driver(const char* driver_name, int fd_driver) {

    int result = close(fd_driver);
    if (result == -1) {
        perror("ERROR: could not close driver");
    }
}


int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <parent_pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t parent_pid = atoi(argv[1]);

    // open ioctl driver
    int file_desc; // File descriptor for the character device
    // Open the character device
    file_desc = open_driver("/dev/ioctl_pid_dev");
    if (file_desc < 0) {
        perror("Failed to open the device");
        return -1;
    }
    
    // call ioctl with parent pid as argument to change the parent
    ioctl(file_desc, IOCTL_SET_PARENT_PID, &parent_pid); // Make ioctl call
	
    // close ioctl driver
    close_driver("/dev/ioctl_pid_dev", file_desc);
    sleep(1);
	return EXIT_SUCCESS;
}