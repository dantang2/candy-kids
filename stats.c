//Filename : stats.c
//CMPT 300 - Project 3
//Dan Tang - 301256468 - dltang@sfu.ca
//Last modified on: November 6 2017

#include "stats.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>


typedef struct Statistics
{
  int factoryNumber, numberOfCandiesMade, numberOfCandiesConsumed;
  double minDelay, avgDelay, maxDelay, totalDelay;
}stats;

stats* statsArray;

sem_t mutex;
int numberOfFactories = 0;

void stats_init(int number)
{
  numberOfFactories = number;
  statsArray = malloc(number*sizeof(stats));
  for(int i = 0; i<number; i++)
  {
    statsArray[i].factoryNumber = i;
    statsArray[i].numberOfCandiesMade = 0;
    statsArray[i].numberOfCandiesConsumed = 0;
    statsArray[i].minDelay = 0;
    statsArray[i].avgDelay = 0;
    statsArray[i].maxDelay = 0;
    statsArray[i].totalDelay = 0;
  }
  sem_init(&mutex, 0, 1);
}

void stats_cleanup(void)
{
  free(statsArray);
  statsArray = NULL;
}

void stats_record_produced(int factoryNumber)
{
  sem_wait(&mutex); // blocks if another thread is in its critical section
  statsArray[factoryNumber].numberOfCandiesMade++;
  sem_post(&mutex); // unblocks when done with critical section
}


void stats_record_consumed(int factoryNumber, double delay_in_ms)
{

  sem_wait(&mutex); // blocks if another thread is in its critical section

  statsArray[factoryNumber].numberOfCandiesConsumed++;
  if(statsArray[factoryNumber].totalDelay == 0) // test for first candy
  {
    statsArray[factoryNumber].minDelay = delay_in_ms;
    statsArray[factoryNumber].avgDelay = delay_in_ms;
    statsArray[factoryNumber].maxDelay = delay_in_ms;
    statsArray[factoryNumber].totalDelay = delay_in_ms;
  }

  else
  {
    if(statsArray[factoryNumber].minDelay > delay_in_ms)
    {
      statsArray[factoryNumber].minDelay = delay_in_ms;
    }

    if(statsArray[factoryNumber].maxDelay < delay_in_ms)
    {
      statsArray[factoryNumber].maxDelay = delay_in_ms;
    }

    statsArray[factoryNumber].totalDelay = statsArray[factoryNumber].totalDelay + delay_in_ms;
    
    statsArray[factoryNumber].avgDelay = statsArray[factoryNumber].totalDelay/statsArray[factoryNumber].numberOfCandiesConsumed;

  }

  sem_post(&mutex); // unblocks when done with critical section
}

void stats_display(void)
{
	printf("\nStatistics: \n");
	printf("%s%10s%12s%24s%24s%24s\n", "Factory #", "# Made", "# Eaten", "Min Delay [ms]", "Avg Delay [ms]", "Max Delay [ms]");
  
	for(int i=0; i<numberOfFactories; i++)
  {

    if(statsArray[i].numberOfCandiesMade != statsArray[i].numberOfCandiesConsumed)
    {
			printf("\tERROR: Mismatch between number made and eaten.\n");
		}

		else
    {
      printf("%6d%10d%12d%24f%24f%24f\n",
				statsArray[i].factoryNumber, statsArray[i].numberOfCandiesMade, statsArray[i].numberOfCandiesConsumed,
				statsArray[i].minDelay, statsArray[i].avgDelay, statsArray[i].maxDelay);
    }


	}
}
