#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"


#define NJ 2                       /* Nº de jogadores no time */ 
#define NB 1                        /* Nº de jogadores no banco de reservas */
#define ENERGIA 100                 /* Quantidade de energia dos jogadores */


pthread_mutex_t msg;                /* Lock para jogadores do banco e do campo se comunicarem */
pthread_cond_t banco;               /* Variável de condição para os jogadores do banco esperarem ser chamados */

int esperando = 1;                  /* (0 ou 1) define se os jogadores do banco esperam ou não */
int descansado = 0;                 /* Nº de jogadores no banco descansados */

void* jogador(void *arg) {
    int id = *((int*) arg);
    int jogando = 0;
    int energia = ENERGIA;
    /* Jogadores entram em campo */
        if (id <= NJ) {
            jogando = 1;
            printf("Jogador %d no campo. \n", id);
        } else {
            jogando = 0;
            printf("Jogador %d no banco. \n", id);
        }
    
    while(1) {
        

        /* Se o jogador está em campo, vai jogar, caso contrário descansa */    
        if(jogando) {
            energia -= rand()%21;
            sleep(2);

        /* Se a ernegia está ficando baixa tentar trocar com alguém do banco */

            if(energia < ENERGIA/5) {
                printf("Jogador %d: Minha energia está baixa. \n", id);


                pthread_mutex_lock(&msg);     /* Pega o Lock para falar com o banco */
                    if(descansado > 0) {
                        /* Jogador cansado vai para o banco */
                        printf("Jogador %d: Indo pro banco.\n", id);
                        jogando = 0;
                        descansado -= 1;    

                        /* Chama um jogador do banco para substituí-lo */
                        esperando = 0;              
                        pthread_cond_signal(&banco);  
                    }
                pthread_mutex_unlock(&msg);
            
            }


        } else {
        /* Jogadores no banco descansam */
            
            if(energia < ENERGIA-35) {
                energia += rand()%21 + 15;
                sleep(2);
            } else {
                printf("Jogador %d: Estou descansado: %d. Vou esperar alguém me chamar. \n", id, energia);

                pthread_mutex_lock(&msg);
                    descansado += 1;
                    while(esperando) {
                        pthread_cond_wait(&banco, &msg);
                    }
                    esperando = 1;
                pthread_mutex_unlock(&msg);

                /* Jogador do banco vai para o campo */
                printf("Jogador %d: Entrando em campo.\n", id);

                jogando = 1;
            }
        }

    }
    pthread_exit(0);
}

void main() {
    pthread_mutex_init(&msg, NULL);
    pthread_cond_init(&banco, NULL);
    pthread_t j[NJ+NB];
                                                                                    
    int i;  
    int *id;
    for (i=1; i <= (NJ+NB); i++) {                      /* Criação das threads dos jogadores */
        id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&j[i], NULL, jogador, (void *) (id));    
    }

    for (i = 1; i <= (NJ+NB); i++) {                    /* Espera as threads dos jogadores terminarem */
        pthread_join(j[i], NULL);
    }

    pthread_mutex_destroy(&msg);
    pthread_cond_destroy(&banco);
}
