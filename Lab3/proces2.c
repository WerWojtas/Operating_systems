#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int global = 0; // Zmienna globalna

int main(int argc, char *argv[]) {
    int local = 0; 
    
    if (argc != 2) {
        fprintf(stderr, "Bad number of arguments\n");
        return 1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        printf("child process\n");
        global++; 
        local++;
        printf("child pid = %d, parent pid = %d\n", getpid(), getppid());
        printf("child's local = %d, child's global = %d\n", local, global);
        execl("/bin/ls", "ls", argv[1], NULL);
        perror("execl");
        return 1;
    } else {
        printf("parent process\n");
        printf("parent pid = %d, child pid = %d\n", getpid(), pid);
        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            printf("Child exit code: %d\n", WEXITSTATUS(status));
        }
        printf("Parent's local = %d, parent's global = %d\n", local, global);
        return 0;
    }
}
