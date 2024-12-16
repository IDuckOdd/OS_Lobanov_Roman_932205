#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
int ready = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_var;

void* producer(void* arg) {
    while(1) {
        sleep(1); //delay 1 sec

        pthread_mutex_lock(&mutex);

        //Wait free place
        if (ready == 1) {
            pthread_mutex_unlock(&mutex);
            continue;
        }

        ready = 1;
        printf("%s\n", "Event send");

        pthread_cond_signal(&cond_var); //Notify consumer
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* consumer(void* arg) {
    while (1) {

        pthread_mutex_lock(&mutex);

        //Wait until appears
        while (ready == 0) {
            pthread_cond_wait(&cond_var, &mutex);
        }

        printf("Event taken\n");
        ready = 0;

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_var, NULL);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);

    return 0;
}
