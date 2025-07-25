#include <stdio.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

sem_t *stol_prazan;
sem_t *sir;     // = 3
sem_t *sunka;   // = 2
sem_t *kruh;    // = 1
int *sastojci;  // 1-sir/sunka, 2-sir/kruh, 3-kruh/sunka
sem_t *kupac1;
sem_t *kupac2;
sem_t *kupac3;

void Trgovac ()
{
    	srand(time(NULL));
    	while (1) {
        	sem_wait(stol_prazan);
        	int dva_sastojka = (rand() % 3) + 1;    //odaberi sastojke
        	*sastojci = dva_sastojka;    		//stavi na stol

        	if (dva_sastojka == 1)
            		printf("\nTrgovac stavlja: sunku i sir\n");
        	else if (dva_sastojka == 2)
            		printf("\nTrgovac stavlja: kruh i sir\n");
        	else
			printf("\nTrgovac stavlja: kruh i sunku\n");
		sleep(1);

		sem_post(sir);
        	sem_post(sunka);
        	sem_post(kruh);
   	}
}

void Kupac (int potrebniSastojci)
{
   	while (1) {
        	switch (potrebniSastojci) {
            	case 1: sem_wait(kruh); break;
            	case 2: sem_wait(sunka); break;
            	case 3: sem_wait(sir); break;
        	}

        	if (potrebniSastojci == *sastojci) {
        	    	*sastojci=0;
        	    	printf("Kupac Sendviča %d: uzima sastojke, izlazi iz trgovine, slaže si sendvič i jede\n", potrebniSastojci);
        	    	sleep(1);
        	    	sem_post(stol_prazan);
        	    	return;
        	}
    	}
}

int main (void)
{
    	pid_t pid;
    	int ID = shmget (IPC_PRIVATE, 7 * sizeof(sem_t) + sizeof(int), 0600);
	stol_prazan = shmat (ID, NULL, 0);
    	srand ((unsigned int) time(NULL));

    	sir = (sem_t *) ((char *)stol_prazan + sizeof(sem_t));
    	sunka = (sem_t *) ((char *)stol_prazan + 2 * sizeof(sem_t));
     	kruh = (sem_t *) ((char *)stol_prazan + 3 * sizeof(sem_t));
     	kupac1 = (sem_t *) ((char *)stol_prazan + 4 * sizeof(sem_t));
    	kupac2 = (sem_t *) ((char *)stol_prazan + 5 * sizeof(sem_t));
    	kupac3 = (sem_t *) ((char *)stol_prazan + 6 * sizeof(sem_t));
    	sastojci = (int *) ((char *)stol_prazan + 7 * sizeof(sem_t));

	shmctl (ID, IPC_RMID, NULL); //moze odmah ovdje ili na kraju nakon shmdt

    	sem_init(stol_prazan, 1, 1);
    	sem_init(sir, 1, 0);
    	sem_init(sunka, 1, 0);
      	sem_init(kruh, 1, 0);
    	*sastojci = 0;
    	sem_init(kupac1, 1, 1);
    	sem_init(kupac2, 1, 1);
    	sem_init(kupac3, 1, 1);

    	printf("Kupac Sendviča 1: ima kruh\n");
    	printf("Kupac Sendviča 2: ima sunku\n");
    	printf("Kupac Sendviča 3: ima sir\n\n");
    	if (fork() == 0) {
        	Trgovac();
        	exit(0);
    	}

    	while (1) {
        	int proizvod = (rand() % 3) + 1;
        	if (fork() == 0) {
            		if (proizvod == 1) {
                		sem_wait(kupac1);
                		Kupac(proizvod);    //printf("Kupac1 usao \n");
                		sem_post(kupac1);
            		} else if (proizvod == 2) {
                		sem_wait(kupac2);
                		Kupac(proizvod);    //printf("Kupac2 usao \n");
                		sem_post(kupac2);
            		} else {
                		sem_wait(kupac3);
                		Kupac(proizvod);    //printf("Kupac3 usao \n");
                		sem_post(kupac3);
            		}
            		exit(0);
        	} else {
            		while (waitpid(-1, NULL, WNOHANG) > 0);
        	}
        	sleep(1);
    	}

    	sem_destroy(stol_prazan);
    	sem_destroy(sir);
    	sem_destroy(sunka);
    	sem_destroy(kruh);
    	sem_destroy(kupac1);
    	sem_destroy(kupac2);
    	sem_destroy(kupac3);
    	shmdt(stol_prazan);

   	 return 0;
}
