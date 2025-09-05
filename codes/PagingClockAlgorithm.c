#include <stdio.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BROJ_STRANICA 4
#define BROJ_PROCESA 3
#define BROJ_OKVIRA 4

sem_t *pristup;
int *sat;
int *kazaljka;
int *okvir;

void ispisi_clock (int stanje)
{
        if(stanje == 0)
                printf("clock: ");
        else if (stanje == 1)
                printf("clock before: ");
        else
                printf("clock after: ");

        for (int i = 0; i < BROJ_OKVIRA; i++) {
                if(i != 0)
                        printf("-");
                printf("%d", *(sat + i));
        }
        printf("\n");

        if(stanje == 0)
                printf("hand: ");
        else if (stanje == 1)
                printf("hand before: ");
        else
                printf("hand after: ");

        for(int k = 0; k < BROJ_OKVIRA; k++) {
                if(k == *kazaljka)
                        printf(" ^");
                else
                        printf("  ");
        }
        printf("\n");
}

void ispisi_okvire ()
{
        printf("frames: ");
        for (int k = 0; k < BROJ_OKVIRA; k++) {
                if (*(okvir + k) == -1)
                        printf("%d:[-:-] ", k);
                else
                        printf("%d:[%d:%d] ", k, *(okvir + k) / 10, *(okvir + k) % 10);
        }
        printf("\n");
}

void obradi_zahtjev (int proces, int la)
{
        int trazim = la / 100;
        int hit = -1;
        for (int k = 0; k < BROJ_OKVIRA; k++) {
                if (*(okvir + k) / 10 == proces && *(okvir + k) % 10 == trazim) {
                        hit = k;
                        k = BROJ_OKVIRA;
                }
        }
        if(hit > -1) {
                ispisi_okvire();
                printf("HIT:   ");
                for (int i = 0; i < BROJ_OKVIRA; i++){
                        if(i == hit)
                                printf(" ^^^^^^^");
                        else
                                printf("        ");
                }
                printf("\n");
		int fiz_adresa = la % 100 + 100 * hit;
                printf("stranicenje: proces %d, stranica=%d => frame=%d, LA=%d => FA=%d\n", proces, trazim, hit, la, fiz_adresa);
                printf("citanje/pisanje na lokaciju FA=%d\n", fiz_adresa);
		*(sat + hit) = 1;
                ispisi_clock(0);

        } else {
                printf("MISS (page %d not in memory)\n", trazim);
                ispisi_clock(1);
                while (1) {
                        if (sat[*kazaljka] == 0)
                                break;
                        sat[*kazaljka] = 0;
                        *kazaljka = (*kazaljka + 1) % BROJ_OKVIRA;
                }
                ispisi_clock(2);

                printf("koristim frame %d:\n", *kazaljka);
                if(*(okvir + *kazaljka) != -1)
                        printf("- spremam na disk: proces %d, stranica %d\n", *(okvir + *kazaljka) / 10, *(okvir + *kazaljka) % 10);
                printf("- dohvacam s diska: proces %d, stranica %d\n", proces, trazim);
                *(okvir + *kazaljka) = proces * 10 + trazim;
                ispisi_okvire();

                printf("restarting istruction: \n");
                printf("proces %d zahtjeva pristup LA=%d\n", proces, la);
                obradi_zahtjev(proces,la);
        }
        return;
}

void generiraj_zahtjev (int proces)
{
        while(1) {
                sem_wait(pristup);
                srand((unsigned int)time(NULL));
                int x = (rand() % (BROJ_STRANICA)) * 100;
                x += rand() % (100);
                printf("proces %d zahtjeva pristup LA=%d\n", proces, x);
                obradi_zahtjev(proces, x);
                printf("\n");
                sleep(1);
                sem_post(pristup);
                sleep(1);
        }
        return;
}

int main (void)
{
        size_t totalSize = sizeof(sem_t) + sizeof(int) * (BROJ_OKVIRA * 2 + 1);
        int ID = shmget (IPC_PRIVATE, totalSize, 0600);
        char *pocetak = shmat (ID, NULL, 0);
    
        pristup = (sem_t *) pocetak;
        sat = (int *) (pocetak + sizeof(sem_t));
        okvir = (int *) (pocetak + sizeof(sem_t) + sizeof(int) * (BROJ_OKVIRA));
        kazaljka = (int *) (pocetak + sizeof(sem_t) + sizeof(int) * (BROJ_OKVIRA * 2));
    
        shmctl (ID, IPC_RMID, NULL);

        sem_init(pristup, 1, 1);
        for (int i = 0; i < BROJ_OKVIRA; i++)
                okvir[i] = -1; 
        for (int i = 0; i < BROJ_OKVIRA; i++)
                sat[i] = 0; 
        *kazaljka = 0;

	printf("Pocetak programa\n");
        for (int i = 0; i < BROJ_PROCESA; i++) {
                if(fork() == 0)
                        generiraj_zahtjev(i);
        }
        for (int i = 0; i < BROJ_PROCESA; i++)
                wait(NULL);

        return 0;
}
