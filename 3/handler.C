#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <cstdlib>

void handler(int sig) {
    printf("\nControl + C Was Pressed!");
    printf("\nTerminating Myself.\n\n");
    exit(0);
}

int main() {
    signal(SIGINT, handler);
    pid_t pid = fork();

    if (pid == 0) {
        // Child Process
        while(1) {
            printf("Hello ");
            fflush(stdout);
            usleep(500000);
        }   
    } else {
        // Parent Process
        kill(pid, SIGINT);
    }

    return 0;
}