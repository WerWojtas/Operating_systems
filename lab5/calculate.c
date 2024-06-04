#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

double fun(double x) {
    return 4.0 / (x * x + 1);
}

double rectangle(double a, double b) { // Dzielimy nasz obszar od a do b na przedziały szerokości width
    double sum = 0.0;                                // następnie dla każdego przedziału obliczamy pole prostokąta o
    double mid = (b+a)/2;                                     // wysokości funkcji w tym przedziale, pola sumujemy, jest to nasz
    sum = fun(mid) * (b-a);
    printf("%d\n",sum);
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Bad number of args");
        exit(EXIT_FAILURE);
    }

    int process = atoi(argv[1]);
    const char *pipe_name = "/tmp/newpipes";
    int mypipe = open(pipe_name, O_RDONLY);
    if (mypipe == -1) {
        perror("Cannot open pipe");
        exit(EXIT_FAILURE);
    }
    double start, end;
    if (read(mypipe, &start, sizeof(double)) == -1 || read(mypipe, &end, sizeof(double)) == -1) {
        perror("Read pipe operation ended with failure");
        close(mypipe);
        exit(EXIT_FAILURE);
    }
    double interval = (end-start)/process;
    int pipes[process][2];

    for (int i = 0; i < process; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Pipe has not been created properly");
            exit(EXIT_FAILURE);
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("Process has not been created properly");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { 
            close(pipes[i][0]);
            double result = rectangle(i*interval+start, (i+1)*interval+start);
            if (write(pipes[i][1], &result, sizeof(result)) == -1) {
                perror("Write operation has not ended succesfully");
                exit(EXIT_FAILURE);
            }

            close(pipes[i][1]);
            exit(EXIT_SUCCESS);
        } else {
            close(pipes[i][1]);
        }
    }
    for (int i = 0; i < process; i++) {
        wait(NULL);
    }
    double sum = 0.0;
    for (int i = 0; i < process; i++) {
        double result;
        if (read(pipes[i][0], &result, sizeof(result)) == -1) {
            perror("Read operation has not ended succesfully");
            exit(EXIT_FAILURE);
        }
        sum += result;
        close(pipes[i][0]);
    }
    mypipe = open(pipe_name, O_WRONLY);
    if (mypipe == -1) {
        perror("Openning pipe ended with failure 3");
        exit(EXIT_FAILURE);
    }
    if (write(mypipe, &sum, sizeof(double)) == -1) {
        perror("Writing operation ended with failure");
        close(mypipe);
        exit(EXIT_FAILURE);
    }

    close(mypipe);


    return 0;
}
