// Noah Gallego 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/file.h>  
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

// Size of Buffer
#define bufferSize 100

// Fibonacci Logic
int fib(int n) {
    // Base Case
    if (n <= 1) return n;

    // Fib Sequence (Recursively)
    return fib(n - 1) + fib(n - 2);
}

// Zero Out Buffer
void zeroBuffer(char *str) {
    for (int i = 0; i < bufferSize; i++) str[i] = 0;
} 

int main(int argc, char *argv[]) {
    char buf[bufferSize];
    zeroBuffer(buf);

    // Error Handling: If no cl argument exists, return error
    if (argc < 2) {
        printf("Usage: %s <n>\n       examples: %s 9 (for 9th fibonacci number)\n", argv[0], argv[0]);
        exit(1);
    }

    // Convert Argument to Int
    int input = atoi(argv[1]);

    // Fork a child process
    pid_t pid = fork();

    // Error / Child / Parent Process ID Handling
    if (pid < 0) { // Error
        perror("fork");
        exit(1);
    } else if (pid == 0) { // Child Process
        printf("THE CHILD IS RUNNING...\n");
        
        // Open Log File
        int logFile;

        // Filename, Flags, Permissions (Read, Write) 
        logFile = open("log", O_WRONLY|O_CREAT|O_TRUNC, 0644);  
        if (logFile == -1) perror("File Open Error: "); 
        
        // Write To Child Log File TODO
        time_t T;
        time(&T);

        // Write Date To Log File
        sprintf(buf, "Time: %s\n", ctime(&T));
        write(logFile, buf, bufferSize);

        // Call Fib Sequence & Print to Terminal
        int fib_n = fib(input);
        printf("The nth fib sequence is: %d\n", fib_n);

        // Write Fib To Log File
        sprintf(buf, "The nth fib sequence is: %d\n", fib_n);
        write(logFile, buf, bufferSize);

        close(logFile);

        // Exit with Input as Code
        exit(input);
    } else if (pid > 0) { // Parent Process
        printf("THE PARENT IS RUNNING...\n");
        int status;
        wait(&status); // Wait for Process to Finish
        
        // If Child Process Ended Normally
        if (WIFEXITED(status)) {
            printf("My child exited with code: %d\n", WEXITSTATUS(status));
        }   
        // Write Info To File
        write(1, buf, bufferSize);

        exit(0);
    }

    return 0;
}
