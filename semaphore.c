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

/*
* This demonstraits how to use semaphores to protect shared memory.
* Allison Bolen
* Alec Allain
* Oct 2018
*/


int main (int argc, char *argv[])
{
   struct sembuf WAIT[1], SIGNAL[1];
   int status;
   long int i, loop, temp, *shmPtr;
   int shmId;
   pid_t pid;
   int semId;

   /*  create a new semaphore set for use by this (and other) processes..
   */
   semId = semget (IPC_PRIVATE, 1, 00600);
   /*  initialize the semaphore set referenced by the previously obtained semId handle.
   */
   semctl (semId, 0, SETVAL, 1);
   // initialze sembuf set up on how to use teh semop function
   WAIT[0].sem_num = 0;
   WAIT[0].sem_op = -1;
   WAIT[0].sem_flg = SEM_UNDO;

   // initialze sembuf set up on how to use teh semop function
   SIGNAL[0].sem_num = 0;
   SIGNAL[0].sem_op = 1;
   SIGNAL[0].sem_flg = SEM_UNDO;
   
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
         semop(semId, WAIT, 1); // semaphore wait block
	       memcpy(&temp, &shmPtr[0], sizeof(shmPtr[0]));
	       memcpy(&shmPtr[0], &shmPtr[1], sizeof(shmPtr[1]));
	       memcpy(&shmPtr[1], &temp, sizeof(shmPtr[1]));
         semop(semId, SIGNAL, 1); // semaphore signal release
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
         semop(semId, WAIT, 1); // semaphore wait block
	       memcpy(&temp, &shmPtr[0], sizeof(shmPtr[0]));
	       memcpy(&shmPtr[0], &shmPtr[1], sizeof(shmPtr[1]));
	       memcpy(&shmPtr[1], &temp, sizeof(shmPtr[1]));
         semop(semId, SIGNAL, 1); // semaphore signal release
      }

   wait(&status);
   printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);

   if (shmdt (shmPtr) < 0) {
      perror ("just can't let go\n");
      exit (1);
   }
   if (shmctl (shmId, IPC_RMID, 0) < 0) {
      perror ("can't deallocate\n");
      exit(1);
   }

   return 0;
}
