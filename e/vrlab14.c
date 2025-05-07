// Noah Gallego

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>

int fd[2];

int main(int argc, char *argv[], char *envp[]) {
    char *program_name = argv[0];

    if (argc == 1 || (argc >= 2 && (strcmp(argv[1], "PRODUCER") && strcmp(argv[1], "CONSUMER")))) {
        if (pipe(fd) < 0) return EXIT_FAILURE;
        int seed = 1000;
        if (argc >= 2) seed = atoi(argv[1]);

        // Convert Seed To Integers
        char seed_str[10], fd_0_str[10], fd_1_str[10];
        sprintf(seed_str, "%d", seed);
        sprintf(fd_0_str, "%d", fd[0]);
        sprintf(fd_1_str, "%d", fd[1]);

        // Fork Producer Process
        pid_t producer = fork();
        if (producer == 0) {
            char *args[] = {program_name, "PRODUCER", seed_str, fd_1_str, NULL};
            execve(program_name, args, envp);
        }

        // Fork Consumer Process
        pid_t consumer = fork();
        if (consumer == 0) {
            char *args[] = {program_name, "CONSUMER", fd_0_str, NULL};
            execve(program_name, args, envp);
        }

        // Parent Waits For Consumer & Producer
        int producer_status, consumer_status;
        waitpid(producer, &producer_status, 0); 
        waitpid(consumer, &consumer_status, 0);

        // Get Exit Codes
        int producer_exit = WEXITSTATUS(producer_status);
        int consumer_exit = WEXITSTATUS(consumer_status);

        printf("Producer Exited With Status: %d\n", producer_exit);
        printf("Consumer Exited With Status: %d\n", consumer_exit);

    } else if (strcmp(argv[1], "PRODUCER") == 0) { // Producer Mode
        int write_fd = atoi(argv[3]);
        int seed = atoi(argv[2]);

        srand(seed);
        int num = rand() % 90 + 10;
        dprintf(write_fd, "%d\n", num);
        
        printf("Producer Wrote: %d\n", num);
        return num / 10;
    } else if (strcmp(argv[1], "CONSUMER") == 0) { // Consumer Mode
        int read_fd = atoi(argv[2]);
        char buffer[16];
        read(read_fd, buffer, sizeof(buffer));
        int num = atoi(buffer);

        printf("Consumer read: %d\n", num);
        return num % 10;
    } 

    return 0;
}