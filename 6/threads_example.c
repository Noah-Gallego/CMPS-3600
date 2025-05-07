// Noah Gallego
// POSIX THREAD
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *mythread(void *arg) {
    int id = (int)(long)arg;

    for (int i = 0; i < 5; i++)
        printf("Hello! My Thread Number is: %i\n", id);

    return NULL;
}

int main() {    
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, mythread, (void *)1);
    pthread_create(&thread2, NULL, mythread, (void *)2);

    pthread_join(thread1, NULL); // Wait for the thread to join the main program
    pthread_join(thread2, NULL); // Wait for the thread to join the main program

    return 0;
}