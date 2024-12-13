#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 1

typedef struct {
    char *data;
} Event;

Event buffer[BUFFER_SIZE];
int count = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_var;

void* producer(void* arg) {
    while (1) {
        sleep(1); //delay 1 sec
        Event event;
        event.data = "Event send";

        pthread_mutex_lock(&mutex);
        
        //Wait free place
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&cond_var, &mutex);
        }

        buffer[count] = event;
        count++;
        printf("%s\n", event.data);

        pthread_cond_signal(&cond_var); // Notify consumer
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* consumer(void* arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        
        //Wait until appears
        while (count == 0) {
            pthread_cond_wait(&cond_var, &mutex);
        }

        printf("Event taken\n");
        count--;

        pthread_cond_signal(&cond_var); // Notify producer
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
    pthread_join(consumer_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_var);

    return 0;
}
