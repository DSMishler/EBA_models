#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t lock;

int ints[16] = {5};

void *fakelock;

void* threadfunc(void *arg)
{
   pthread_mutex_lock(fakelock);
   printf("mythreadnum is %d\n", *(int*)arg);
   sleep(1);
   pthread_mutex_unlock(fakelock);
   return NULL;
}

void* tfunc(void* arg)
{
   pthread_mutex_lock(fakelock);
   printf("mythreadnum is %lld\n", (long long int) arg);
   sleep(1);
   pthread_mutex_unlock(fakelock);
   return NULL;
}

int main(void)
{
   printf("ints[0]: %d\n", ints[0]);
   printf("ints[2]: %d\n", ints[2]);
   pthread_t tids[4];
   int targs[4];
   fakelock = malloc(sizeof(pthread_mutex_t));
   
   printf("thread mutex tester\n");
   printf("size is %lu\n", sizeof(pthread_mutex_t));

   pthread_mutex_init(fakelock, NULL);

   int i;
   for(i = 0; i < 4; i++)
   {
      targs[i] = i;
      pthread_create(tids+i, NULL, tfunc, (void*)(targs[i]));
   }

   for(i = 0; i < 4; i++)
   {
      pthread_join(tids[i], NULL);
   }

   pthread_mutex_destroy(fakelock);

   free(fakelock);

   return 0;
}
