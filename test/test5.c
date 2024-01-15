#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGE_SIZE 4096

uint64_t get_pagemap_entry(int pid, uintptr_t vaddr) {
    off_t offset = (vaddr / PAGE_SIZE) * sizeof(uint64_t);
    char path[256];
    int fd;

    sprintf(path, "/proc/%d/pagemap", pid);

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening pagemap");
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("Error seeking in pagemap");
        close(fd);
        exit(EXIT_FAILURE);
    }

    uint64_t pagemap_entry;
    if (read(fd, &pagemap_entry, sizeof(uint64_t)) != sizeof(uint64_t)) {
        perror("Error reading pagemap entry");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    return pagemap_entry;
}

uintptr_t virtual_to_physical_mapping(int pid, uintptr_t vaddr) {
    uint64_t pagemap_entry = get_pagemap_entry(pid, vaddr);
    uint64_t page_frame_number = pagemap_entry & 0x7FFFFFFFFFFFFF;
    uintptr_t physical_addr = (page_frame_number * PAGE_SIZE) | (vaddr % PAGE_SIZE);

    return physical_addr;
}

int main() {
    int pid;
    uintptr_t vaddr;

    printf("Enter the process ID: ");
    scanf("%d", &pid);

    printf("Enter the virtual address in hexadecimal (e.g., 0x...): ");
    scanf("%lx", &vaddr);

    uintptr_t physical_addr = virtual_to_physical_mapping(pid, vaddr);

    printf("Virtual Address: 0x%lx\n", vaddr);
    printf("Physical Address: 0x%lx\n", physical_addr);

    return 0;
}
