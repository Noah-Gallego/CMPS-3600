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

// Setup Grab / Release Arrays
struct sembuf grab[2][2], release[2][1];

/* Main Function */
int main(int argc, char *argv[]) {
    if (argc > 1) LIMIT = atoi(argv[1]);

    pthread_t producer_thread, consumer_thread;

    // Create semaphores
    semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);

    // Initialize semaphores
    semun.val = 0;
    semctl(semid, PSEM, SETVAL, semun);
    semun.val = 1;
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
        semop(semid, grab[0], 2);

        // Critical Section: Write to buffer
        buf[0] = fgetc(fin);
        // Critical Section End

        if (buf[0] == EOF) break;

        semop(semid, &release[0][0], 1); 
        fib(15);
    }
    pthread_exit(0);
}

/* Consumer Thread Function */
void *consumer(void *arg) {
    fout = fopen("log", "w");  // Open log file

    for (int i = 0; i < LIMIT; i++) {
        semop(semid, grab[1], 2); 

        // Critical Section: Read from buffer
        fputc(buf[0], fout);
        // Critical Section End

        if (buf[0] == EOF) break; 

        semop(semid, &release[1][0], 1); 
        fib(14);
    }

    fputc('\n', fout);

    fclose(fout);  // Close log file
    pthread_exit(0);
}

/* Fibonacci function for delay simulation */
int fib(int n) {
    return (n == 1 || n == 2) ? 1 : (fib(n - 1) + fib(n - 2));
}

/* Define Grab / Release Behavior / Wait For Zero */
void setup_grab_release_arrays() {
    // Producer Semaphore: Wait FOR Zero 
    grab[0][0].sem_num = PSEM;
    grab[0][0].sem_op = 0; 
    grab[0][0].sem_flg = SEM_UNDO;

    // Producer Semaphore: Increment
    grab[0][1].sem_num = PSEM;
    grab[0][1].sem_op = 1;
    grab[0][1].sem_flg = SEM_UNDO;
    
    // Consumer Semaphore: Decrement
    release[0][0].sem_num = CSEM;
    release[0][0].sem_op = -1;
    release[0][0].sem_flg = SEM_UNDO;

    // Consumer Semaphore: Wait FOR Zero
    grab[1][0].sem_num = CSEM;
    grab[1][0].sem_op = 0; 
    grab[1][0].sem_flg = SEM_UNDO;

    // Consumer Semaphore: Increment
    grab[1][1].sem_num = CSEM;
    grab[1][1].sem_op = 1;
    grab[1][1].sem_flg = SEM_UNDO;
    
    // Producer Semaphore: Decrement
    release[1][0].sem_num = PSEM;
    release[1][0].sem_op = -1;
    release[1][0].sem_flg = SEM_UNDO;
}