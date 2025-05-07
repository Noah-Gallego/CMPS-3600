// Noah Gallego
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>

sem_t *sem[2]; // Array Of Two Semaphore Pointers

int main() {
    pid_t pid = fork();

    sem[0] = sem_open("/parentsemaphore1313131389", O_CREAT | O_EXCL, 0644, 1); // Parent Semaphore
    sem[1] = sem_open("/childsemaphore1313131387", O_CREAT | O_EXCL, 0644, 0); // Child Semaphore

    if (pid == 0) { // Child Process
        for (int i = 0; i < 5; i++) {
            sem_wait(sem[1]);
            printf("Child %i\n", i);
            sem_post(sem[0]);
        }
    } else { // Parent Process
        for (int i = 0; i < 5; i++) {
            // Critical Section Start
            sem_wait(sem[0]);
            printf("Parent %i\n", i);
            sem_post(sem[1]);
            // Critical Section End
        }

        sem_unlink("/parentsemaphore131313131389"); // Parent Semaphore
        sem_unlink("/childsemaphore131313131387"); // Child Semaphore
    }

    return 0;
}