#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

unsigned int T_P = 0;
unsigned int prihvacam = 1;
int KON[6] = {-1, -1, -1, -1, -1, -1};
unsigned int K_Z[5] = {0, 0, 0, 0, 0};

void prekidna_rutina(int sig);
int pronadi_j();

/* program s prioritetom 1 */
void obradi_sigcont(int sig)
{
	if (prihvacam == 1) {
		prihvacam = 0;
		printf("\n");
		printf("Pohranjujem na stog reg[%d]\n", T_P);
		K_Z[0] = 1;
		printf("Primljen signal %d prioriteta 1!  ->  ", sig);
		printf("K_Z = [%d, %d, %d, %d, %d]\n", K_Z[0], K_Z[1], K_Z[2],
		K_Z[3], K_Z[4]);
		printf("\n");
		return prekidna_rutina(sig);
	} else {
		K_Z[0] = 1;
		return;
	}   
}

/* program s prioritetom 2 */
void obradi_sigint(int sig)
{
	if (prihvacam == 1) {
		prihvacam = 0;
		printf("\n");
		printf("Pohranjujem na stog reg[%d]\n", T_P);
		K_Z[1] = 1;
		printf("Primljen signal %d prioriteta 2!  ->  ", sig);
		printf("K_Z = [%d, %d, %d, %d, %d]\n", K_Z[0], K_Z[1], K_Z[2], 
		K_Z[3], K_Z[4]);
		printf("\n");
		return prekidna_rutina(sig);
	} else {
		K_Z[1] = 1; 
		return;
	}
}

/* program s prioritetom 3 */
void obradi_sigalrm(int sig)
{
	if (prihvacam == 1) {
		prihvacam = 0;
		printf("\n");
		printf("Pohranjujem na stog reg[%d]\n", T_P);
		K_Z[2] = 1;
		printf("Primljen signal %d prioriteta 3!  ->  ", sig);
		printf("K_Z = [%d, %d, %d, %d, %d]\n", K_Z[0], K_Z[1], K_Z[2], 
		K_Z[3], K_Z[4]);
		printf("\n");
		return prekidna_rutina(sig);
	} else {
		K_Z[2] = 1;
		return;
	}  
}

/* program s prioritetom 4 */
void obradi_sigusr1(int sig)
{
	if (prihvacam == 1) {
		prihvacam = 0;
		printf("\n");
		printf("Pohranjujem na stog reg[%d]\n", T_P);
		K_Z[3] = 1;
		printf("Primljen signal %d prioriteta 4!  ->  ", sig);
		printf("K_Z = [%d, %d, %d, %d, %d]\n", K_Z[0], K_Z[1], K_Z[2], 
		K_Z[3], K_Z[4]);
		printf("\n");
		return prekidna_rutina(sig);
	} else {
		K_Z[3] = 1;
		return;
	}
}

/* program s prioritetom 5 */
void obradi_sigterm(int sig)
{
	if (prihvacam == 1) {
		prihvacam = 0;
		printf("\n");
		printf("Pohranjujem na stog reg[%d]\n", T_P);
		K_Z[4] = 1;
		printf("Primljen signal %d prioriteta 5!  ->  ", sig);
		printf("K_Z = [%d, %d, %d, %d, %d]\n", K_Z[0], K_Z[1], K_Z[2], 
		K_Z[3], K_Z[4]);
		printf("\n");
		return prekidna_rutina(sig);
	} else {
		K_Z[4] = 1;
		return;
	}
}

void obradi_1(int sig)
{
	printf("\n");
	for (int i = 1; i <= 3; i++) {
		printf("Prekidni program 1: iteracija %d / 3\n", i);
		if (pronadi_j() != -1) 
			prekidna_rutina(sig);
		sleep(1);
		sleep(1);
		sleep(1);
	}
	printf("- - - - - Gotov prekid 1 - - - - -\n\n");
}

void obradi_2(int sig)
{
	printf("\n");
	for (int i = 1; i <= 5; i++) {
		printf("Prekidni program 2: iteracija %d / 5\n", i);
		if (pronadi_j() != -1) 
			prekidna_rutina(sig);
		sleep(1);
		sleep(1);
		sleep(1);
	}
	printf("- - - - - Gotov prekid 2 - - - - -\n\n");
}

