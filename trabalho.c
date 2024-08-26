#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "pthread.h"


#define NJ 11                       /* Nº de jogadores no time */ 
#define NB 7                        /* Nº de jogadores no banco de reservas */
#define ENERGIA 100                 /* Quantidade de energia dos jogadores */
struct Mensagem {
    int jogador;
    int energia;                  /* (0 ou 1) para o treinador determinar se há uma nova mensagem */
};

struct Mensagem mensagem; 

pthread_mutex_t cmp;                /* Lock para o jogador entrar e sair do campo */
pthread_mutex_t msg;                /* Lock para o jogador se comunicar com o treinador */
pthread_mutex_t ans;                /* Lock para o jogador receber a resposta do treinador */
pthread_cond_t trn;                 /* Variável de condição para acordar o treinador */
pthread_cond_t jgr;                 /* Variável de condição para fazer o jogador esperar a resposta do treinador */


int nCampo;                         /* Nº de jogadores em campo */
int newMsg = 0;                         /* (0 ou 1) Determina se há uma nova mensagem */
int ack = 0;                            /* (0 ou 1) Determina se o treinador recebeu a mensagem */

void treinador() {
    int jogador;
    int energia;


    while(1){
        /* Vê se há uma nova mensagem */
        pthread_mutex_lock(&msg);
        while(newMsg == 0) {
            pthread_cond_wait(&trn, &msg);
        }
         
        /* Atualiza os dados da mensagem */

        jogador = mensagem.jogador;
        energia = mensagem.energia;

        newMsg == 0;    
        pthread_mutex_unlock(&msg);

        if(energia < ENERGIA/5) {
            
            

        }

    }
}

void* jogador(void *arg) {
    int id = *((int*) arg);
    int jogando = 0;
    int energia = ENERGIA;
    /* Jogadores entram em campo */
    pthread_mutex_lock(&cmp);
        if (nCampo < 11) {
            nCampo ++;
            jogando = 1;
            printf("Jogador %d no campo. \n", id);
        } else {
            jogando = 0;
            printf("Jogador %d no banco. \n", id);
        }
    pthread_mutex_unlock(&cmp);
    
    while(1) {
        

        /* Se o jogador está em campo, vai jogar, caso contrário descansa */    
        if(jogando) {
            energia -= rand()%6 + 5;
            sleep(1);
        /* Se a ernegia está ficando baixa avisa o treinador */

            if(energia < ENERGIA/5) {
                printf("Jogador %d minha energia está baixa: %d. \n", id, energia);
                pthread_mutex_lock(&msg);           /* É possível implementar com try lock para os jogadores não pararem */
                mensagem.jogador = id;
                mensagem.energia = energia;
                
                newMsg = 1;
                pthread_cond_signal(&trn);                
                pthread_mutex_unlock(&msg);

                pthread_mutex_lock(&ans);
                while(ack == 0) {
                    pthread_cond_wait(&jgr, &ans);      /* Espera a resposta do treinador */
                }

                ack = 0;
                phread_mutex_unlock(&ans);
        
            }

        } else {
        /* Jogadores no banco descansam */
            
            
            
            if(energia <= ENERGIA-5) {
                energia += rand()%21 + 5;
                printf("%d Recuperando energia: %d \n", id, energia);
            } else {
                printf("%d Estou descansado. Vou esperar o treinador me chamar. \n", id)
                pthread_mutex_lock(&msgBanco);
                while(msgBanco == 0) {

                    pthread_cond_wait(&banco);

                }

                pthread_mutex_unlock(&msgBanco);
            }
            sleep(5);
        }

    }
    pthread_exit(0);
}

void main() {
    pthread_mutex_init(&cmp, NULL);
    pthread_mutex_init(&msg, NULL);
    pthread_mutex_init(&ans, NULL);
    pthread_cond_init(&trn, NULL);    
    pthread_cond_init(&jgr, NULL);
    pthread_t j[NJ+NB];
                                                                                    
    int i;  
    int *id;
    for (i=1; i <= (NJ+NB); i++) {                      /* Criação das threads dos jogadores */
        id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&j[i], NULL, jogador, (void *) (id));    
       
    }
    treinador();
}
