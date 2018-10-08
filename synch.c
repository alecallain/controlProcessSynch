#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/sem.h>

#define SIZE 1024

/**
* This class is for understanding the use
* of proper synchronization mechanisms
*
* @author Allsion Bolen
* @author Alec Allain
* @version 10/7/18
*/ 

/** Global variables */
int shmId;
char* shmPtr;
pid_t pid;
sem_t mutex;
int status;
int semId;
struct sembuf semBuffer;

/** Instanciating methods */
void sigHandler(int input);

/**
* Main method
*/
int main (int argc, char** argv) {
	signal(SIGINT, sigHandler);

	if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0 ) {
		fprintf(stderr, "Error getting ID\n");
		exit(1);
	} else if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) {
		fprintf(stderr, "\n");
		exit(1);
	}

	printf("\nCreating a semaphore\n");

	// creates a new semaphore 
	semId = semget (IPC_PRIVATE, 1, 600);

	// initializes the semaphore set
	semctl (semId, 0, SETVAL, 1);

	// performs operations on elements
	semop(semId, &semBuffer, 1);

	// removes the semaphore
	semctl (semId, 0, IPC_RMID);

}

/**
* This method waits for the control-c 
* function from the user
*/
void sigHandler (int input) {
	printf("Exiting program...\nDeleting and detaching connections\n");

	if (shmdt(shmPtr) < 0) {
		fprintf(stderr, "Can't let go...\n");
		exit(1);
	} else if (shmctl(shmId, IPC_RMID, 0) < 0) {
		fprintf(stderr, "Memory can't be deallocated");
		exit(1);
	}

	exit(0);
}
