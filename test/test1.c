#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    const int numChildren = 5;
    printf("Parent process (PID: %d)\n", getpid());

    for (int i = 0; i < numChildren; ++i) {
        pid_t childPid = fork();

        if (childPid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (childPid == 0) {
            // Child process
            printf("Child process %d (PID: %d) spawned\n", i + 1, getpid());
            sleep(15);
            printf("Child process %d (PID: %d) exiting.\n", i + 1, getpid());
            exit(EXIT_SUCCESS);
        }
    }

    // Parent process
    printf("Parent process (PID: %d) waiting\n", getpid());

    for (int i = 0; i < numChildren; ++i) {
        int status;
        pid_t terminatedChild = wait(&status);

        if (terminatedChild == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            printf("Child process %d (PID: %d) exited\n", i + 1, terminatedChild);
        } else {
            printf("Child process %d (PID: %d) exited abnormally.\n", i + 1, terminatedChild);
        }
    }

    printf("Parent process (PID: %d) all children have exited.\n", getpid());

    return 0;
}
