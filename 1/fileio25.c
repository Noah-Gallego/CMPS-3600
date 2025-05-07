// Noah Gallego
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    char name[200];
    write(1, "Enter your name: ", sizeof("Enter your name: ") - 1);
    int bytes_read = read(0, name, sizeof(name));

    char num_str[20];
    write(1, "Enter a number: ", sizeof("Enter a number: ") - 1);
    bytes_read = read(0, num_str, sizeof(num_str));
    
    int num = atoi(num_str);

    int fd = open("log", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    char buffer[200];
    int len = snprintf(buffer, sizeof(buffer), "User's Name: %s\n", name);
    write(fd, buffer, len);

    int sum = 0;
    for (int i = 1; i <= num; i++) {
        sum += i;
    }

    len = snpriddntf(buffer, sizeof(buffer), "Summation Total (1 to %d): %d\n", num, sum);
    write(fd, buffer, len);

    close(fd);

    return 0;
}
