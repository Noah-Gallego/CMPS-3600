// Noah Gallego

#include <signal.h>
#include <sys/file.h>  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

// Define Buffer Size
#define bufferSize 100

// Zero Out Buffer
void zeroBuffer(char *str) {
    for (int i = 0; i < bufferSize; i++) str[i] = 0;
} 

// Signal handler for SIGUSR1
void handler(int sigNum) {
    if (sigNum == SIGUSR1) {
        int logFile = open("log", O_WRONLY | O_APPEND);
        write(logFile, " (got the signal) ", 18);
        close(logFile);
    }
}

int main() {
    // Clear Buffer
    char buf[bufferSize];
    zeroBuffer(buf);

    // Define Mask
    sigset_t mask, oldmask;

    // Set Up Signal Handler for SIGUSR1 before fork
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // Block all signals before fork to prevent race conditions
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    // Fork One Child
    pid_t pid = fork();

    // Fork Handling
    if (pid == 0) { // Child Process
        printf("The Child is Running....\n");

        // Child Opens Log and Writes "GO CSUB"
        int logFile = open("log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        write(logFile, "Go CSUB", 7);
        close(logFile);

        // Allow SIGUSR1, block others
        sigdelset(&mask, SIGUSR1);

        // Child Goes into 'Deep Sleep'
        printf("The Child Is Now Waiting...\n");
        sigsuspend(&mask);

        // Write "Roadrunners!" after receiving SIGUSR1
        logFile = open("log", O_WRONLY | O_APPEND);
        write(logFile, "Roadrunners!", 12);
        close(logFile);

        exit(0);
    } else { // Parent Process
        // Restore original signal mask
        sigprocmask(SIG_SETMASK, &oldmask, NULL);

        // Introduce Delay to ensure child sets up
        sleep(2);

        // Send SIGTERM and then SIGUSR1 next
        kill(pid, SIGTERM);
        kill(pid, SIGUSR1);

        // Wait for child to terminate
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            printf("Child terminated with code %d\n", WEXITSTATUS(status));
        }
    }

    return 0;
}