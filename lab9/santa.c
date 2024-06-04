#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_REINDEER 9
#define DELIVERIES 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t santa_cond = PTHREAD_COND_INITIALIZER;

int reindeer_count = 0;
int deliveries_made = 0;

void* santa_thread(void* arg) {
    while (deliveries_made < DELIVERIES) {
        pthread_mutex_lock(&mutex);
        while (reindeer_count < NUM_REINDEER) {
            pthread_cond_wait(&santa_cond, &mutex);
        }
        printf("Santa starts delivering toys\n");
        sleep(rand() % 3 + 2); 
        reindeer_count = 0;
        deliveries_made++;
        printf("Santa gets back to sleep\n");
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* reindeer_thread(void* arg) {
    int id = *(int*)arg;
    free(arg);
    while (1) {
        sleep(rand() % 6 + 5); 
        pthread_mutex_lock(&mutex);
        reindeer_count++;
        printf("Reindeer %d gets back from vacation, reindeers waiting  %d\n", id, reindeer_count);
        if (reindeer_count == NUM_REINDEER) {
            printf("Reindeer %d wakes up santa\n", id);
            pthread_cond_signal(&santa_cond);
        }
        pthread_mutex_unlock(&mutex);
        sleep(rand() % 3 + 2);
    }
    return NULL;
}

int main() {
    pthread_t santa;
    pthread_t reindeers[NUM_REINDEER];

    pthread_create(&santa, NULL, santa_thread, NULL);

    for (int i = 0; i < NUM_REINDEER; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&reindeers[i], NULL, reindeer_thread, id);
    }

    pthread_join(santa, NULL);

    for (int i = 0; i < NUM_REINDEER; i++) {
        pthread_cancel(reindeers[i]);
        pthread_join(reindeers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&santa_cond);

    return 0;
}
