// Noah Gallego

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h> // Posix Semaphore
#include <pthread.h>

int done = 0;
int eat[5] = {0, 0, 0, 0, 0};
int max_eats = 10000;
sem_t forks[5];
pthread_mutex_t monitor;

void *philosopher(void *arg) {
    int id = (int)(long)arg;
    int forkno[2] = {id, (id + 1) % 5}; // Fork at Current Pos & Pos + 1 (left + right)

    // Prevent Starvation/Deadlock via Hierachy Solution - BREAKS CODE???
    // if (forkno[0] > forkno[1]) {
    //     // Swap Forks
    //     int temp = forkno[0];
    //     forkno[1] = forkno[0];
    //     forkno[0] = temp;
    // }

    while (!done) {
        // Think 🧠

        // Critical Seciton Start (Eat) 🍽️
        pthread_mutex_lock(&monitor);
        sem_wait(&forks[forkno[0]]); // Grab
        sem_wait(&forks[forkno[1]]); // Grab
        pthread_mutex_unlock(&monitor);

        eat[id]++;

        printf("%i %i %i %i %i \n", eat[0], eat[1], eat[2], eat[3], eat[4]);

        sem_post(&forks[forkno[0]]); // Release
        sem_post(&forks[forkno[1]]); // Release
        // Critical Section End (Eat) 🍽️

        // Rest 😴
        if (eat[id] > max_eats) done = 1;
    }

    return (void *)0;
}

int main() {
    pthread_t threadID[5];
    void *status[5];
    pthread_mutex_init(&monitor, NULL); // Unlocked

    for (int i = 0; i < 5; i++) {
        // Initialize Semaphores
        sem_init(&forks[i], 0, 1);
    }

    for (int i = 0; i < 5; i++) {
        pthread_create(&threadID[i], NULL, philosopher, (void *)(long)i);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(threadID[i], &status[i]);
    }

    return 0;
}