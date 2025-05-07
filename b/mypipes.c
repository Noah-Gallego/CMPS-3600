#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    char str[100];

    // File Descriptors
    int fd[2];

    // Open A Pipe
    pipe(fd);

    // Start Fork
    pid_t pid = fork();

    if (pid == 0) { // Child Process
        close(fd[1]);
        printf("Child Is Reading The Pipe: %s\n", str);
        fflush(stdout);
        printf("String: **%s**\n", str);
        printf("Child Is Reading The Pipe AGAIN: %s\n", str);
        fflush(stdout);
        read(fd[0], str, 5);

        // Write to FD
        exit(1);
    } else { // Parent Process
        // Read From FD
        close(fd[0]);   
        sleep(10);
        write(fd[1], "Hello", 5);
        sleep(5);
        close(fd[1]);
    }

    return 0;
}