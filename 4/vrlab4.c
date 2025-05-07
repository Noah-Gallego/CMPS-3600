// Noah Gallego
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

// Define Message Characteristics
struct mymsg {
    long type;
    char text[100];
};

// Shared Mem Seg --> Shared Mem Pointer --> Message Queue -->

int main() {
    // Initialize Message
    struct mymsg message;

    // Before Fork - Generate IPC Key
    key_t key = ftok("foo", 21);

    // Generate a Shared Memory Segment to Hold One Int
    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);

    // Create Shared Memory Pointer
    int *shared = (int *)shmat(shmid, NULL, 0);
    *shared = 0; // Initialize to 0

    // Create Message Queue
    int msqid = msgget(key, IPC_CREAT | 0666);

    // Open Log File (Before Fork, So Both Processes Use It)
    int logFile = open("log", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // Fork
    pid_t pid = fork();

    if (pid == 0) { // Child Process
        // Busy-wait for shared memory change
        while (*shared == 0) {
            usleep(1000);
        }

        // Read message from queue
        msgrcv(msqid, &message, sizeof(message.text), 1, 0);

        // Write shared memory value to log
        dprintf(logFile, "%d\n", *shared);

        // Write message queue content to log
        dprintf(logFile, "%s\n", message.text);

        // Close log file & clean up
        close(logFile);
        shmdt(shared);
        exit(0);
    } else if (pid > 0) { // Parent Process
        // Define Input Size as Char Array
        char buf[10];

        // Collect User Input For Shared Memory
        write(1, "Enter a two-digit number: ", 26);
        read(0, buf, 4);
        *shared = atoi(buf);

        // Get User Input For Message Queue
        write(1, "Enter a word: ", 14);
        read(0, message.text, sizeof(message.text) - 1);
        message.text[strcspn(message.text, "\n")] = 0;  // Remove newline
        message.type = 1;
        msgsnd(msqid, &message, sizeof(message.text), 0);

        // Wait for Child
        wait(NULL);

        printf("Child exited with status code 0\n");

        // Clean Up Shared IPC Objects
        msgctl(msqid, IPC_RMID, NULL);
        shmctl(shmid, IPC_RMID, NULL);
        shmdt(shared);
        close(logFile); // Close log file in parent
    }

    return 0;
}