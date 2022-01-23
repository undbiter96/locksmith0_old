#include <pthread.h>
#include <stdio.h>

pthread_mutex_t lock[3];
int shared;

void* run(void*);

int main() {
  pthread_t t[3];

  //pthread_mutex_init(&lock, NULL);
  int i;
  for (i = 0; i < 3; i++)
  {
	int *a = &i;
	pthread_mutex_init(&lock[i], NULL);
  	pthread_create(&t[i], NULL, run, a);
  }
  return 1;
}

void *run(void *arg) {
  int i = *arg;
  pthread_mutex_lock(&lock[i]);
  shared++;
  printf("Done\n");
  pthread_mutex_unlock(&lock[i]);
  return NULL;
}

