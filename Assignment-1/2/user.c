#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_GET_PHYSICAL_ADDRESS _IOR('q', 1, unsigned long)
#define IOCTL_WRITE_TO_PHYSICAL _IOW('q', 2, struct ioctl_data)

struct ioctl_data {
    unsigned long virtual_address;
    unsigned long physical_address;
    char value;
};

int main() {
    int file_desc;
    char *user_memory;
    struct ioctl_data user_data;

    // Open the character device
    file_desc = open("/dev/ioctl_mod_dev", O_RDWR);
    if (file_desc < 0) {
        perror("Failed to open the device");
        return -1;
    }

    // Allocate byte-size memory on the heap
    user_memory = malloc(sizeof(char));
    if (!user_memory) {
        perror("Failed to allocate user memory");
        close(file_desc);
        return -1;
    }

    // Assign the value "6" to the memory
    *user_memory = 6;

    // Print the virtual address and value of the allocated memory
    printf("Virtual Address: %p, Value: %d\n", user_memory, *user_memory);

    // Make an ioctl call to get the physical address of the allocated memory
    user_data.physical_address = 0;
    user_data.virtual_address = (unsigned long) user_memory;
    ioctl(file_desc, IOCTL_GET_PHYSICAL_ADDRESS, &user_data);
    printf("Physical Address: %lx\n", user_data.physical_address);

    // Make another ioctl call to change the value of the memory to "5" using a physical memory address
    user_data.value = 5;
    ioctl(file_desc, IOCTL_WRITE_TO_PHYSICAL, &user_data);

    // Verify the modified value by printing the content of the allocated memory
    printf("Modified Value: %d\n", *user_memory);

    // Clean up
    free(user_memory);
    close(file_desc);

    return 0;
}
