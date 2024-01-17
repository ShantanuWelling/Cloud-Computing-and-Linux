#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_GET_PHYSICAL_ADDRESS _IOR('q', 1, unsigned long) // Macro to create ioctl call to get physical address
#define IOCTL_WRITE_TO_PHYSICAL _IOW('q', 2, struct ioctl_data) // Macro to create ioctl call to write to physical address

struct ioctl_data { // Structure to store data for ioctl calls
    unsigned long virtual_address; // Virtual address of the memory
    unsigned long physical_address; // Physical address of the memory
    char value; // Value to be written to the memory
};

int main() {
    int file_desc; // File descriptor for the character device
    char *user_memory; // Pointer to the value
    struct ioctl_data user_data; // store user data for ioctl calls

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
    printf("Making ioctl call to get physical address..\n");

    // Make an ioctl call to get the physical address of the allocated memory
    user_data.physical_address = 0; // Initialize physical address to 0
    user_data.virtual_address = (unsigned long) user_memory; // Assign virtual address of the allocated memory
    ioctl(file_desc, IOCTL_GET_PHYSICAL_ADDRESS, &user_data); // Make ioctl call
    printf("Received Physical Address: 0x%lx corresponding to Virtual Address: %p\n" , user_data.physical_address, user_memory);

    // Make another ioctl call to change the value of the memory to "5" using a physical memory address
    user_data.value = 5; // Assign the value to be written
    printf("Making ioctl call to write value %d to physical address 0x%lx ..\n", user_data.value, user_data.physical_address);
    ioctl(file_desc, IOCTL_WRITE_TO_PHYSICAL, &user_data); // Make ioctl call
    
    // Verify the modified value by printing the content of the allocated memory
    printf("Modified Value: %d\n", *user_memory);

    // Clean up
    free(user_memory);
    close(file_desc);

    return 0;
}
