#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// Buffer Size / Prototypes
#define BUFSIZE 256
int status;
int fib(int);

// Define Semaphore Variables
union {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
} my_semun;

struct sembuf grab[2], release[1];
char pathname[200];
key_t ipckey;
int semid;

int main(int argc, char **argv) {
    int n;
    char buf[BUFSIZE];
    pid_t cpid;
    int shmid; 
    int *shared;

    // Check if n was given on command line else 25
    n = argc >= 2 ? atoi(argv[1]) : 25;

    shmid = shmget(IPC_PRIVATE, sizeof(int)*100, IPC_CREAT | 0666);
    shared = shmat(shmid, NULL, 0); // Attach and initialize memory segment
    *shared = 0;

    int nsem = 1;
	semid = semget(ipckey, nsem, 0666 | IPC_CREAT);

    // Semaphore Setup
    grab[0].sem_num = grab[1].sem_num = 0; // Which Semaphore
    grab[0].sem_flg = grab[1].sem_flg = SEM_UNDO; // Set Flags (Release Upon Death)

    // Sem Operations
    grab[0].sem_op = 0;
	grab[1].sem_op = 1;

    // Initialize Release
    release[0].sem_num = 0;
	release[0].sem_flg = SEM_UNDO;
	release[0].sem_op = -1; 

    // Set Semun Value
    my_semun.val = 0;
    semctl(semid, 0, SETVAL, my_semun); // Retrievable via: semctl(semid, 0, GETVAL);

    // Set Working Directly
	getcwd(pathname,200);
	strcat(pathname,"/foo");
	ipckey = ftok(pathname, 42);

    // BEFORE FORK: Parent Grabs Semaphore
    semop(semid, grab, 2);

    // Start Fork
    cpid = fork();

    if (cpid < 0) {
        printf("Error - fork command\n");
        fflush(stdout);
        exit(0);
    }

    // Open Log File (Before Fork, So Both Processes Use It)
    int logFile = open("log", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (cpid == 0) { // Child Process
        // Critical Section Start
        semop(semid, grab, 2);
        shared = shmat(shmid, NULL, 0);
        // Critical Section End

        // Child reads from memory and Displays
        int val = *shared;
        sprintf(buf, "Child reads: %d\n", val);
        write(1, buf, strlen(buf));
        dprintf(logFile, "%s\n", buf);

        close(logFile);
        shmdt(shared); // Detach from segment
        exit(0);
    } else { // Parent Process
        // Parent computes fib(n) and writes it to shared memory
        // Critical Section Start
        *shared = fib(n);
        semop(semid, release, 1);
        // Critical Section End

        wait(&status); 

        shmdt(shared);              // Detach from segment
        shmctl(shmid, IPC_RMID, 0); // Remove shared segment
    }
    return 0;
}

// Busy Work For Parent - Fibonacci Function
int fib(int n) {
    return (n == 1 | n == 2) ? n : fib(n - 1) + fib(n - 2);
}


