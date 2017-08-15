/*
 * A multi-thread program that gives readers priority over writers
 * concerning a shared (global) variable. Essentially, if any readers are
 * waiting, then they have priority over writer threads -- writers can only
 * write when there are no readers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5
#define LOOP 10

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;					// mutex lock
pthread_cond_t writer_condition = PTHREAD_COND_INITIALIZER;		// writer condition variable
pthread_cond_t reader_condition = PTHREAD_COND_INITIALIZER;		// reader condition variable

int waitingReaders = 0;
int resource_counter = 0;
int variable = 0;

void *writer(void *param);
void *reader(void *param);

int main(int argc, char *argv[]) {
	pthread_t tid1[NUM_THREADS], tid2[NUM_THREADS];  // thread identifiers
	int i;
	int tNum[NUM_THREADS];

	// create the writer threads
	for (i = 0; i < NUM_THREADS; i++) {
		tNum[i] = i;
		if (pthread_create(&tid1[i], NULL, writer, &tNum[i]) != 0) {
			fprintf(stderr, "Unable to create writer thread\n");
			exit(1);
		}
	}

	// create the reader threads
	for (i = 0; i < NUM_THREADS; i++) {
		if (pthread_create(&tid2[i], NULL, reader, &tNum[i]) != 0) {
			fprintf(stderr, "Unable to create reader thread\n");
			exit(1);
		}
	}

	// wait for created thread to exit
	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(tid1[i], NULL);
	}

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(tid2[i], NULL);
	}

	printf("Parent quitting\n");

	return 0;
}

void *writer(void *param) {
	int id = *((int*)param);
	int i;

	for (i = 0; i < LOOP; i++) {
		// Wait so that reads and writes do not all happen at once
		usleep(1000 * (random() % NUM_THREADS + id));

		pthread_mutex_lock(&m);
			while (resource_counter != 0) {
				pthread_cond_wait(&writer_condition, &m);
			}
			resource_counter = -1;
		pthread_mutex_unlock(&m);

		// Write
		variable++;
		printf("Writer %d is accessing on iteration %d: %d\n", id, i, variable);
		printf("Number of readers: %d\n", resource_counter);
		fflush(stdout);

		pthread_mutex_lock(&m);
			resource_counter = 0;
			if (waitingReaders > 0) {
				pthread_cond_broadcast(&reader_condition);
			} else {
				pthread_cond_signal(&writer_condition);
			}
		pthread_mutex_unlock(&m);
	}

	printf("Writer %d finished\n", id);
	fflush(stdout);
	return 0;
}

void *reader(void *param) {
	int id = *((int*)param);
	int i;

	for (i = 0; i < LOOP; i++) {
		// Wait so that reads and writes do not all happen at once
		usleep(1000 * (random() % NUM_THREADS + NUM_THREADS));

		pthread_mutex_lock(&m);
			waitingReaders++;
			while (resource_counter == -1) {
				pthread_cond_wait(&reader_condition, &m);
			}
			waitingReaders--;
			resource_counter++;
		pthread_mutex_unlock(&m);

		// Read
		printf("Reader %d is accessing on iteration %d: %d\n", id, i, variable);
		printf("Number of readers: %d\n", resource_counter);
		fflush(stdout);

		pthread_mutex_lock(&m);
			resource_counter--;
			if (resource_counter == 0) {
				pthread_cond_signal(&writer_condition);
			}
		pthread_mutex_unlock(&m);
	}

	printf("Reader %d finished\n", id);
	fflush(stdout);
	return 0;
}

