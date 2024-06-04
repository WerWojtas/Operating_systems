#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
      if (argc != 3) {
        fprintf(stderr, "Bad number of args");
        exit(EXIT_FAILURE);
    }
    double start = atof(argv[1]); 
    double end = atof(argv[2]); 
    const char *pipe_name = "/tmp/newpipes";
    if (mkfifo(pipe_name, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    int mypipe = open(pipe_name, O_WRONLY);
    if (mypipe == -1) {
        perror("Opening pipe has ended with failure 1");
        exit(EXIT_FAILURE);
    }
    if (write(mypipe, &start, sizeof(double)) == -1 || write(mypipe, &end, sizeof(double)) == -1) {
        perror("Writing start and end data in pipe ended with failrue");
        close(mypipe);
        exit(EXIT_FAILURE);
    }
    close(mypipe);

    mypipe = open(pipe_name, O_RDONLY);
    if (mypipe == -1) {
        perror("Opening pipe for reading has ended with failure 2");
        exit(EXIT_FAILURE);
    }
    if (read(mypipe, &end, sizeof(double)) == -1) {
        perror("Reading result data from pipe ended with failure");
        close(mypipe);
        exit(EXIT_FAILURE);
    }
    close(mypipe);
    printf("Result: %f\n", end);
    return 0;
}
