#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

pthread_mutex_t m;
pthread_cond_t red[2];
int brojStudenata, uSobi = 0, brejker = 0, id = 0;

void udi_u_sobu (int k)
{
	pthread_mutex_lock(&m);
	while (brejker == 1)
		pthread_cond_wait(&red[1], &m);
      	printf("Student %d je usao u sobu (u sobi: %d)\n", k, uSobi + 1);
      	uSobi++;
        if (uSobi >= 3) {
            	pthread_cond_signal(&red[0]);
        }
      	pthread_mutex_unlock(&m);
}

void izadi_iz_sobe (int k, int i)
{
    	pthread_mutex_lock(&m);
      	printf("Student %d je izasao iz sobe (u sobi: %d)\n", k, uSobi - 1);
      	uSobi--;
      	if (i == 2)
		brojStudenata--;
      	if (uSobi == 0)
            	pthread_cond_signal(&red[0]);
	pthread_mutex_unlock(&m);
}

void break_party ()
{
      	pthread_mutex_lock(&m);
      	while (uSobi < 3)
        	pthread_cond_wait(&red[0], &m);
      	printf("Partibrejker je usao u sobu\n");
      	brejker = 1;
      	pthread_mutex_unlock(&m);

      	pthread_mutex_lock(&m);
      	while (uSobi > 0)
        	pthread_cond_wait(&red[0], &m);
      	printf("Partibrejker je izasao iz sobe\n");
      	brejker = 0;
      	pthread_cond_broadcast(&red[1]);
      	pthread_mutex_unlock(&m);
}

void *partybrejker ()
{
    	while (brojStudenata > 0) {
   		usleep( 1000 * (int) (rand() % (1000 - 100 + 1) + 100)); //spavaj X milisekundi
      		break_party();
    	}
    	return NULL;
}

void *student ()
{
    	int k = ++id;
    	usleep( 1000 * (int) (rand() % (500 - 100 + 1) + 100));
    	for (int i = 0; i < 3; i++) {
      		udi_u_sobu(k);
      		usleep( 1000 * (int) (rand() % (2000 - 1000 + 1) + 1000)); //zabavi se
      		izadi_iz_sobe(k, i);
      		usleep( 1000 * (int) (rand() % (2000 - 1000 + 1) + 1000));
   	}
   	return NULL;
}

int main (void)
{
    	pthread_t thr_id;
    	pthread_attr_t attr;

    	srand ((unsigned int) time(NULL));

    	printf("Unesite broj studenata u sustavu: \n");
    	scanf("%d", &brojStudenata);

    	pthread_mutex_init(&m, NULL);
    	pthread_cond_init(&red[0], NULL);
    	pthread_cond_init(&red[1], NULL);

    	pthread_attr_init(&attr);
    	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    	pthread_create(&thr_id, &attr, partybrejker, NULL);
    	for (int i = 0; i < brojStudenata; i++) {
        	pthread_create(&thr_id, &attr, student, NULL);
        	sleep(1);
    	}
    	while (brojStudenata > 0)
		sleep(1);
    	return 0;
}
