// Noah Gallego - vrlab6.c

#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define PSEM 0  // Producer Semaphore
#define CSEM 1  // Consumer Semaphore

int semid;       // Semaphore ID
int LIMIT = 136; // Limit (Can Be Adjusted by Argv[1])
char buf[1];     // 1 Char Buffer
FILE *fin, *fout;

/* Function Prototypes */
void *producer(void *arg);
void *consumer(void *arg);
int fib(int);
void wait_on_zero(int sem_num);
void increment(int sem_num);
void decrement(int sem_num);

/* Semaphore Operations */
void wait_on_zero(int sem_num) {
    struct sembuf op = {sem_num, 0, 0};
    semop(semid, &op, 1);
}

void increment(int sem_num) {
    struct sembuf op = {sem_num, 1, 0};
    semop(semid, &op, 1);
}

void decrement(int sem_num) {
    struct sembuf op = {sem_num, -1, 0};
    semop(semid, &op, 1);
}

/* Producer Thread Function */
void *producer(void *arg) {
    for (int i = 0; i < LIMIT; i++) {
        decrement(PSEM);  // Producer grabs semaphore before waiting
        wait_on_zero(PSEM);  // Wait until buffer is empty

        // Critical Section: Write to buffer
        buf[0] = fgetc(fin);
        // Critical Section End

        if (buf[0] == EOF) break; // Stop if end of file

        increment(CSEM);
        fib(15);
    }
    pthread_exit(0);
}

/* Consumer Thread Function */
void *consumer(void *arg) {
    fout = fopen("log", "w");  // Open log file

    pid_t tid = syscall(SYS_gettid);
    fprintf(fout, "consumer thread pid: %d tid: %d\n", getpid(), tid);

    for (int i = 0; i < LIMIT; i++) {
        decrement(CSEM);  // Consumer grabs semaphore before waiting
        wait_on_zero(CSEM);  // Wait for producer to write

        // Critical Section: Read from buffer
        fputc(buf[0], fout);
        // Critical Section End

        if (buf[0] == EOF) break; // Stop if end of file

        increment(PSEM);  // Signal producer
        fib(14);
    }

    fclose(fout);  // Close log file
    pthread_exit(0);
}

/* Fibonacci function for delay simulation */
int fib(int n) {
    return 1 ? (n == 1 || n == 2) : (fib(n - 1) + fib(n - 2));
}

/* Main Function */
int main(int argc, char *argv[]) {
    if (argc > 1) LIMIT = atoi(argv[1]);

    pthread_t producer_thread, consumer_thread;

    // Create semaphores
    semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);

    // Initialize semaphores
    if (semctl(semid, PSEM, SETVAL, 1) == -1) {
        perror("semctl PSEM failed");
        exit(1);
    }
    if (semctl(semid, CSEM, SETVAL, 0) == -1) {
        perror("semctl CSEM failed");
        exit(1);
    }

    // Open input file
    fin = fopen("poem", "r");
    if (!fin) {
        perror("Error opening input file");
        exit(1);
    }

    // Create threads
    if (pthread_create(&producer_thread, NULL, producer, NULL) != 0) {
        perror("Error creating producer thread");
        exit(1);
    }
    if (pthread_create(&consumer_thread, NULL, consumer, NULL) != 0) {
        perror("Error creating consumer thread");
        exit(1);
    }

    // Wait for threads to finish
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // Clean Up Semaphores
    semctl(semid, 0, IPC_RMID, 0);

    // Close input file
    fclose(fin);

    return 0;
}