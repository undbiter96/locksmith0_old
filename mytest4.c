#include <pthread.h>
#include <stdio.h>

pthread_mutex_t lock[3];
pthread_mutex_t simple_lock;
int shared;
int unprotected;
int arr[5];
int simple;

void* run(void*);
void* my_func(void*);

int main() {
  pthread_t t[3];
  pthread_t t1;

  pthread_mutex_init(&simple_lock, NULL);
  int i;
  for (i = 0; i < 3; i++)
  {
	int *a = &i;
	pthread_mutex_init(&lock[i], NULL);
  	pthread_create(&t[i], NULL, run, a);
  }

  arr[0] = 5;
  arr[1] = 9;
  arr[2] = 16;
  arr[3] = 11;
  arr[4] = 8;
  pthread_create(&t1, NULL, my_func, NULL);
  int a = unprotected;
  unprotected = arr[3];
  simple = arr[0];
  return 1;
}

void *run(void *arg) {
  int index = *arg;
  pthread_mutex_lock(&lock[index]);
  shared++;
  printf("Done\n");
  pthread_mutex_unlock(&lock[index]);
  return NULL;
}


void *my_func(void *arg) 
{
  unprotected = 5;
  arr[0] = 18;
  arr[3] = 10;
  pthread_mutex_lock(&simple_lock);
  unprotected++;
  pthread_mutex_unlock(&simple_lock);
  unprotected++;
  return NULL;
}
