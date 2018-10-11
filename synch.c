#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/sem.h>

#define SIZE 16

/**
* This class is for understanding the use
* of proper synchronization mechanisms
*
* @author Allison Bolen
* @author Alec Allain
* @version 10/7/18
*/ 

/** Global varaibles */
struct sembuf semBuffer;

/** Instanciating methods */

/**
* Main method
*/ 
int main (int argc, char** argv) { 
   int status; 
   long int i, loop, temp, *shmPtr; 
   int shmId; 
   pid_t pid;
   int semId;

   // Creates new semaphore
   semId = semget (IPC_PRIVATE, 1, 00600);

   // Initializes the semaphore set
   semctl (semId, 0, SETVAL, 1);

      // get value of loop variable (from command-line argument)
   loop = atoi(argv[1]);

   if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
      perror ("i can't get no..\n"); 
      exit (1); 
   } 
   if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) { 
      perror ("can't attach\n"); 
      exit (1); 
   }

   shmPtr[0] = 0; 
   shmPtr[1] = 1;

   if (!(pid = fork())) { 
      for (i=0; i<loop; i++) { 
	      // swap the contents of shmPtr[0] and shmPtr[1]
	      
	      // Semaphore creates block signal
	      semop (semId, semBuffer, 1);

	      memcpy(&temp, &shmPtr[0], sizeof(shmPtr));
	      memcpy(&shmPtr[0], &shmPtr[1], sizeof(shmPtr));
	      memcpy(&shmPtr[1], &temp, sizeof(shmPtr));

	      // Semaphore stops the block
	      semop (semId, semBuffer, 1);

	      // wrong
	      //temp = shmPtr[0];
	      //shmPtr[0] = shmPtr[1];
	      //shmPtr[1] = temp;
      } 
      if (shmdt (shmPtr) < 0) { 
         perror ("just can't let go\n"); 
         exit (1); 
      } 
      exit(0); 
   } 
   else 
      for (i=0; i<loop; i++) { 
               // swap the contents of shmPtr[1] and shmPtr[0]   
	       
	       // Semaphore creates block signal
	       semop (semId, semBuffer, 1);

	       memcpy(&temp, &shmPtr[1], sizeof(shmPtr));
	       memcpy(&shmPtr[1], &shmPtr[0], sizeof(shmPtr));
	       memcpy(&shmPtr[0], &temp, sizeof(shmPtr));

	       // Semaphore stops block signal
	       semop (semId, semBuffer, 1);

	       // wrong
	       //temp = shmPtr[1];
	       //shmPtr[1] = shmPtr[0];
	       //shmPtr[0] = temp; 
      }

   wait (&status); 
   printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);

   if (shmdt (shmPtr) < 0) { 
      perror ("just can't let go\n"); 
      exit (1); 
   } 
   if (shmctl (shmId, IPC_RMID, 0) < 0) { 
      perror ("can't deallocate\n"); 
      exit(1); 
   }

   // Removes the semaphore


   return 0; 
}
