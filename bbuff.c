//Filename : bbuff.c
//CMPT 300 - Project 3
//Dan Tang - 301256468 - dltang@sfu.ca
//Last modified on: November 6 2017


#include "bbuff.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <time.h>

sem_t isBufferEmpty;
sem_t isBufferFull;
sem_t mutex;

void* bufferArray[BUFFER_SIZE];

int counter;
int produced;
int consumed;


void bbuff_init(void)
{
  counter = 0;
  produced = 0;
  consumed = 0;

  //Initialize semaphores
  sem_init(&mutex, 0, 1);
  sem_init(&isBufferFull, 0, BUFFER_SIZE); 
  sem_init(&isBufferEmpty, 0, 0); 
}

void bbuff_blocking_insert(void* aCandy)
{
  sem_wait(&isBufferFull); // decrements full buffer semaphore, is blocked if buffer is full
  sem_wait(&mutex); //blocks if another thread is in its critical section

  bufferArray[produced] = aCandy;
  produced = (produced+1) % BUFFER_SIZE;
  counter++;

  sem_post(&mutex);//unblocks when done with critical section
  sem_post(&isBufferEmpty); //increments empty buffer semaphore, to indicate that there is an item in the bounded buffer

  return;
}

void *bbuff_blocking_extract(void)
{
  sem_wait(&isBufferEmpty); //decrement empty buffer semaphore, is blocked if buffer is empty
  sem_wait(&mutex); // blocks if another thread is in its critical section

  void* aCandy = bufferArray[consumed];
  bufferArray[consumed] = NULL;
  consumed = (consumed+1) % BUFFER_SIZE;
  counter--;

  sem_post(&mutex); // unblocks when done with critical section
  sem_post(&isBufferFull); // increments full buffer semaphore, to indicate that an item as been removed from the bounded buffer

  return aCandy;
}

_Bool bbuff_is_empty(void)
{
  _Bool empty = counter == 0;
  return empty; // check if bounded buffer is empty
}

double current_time_in_ms(void)
{
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return now.tv_sec * 1000.0 + now.tv_nsec/1000000.0;
}