/* PThread creation with detached attribute*/

#include <stdio.h>
#include <pthread.h>

void *foo (void *arg) {
	printf("Foobar!\n");
	pthread_exit(NULL);
}

int main (void) {

	pthread_t tid; 			// thread ID
	pthread_attr_t attr; 	// thread attribute

	// set thread detach state attribute to DETACHED
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	// set scope attribute and create the thread
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_create(&tid, &attr, foo, NULL);

	return 0;
}
