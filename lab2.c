#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <termios.h>

#define MAXARGS 5
typedef struct {
        pid_t pid;
        char name[128];
} Proces;
struct sigaction prije;

char history[64][128];
int history_size = 0;
Proces ps_procesi[64];
int ps_count = 0;

void obradi_cd(char *path);
void obradi_history();
void obradi_ps();
void obradi_dogadjaj(int sig);
void obradi_kill(char *pid, char *signal);
pid_t pokreni_program(char *naredba[], int u_pozadini);


void dodaj_naredbu(char *naredba){
        strcpy(history[history_size], naredba);
        history_size++;
}
void obradi_usklicnik(char *naredba)
{
        int broj = atoi(naredba+1);
        if((broj - 1) < history_size){
                pid_t pid_novi;
                struct termios shell_term_settings;
                char nova[128];
                strcpy(nova,history[broj - 1]);
                char *argv[MAXARGS];
                int argc = 0;

                argv[argc] = strtok(nova, " \t\n");
                while (argv[argc] != NULL) {
                        argc++;
                        argv[argc] = strtok(NULL, " \t\n");
                }

                int background = 0;
                if(strcmp(argv[argc - 1], "&") == 0){
                        background = 1;
                        argv[argc - 1] = NULL;
                        argc--;
                }
                if(strncmp(argv[0], "cd", 2) == 0)
                        obradi_cd(nova + 3);
                else if(strncmp(argv[0], "!", 1) == 0){
                        obradi_usklicnik(argv[0]);
                        tcsetpgrp(STDIN_FILENO, getpgid(0));
                        tcsetattr(STDIN_FILENO, 0, &shell_term_settings);
                }else if(strncmp(argv[0], "history", 7) == 0)
                        obradi_history();
                else if(strncmp(argv[0], "ps", 2) == 0)
                        obradi_ps();
                else if(strncmp(argv[0], "kill", 4) == 0)
                        obradi_kill(argv[1], argv[2]);
                else {
                        pid_novi = pokreni_program(argv, background);
                        ps_procesi[ps_count].pid = pid_novi;
                        strcpy(ps_procesi[ps_count].name,argv[0]);
                        ps_count++;
                        if(!background) {
                                pid_t pid_zavrsio;
                                do {
                                        pid_zavrsio = waitpid(pid_novi, NULL, 0);
                                        if (pid_zavrsio > 0) {
                                                if (kill(pid_novi, 0) == -1) {
                                                        for(int i=0; i<ps_count; i++)
                                                                if(ps_procesi[i].pid == pid_novi) ps_procesi[i].pid = 0;
                                                        tcsetpgrp(STDIN_FILENO, getpgid(0));
                                                        tcsetattr(STDIN_FILENO, 0, &shell_term_settings);
                                                }
                                        }
                                } while(pid_zavrsio <= 0);
                        }
                }

        }
}

void obradi_cd(char *path)
{
        chdir(path);
}

void obradi_history()
{
        for (int i = 0; i < history_size; i++)
                printf("%d : %s", i + 1, history[i]);

}

void obradi_kill(char *pid, char *signal)
{
        int cpid = atoi(pid);
        int sig = atoi(signal);
        for(int i = 0; i < ps_count; i++) {
                if(ps_procesi[i].pid == cpid){
                        kill((pid_t)cpid, sig);
                        ps_procesi[i].pid = 0;
                }
        }
}

void obradi_ps()
{
        printf("aktivni procesi:\n");
        for(int i = 0; i < ps_count; i++) {
                if((int)ps_procesi[i].pid > 0)
                        printf("%d  %s\n", (int)ps_procesi[i].pid, ps_procesi[i].name);
        }
        printf("\n");
}

void obradi_dogadjaj(int sig)
{
        printf("\n[signal SIGINT] proces %d primio signal %d\n", (int) getpid(), sig);
}

void obradi_signal_zavrsio_neki_proces_dijete(int id)
{
        //ako je već dole pozvan waitpid, onda na ovaj signal waitpid ne daje informaciju (ponovo)
        pid_t pid_zavrsio = waitpid(-1, NULL, WNOHANG); //ne čeka
        if (pid_zavrsio > 0)
                if (kill(pid_zavrsio, 0) == -1) {
                        for (size_t i = 0; i < ps_count; i++) {
                                if(ps_procesi[i].pid == pid_zavrsio)
                                        ps_procesi[i].pid = 0;
                        }

                }
        if (pid_zavrsio > 0)
                printf("pozadinski proces [%d] zavrsio\n", (int)pid_zavrsio);
}

