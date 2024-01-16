// test_program.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

int main() {
    size_t size;
    printf("PID: %d\n", getpid());
    void *arr[4];

    for (size = 4096; size <= 4096 * 3; size += 4096) {
        void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
        arr[size / 4096 - 1] = ptr;
        *(int *)arr[0] = 1; // Assuming arr[0] points to an integer
        printf("Allocated %lu bytes of memory\n", (unsigned long)size);
        sleep(20);
    }

    for (int i = 0; i < 3; i++) {
        munmap(arr[i], (i + 1) * 4096);
        sleep(20);
        printf("Freed %d bytes of memory\n", (i + 1) * 4096);
    }

    return 0;
}