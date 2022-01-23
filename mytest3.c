#include <pthread.h>
#include <stdio.h>

pthread_mutex_t lock[3];
int shared;
int other;
char *str; 

void* run(void*);

int main() {
  pthread_t t[3];

  //pthread_mutex_init(&lock, NULL);
  int i;
  shared = 0;
  other = 0;
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
  int *ptr = &shared;
  pthread_mutex_lock(&lock[i]);
  shared++;
  str = realloc(str, shared * sizeof(char));
  str[0] = 'a';
  pthread_mutex_unlock(&lock[i]);
  char *cpy = str;
  pthread_mutex_lock(&lock[i]);
  other--;
  cpy = realloc(cpy, shared * sizeof(char));
  cpy[0] = 'b';
  shared += other;
  pthread_mutex_unlock(&lock[i]);
  return NULL;
}