pid_t pokreni_program(char *naredba[], int u_pozadini)
{
        pid_t pid_novi;
        if ((pid_novi = fork()) == 0) {
                sigaction(SIGINT, &prije, NULL); //resetiraj signale
                setpgid(pid_novi, pid_novi); //stvori novu grupu za ovaj proces
                if (!u_pozadini) {
                        tcsetpgrp(STDIN_FILENO, getpgid(pid_novi)); //dodijeli terminal
                        execvp(naredba[0], naredba);
                        exit(1);
                }else {
                        execvp(naredba[0], naredba);
                        exit(1);
                }
        }

        return pid_novi; //roditelj samo dolazi do tuda
}

int main()
{
        struct sigaction act;
        pid_t pid_novi;
        size_t vel_buf = 128;
        char buffer[vel_buf];

        printf("glavni program [%d] krenuo s radom\n", (int) getpid());

         //postavi signale SIGINT i SIGCHLD
        act.sa_handler = obradi_dogadjaj;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGINT, &act, &prije);
        act.sa_handler = obradi_signal_zavrsio_neki_proces_dijete;
        sigaction(SIGCHLD, &act, NULL);
        act.sa_handler = SIG_IGN;
        sigaction(SIGTTOU, &act, NULL); //zbog tcsetpgrp

        struct termios shell_term_settings;
        tcgetattr(STDIN_FILENO, &shell_term_settings);
        tcsetpgrp(STDIN_FILENO, getpgid(0));


        do {
                char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                printf("%s$ ", cwd);

                if (fgets(buffer, vel_buf, stdin) != NULL) {
                        dodaj_naredbu(buffer);
                        char *argv[MAXARGS];
                        int argc = 0;
                        argv[argc] = strtok(buffer, " \t\n");
                        while (argv[argc] != NULL) {
                                argc++;
                                argv[argc] = strtok(NULL, " \t\n");
                        }
                        int background = 0;
                        if(strcmp(argv[argc - 1], "&") == 0) {
                                background = 1;
                                argv[argc - 1] = NULL;
                                argc--;
                        }
                        if(strncmp(argv[0], "cd", 2) == 0)
                                obradi_cd(buffer+3);
                        else if(strncmp(argv[0], "!", 1) == 0) {
                                obradi_usklicnik(argv[0]);
                                tcsetpgrp(STDIN_FILENO, getpgid(0));
                                tcsetattr(STDIN_FILENO, 0, &shell_term_settings);
                                continue;
                        }else if(strncmp(argv[0], "history", 7) == 0)
                                obradi_history();
                        else if(strncmp(argv[0], "ps", 2) == 0)
                                obradi_ps();
                        else if(strncmp(argv[0], "kill", 4) == 0)
                                obradi_kill(argv[1], argv[2]);
                        else {

                                pid_novi = pokreni_program(argv, background);
                                ps_procesi[ps_count].pid = pid_novi;
                                strcpy(ps_procesi[ps_count].name, argv[0]);
                                ps_count++;
                                if(!background) {
                                        pid_t pid_zavrsio;
                                        do {
                                                pid_zavrsio = waitpid(pid_novi, NULL, 0); //čekaj
                                                if (pid_zavrsio > 0) {
                                                        if (kill(pid_novi, 0) == -1) {
                                                                //nema ga više? ili samo mijenja stanje
                                                                for(int i=0; i<ps_count; i++) {
                                                                        if(ps_procesi[i].pid == pid_novi)
                                                                                ps_procesi[i].pid = 0;
                                                                }
                                                                //vraćam terminal ljusci
                                                                tcsetpgrp(STDIN_FILENO, getpgid(0));
                                                                tcsetattr(STDIN_FILENO, 0, &shell_term_settings);
                                                        }
                                                }
                                        } while(pid_zavrsio <= 0);

                                }
                        }
                }
        } while(strncmp(buffer, "exit", 4) != 0);

         //svim procesima posaljj SIGKILL
        for(int i = 0; i < ps_count; i++) {
                if((int)ps_procesi[i].pid > 0) {
                        kill(ps_procesi[i].pid, SIGKILL);
                        printf("proces [%d] ubijen\n", (int)ps_procesi[i].pid);
                }
        }

        return 0;
}