// Noah Gallego - vrlab6.c

#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PSEM 0  // Producer Semaphore
#define CSEM 1  // Consumer Semaphore

int semid;       // Semaphore ID
int LIMIT = 136; // Limit (Can Be Adjusted by Argv[1])
char buf[1];     // 1 Char Buffer
FILE *fin, *fout;

union {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
} semun;

/* Function Prototypes */
void *producer(void *arg);
void *consumer(void *arg);
int fib(int);
void setup_grab_release_arrays();

// Setup Grab / Release Arrays & wait_FOR_zero
struct sembuf grab[2], release[2], wait_for_zero;

/* Main Function */
int main(int argc, char *argv[]) {
    if (argc > 1) LIMIT = atoi(argv[1]);

    pthread_t producer_thread, consumer_thread;

    // Create semaphores
    semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);

    // Initialize semaphores
    semun.val = 1;
    semctl(semid, PSEM, SETVAL, semun);
    semun.val = 0;
    semctl(semid, CSEM, SETVAL, semun);

    // Setup Semaphores (Grab/Release Arrays)
    setup_grab_release_arrays();

    // Open input file
    fin = fopen("poem", "r");

    // Create threads
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Wait for threads to finish
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // Clean Up Semaphores
    semctl(semid, 0, IPC_RMID, 0);

    // Close input file
    fclose(fin);

    return 0;
}

/* Producer Thread Function */
void *producer(void *arg) {
    for (int i = 0; i < LIMIT; i++) {
        semop(semid, &grab[0], 1); // Decrement PSEM

        // Critical Section: Write to buffer
        buf[0] = fgetc(fin);
        // Critical Section End

        if (buf[0] == EOF) break; // Stop if end of file

        semop(semid, &release[0], 1); // Increment CSEM
        fib(15);
    }
    pthread_exit(0);
}

/* Consumer Thread Function */
void *consumer(void *arg) {
    fout = fopen("log", "w");  // Open log file

    for (int i = 0; i < LIMIT; i++) {
        semop(semid, &wait_for_zero, 1); // Wait For Zero (PSEM)
        semop(semid, &grab[1], 1); // Decrement CSEM 

        // Critical Section: Read from buffer
        fputc(buf[0], fout);
        // Critical Section End

        if (buf[0] == EOF) break; 

        semop(semid, &release[1], 1); // Increment PSEM
        fib(14);
    }

    fclose(fout);  // Close log file
    pthread_exit(0);
}

/* Fibonacci function for delay simulation */
int fib(int n) {
    return (n == 1 || n == 2) ? 1 : (fib(n - 1) + fib(n - 2));
}

/* Define Grab / Release Behavior / Wait For Zero */
void setup_grab_release_arrays() {
    // Producer grab: Decrement PSEM
    grab[0].sem_num = PSEM;
    grab[0].sem_op = -1;   // Decrement PSEM
    grab[0].sem_flg = SEM_UNDO;

    // Producer release: Increment CSEM
    release[0].sem_num = CSEM;
    release[0].sem_op = 1;
    release[0].sem_flg = SEM_UNDO;

    // Consumer grab: Decrement CSEM
    grab[1].sem_num = CSEM;
    grab[1].sem_op = -1;   // Decrement CSEM
    grab[1].sem_flg = SEM_UNDO;

    // Consumer release: Increment PSEM
    release[1].sem_num = PSEM;
    release[1].sem_op = 1;
    release[1].sem_flg = SEM_UNDO;

    // Wait-FOR-Zero Operation (Consumer waits for PSEM to be zero before starting)
    wait_for_zero.sem_num = PSEM;
    wait_for_zero.sem_op = 0; // Wait for PSEM to be Zero
    wait_for_zero.sem_flg = SEM_UNDO;
}