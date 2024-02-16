#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main() {
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed.\n");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child process is running.\n");
    } else {
        // Parent process
        printf("Parent process is running with pid: %d, and the child process ID is %d.\n",SYS_getpid, pid);
    }

    return 0;
}

