// Noah Gallego
#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t processID = fork(); // Store a Process ID (0 or 1)
    
    const char *str = (!processID ? "Parent" : "Child");

    printf("%s program is running!\n", str);
    sleep(5);
    return 0;
}
