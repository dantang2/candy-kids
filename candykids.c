//Filename : candykids.c
//CMPT 300 - Project 3
//Dan Tang - 301256468 - dltang@sfu.ca
//Last modified on: November 6 2017

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "bbuff.h"
#include "stats.h"



//candy struct
typedef struct candy
{
	int factory_number;
	double time_stamp_in_ms;
}candy_t;




void launchThread(int counter, pthread_t* thread, void* (*start_thread)(void*))
{
	int thisThread = 0;
	pthread_t id;

	for(int i=0; i<counter; i++) // spawn requested number of candy-factory or kid threads
	{
		thisThread = pthread_create(&id, NULL, start_thread, (void*)(intptr_t)i);
		if(thisThread)
		{
			printf("ERROR: Thread could not launch.\n");
			printf("\n ==========CLOSING CANDY KIDS==========\n\n");
			exit(1);
		}
		thread[i] = id; // store thread IDs in an array
	}


}

_Bool stop_thread = false;

void* factoryThreadFunction(void* arg)
{

	int i = (intptr_t)arg; //change arg into a usuable integer 
	int waitTime;

	while(!stop_thread) // loop until signal
	{
		waitTime = rand()%4; // numbers will range from 0-3
		printf("\tFactory %d ships candy & waits %ds \n", i, waitTime);

		candy_t* thisCandy = malloc(sizeof(candy_t));
		thisCandy->factory_number = i;
		thisCandy->time_stamp_in_ms = current_time_in_ms();

		bbuff_blocking_insert(thisCandy); //add candy item to bounded buffer
		stats_record_produced(i); //add candy to stats array

		sleep(waitTime); // wait for specified amount of time
	}

	printf("Candy-factory %d done\n", i);

	pthread_exit(NULL);


}

void* kidThreadFunction(void* arg)
{
	int waitTime;

	while(true)
	{
		waitTime = rand()%2; // wait time ranges can be either 0 or 1

		candy_t* thisCandy = (candy_t*) bbuff_blocking_extract(); //remove candy from bounded buffer

		if(thisCandy!=NULL) // test in case no candy present
		{
			stats_record_consumed(thisCandy->factory_number, current_time_in_ms()-thisCandy->time_stamp_in_ms); //remove candy from stats array
		}


		free(thisCandy);
		sleep(waitTime);
	}

	pthread_exit(NULL); // terminate thread

}



int main(int argc, char* argv[])
{

	//system("clear");
	printf("\n ==========STARTING CANDY KIDS==========\n\n");

	//Test arguments
	if(argc!=4) // Exit if there are not 3 arguments
	{
		printf("ERROR: Incorrect arguments.\nArguments must contain three positive integers seperated by a space.\n");
		printf("\n ==========CLOSING CANDY KIDS==========\n\n");

		exit(1);
	}

	else
	{
		for(int i=1; i<=3; i++)
		{
			if(atoi(argv[i])<1) // Exit if the arguments are less than 0
			{
				printf("ERROR: Incorrect arguments.\nArguments must contain three positive integers seperated by a space.\n");
				printf("\n ==========CLOSING CANDY KIDS==========\n\n");

				exit(1);
			}
		}
	}


	int numberOfFactories = 0,
		numberOfKids = 0,
		numberOfSeconds = 0;

	int* arguments[3] = {&numberOfFactories, &numberOfKids, &numberOfSeconds};

	//1. Extract Arguments

	for(int i=0; i<3; i++)
	{
		sscanf(argv[i+1], "%d", arguments[i]);
	}

	//2. Modules
	stats_init(numberOfFactories);
	bbuff_init();

	//3. Launch candy-factory threads
	pthread_t* factoryThreads = malloc(numberOfFactories *(sizeof(pthread_t)));
	launchThread(numberOfFactories, factoryThreads, (void*)factoryThreadFunction);

	//4. Launch kid threads
	pthread_t* kidThreads = malloc(numberOfKids *(sizeof(pthread_t)));
	launchThread(numberOfKids, kidThreads, (void*)kidThreadFunction);

	//5. Wait for requested time
	for(int i=0; i<numberOfSeconds; i++)
	{
		sleep(1);
		printf("Time: %ds\n", i+1);
	}

	//6. Stop-candy factory threads
	stop_thread = true; // signal to stop thread
	printf("Stopping candy factories ...\n");
	for(int i=0; i<numberOfFactories; i++)
	{
		pthread_join(factoryThreads[i], NULL); //join terminated threads
	}

	//7. Wait until no more candy
	printf("Waiting for all candy to be consumed\n");
	while(!bbuff_is_empty()) // check for empty buffer
	{
		sleep(1);
	}

	//8. Stop kid threads
	printf("Stopping kids.\n");
	for(int i=0; i<numberOfKids; i++)
	{
		pthread_cancel(kidThreads[i]); // Cancel kid threads if it is not complete 
		pthread_join(kidThreads[i], NULL); // join terminated threads
	}

	//9. Print statistics
	stats_display();

	//10. Cleanup any allocated memory
	stats_cleanup();
	free(factoryThreads);
	free(kidThreads);

	printf("\n ==========CLOSING CANDY KIDS==========\n\n");
	return 0;



}