void obradi_3(int sig)
{
	printf("\n");
	for (int i = 1; i <= 7; i++) {
		printf("Prekidni program 3: iteracija %d / 7\n", i);
		if (pronadi_j() != -1) 
			prekidna_rutina(sig);
		sleep(1);
		sleep(1);
		sleep(1);
	}
	printf("- - - - - Gotov prekid 3 - - - - -\n\n");
}

void obradi_4(int sig)
{
	printf("\n");
	for (int i = 1; i <= 10; i++) {
		printf("Prekidni program 4: iteracija %d / 10\n", i);
		if (pronadi_j() != -1)
			prekidna_rutina(sig);
		sleep(1);
		sleep(1);
	}
	printf("- - - - - Gotov prekid 4 - - - - -\n\n");
}

void obradi_5(int sig)
{
	printf("\n");
	for (int i = 1; i <= 15; i++) {
		printf("Prekidni program 5: iteracija %d / 15\n", i);
		if (pronadi_j() != -1) 
			prekidna_rutina(sig);
		sleep(1);
		sleep(1);
	}
	printf("- - - - - Gotov prekid 5 - - - - -\n\n");
}

int pronadi_j()
{
	unsigned int j; 
	unsigned int pronasao = 0;

	for (j = 5; j > (T_P); j--) {
		if (K_Z[j-1] == 1) {
			pronasao = 1;
			break;
		}
	}
	if (pronasao == 1) 
		return j;
	else 
		return -1;
}

void prekidna_rutina(int sig)
{
	int j;
	while((j = pronadi_j()) != -1) {
		K_Z[j - 1] = 0;
		KON[j] = T_P;
    
		printf("Pohranjujem kontekst sa sustavskog stoga i T_P -> KON[%d] = %d, reg[%d]\n", j, T_P, T_P);
		for (int i = 1; i <= 5; i++) {
			if (KON[i] == -1)
				printf("KON[%d] = --    ", i);
			else
				printf("KON[%d] = %d, reg[%d]    ", 
				i, KON[i], KON[i]);
		}
		printf("\n");
    
		T_P = j;

		/* omoguceno prekidanje */
		prihvacam = 1;

		if(j == 1) 
			obradi_1(sig);
		else if (j == 2) 
			obradi_2(sig);
		else if (j == 3) 
			obradi_3(sig);
		else if (j == 4)
			obradi_4(sig);
		else if (j == 5)
			obradi_5(sig);
	
		/* onemoguceno prekidanje */
		prihvacam = 0;
	
		printf("Vracam na sustavski stog i u T_P sadrzaj iz KON[%d]\n", j);
		T_P = KON[j];
		KON[j] = -1;
		for (int i = 1; i <= 5; i++) {
			if (KON[i] == -1)
				printf("KON[%d] = --    ", i);
			else
				printf("KON[%d] = %d, reg[%d]    ", 
				i, KON[i], KON[i]);
		}
		printf("\n");
		printf("\n");
	}
	prihvacam = 1;
	return;
}

int main()
{
	unsigned int gotovo = 0;
	unsigned int i = 1;
	struct sigaction action;
        
	/* maskiranje signala */
	action.sa_handler = obradi_sigint;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_NODEFER;
	sigaction(SIGINT, &action, NULL);
	        
	action.sa_handler = obradi_sigcont;
	sigaction(SIGCONT, &action, NULL);
	
	action.sa_handler = obradi_sigalrm;
	sigaction(SIGALRM, &action, NULL);
	
	action.sa_handler = obradi_sigusr1;
	sigaction(SIGUSR1, &action, NULL);
	
	action.sa_handler = obradi_sigterm;
	sigaction(SIGTERM, &action, NULL);


	printf("Program s PID=%ld krenuo s radom\n\n", (long) getpid());
	        
	/* glavni program */
	while (gotovo == 0) {
		prihvacam = 1;
		printf("Program: iteracija %d\n", i++);
		sleep(1);
		sleep(1);
	}
	        
	printf("Program s PID=%ld zavrsio s radom\n", (long) getpid());
	        
	return 0;
}
