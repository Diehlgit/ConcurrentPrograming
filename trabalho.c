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
pthread_mutex_t msgBanco;           /* Lock para o jogador do banco se comunicar com o treinador */
pthread_cond_t  banco;              /* Variável de condição para o jogador do banco esperar o treinador chamá-lo */
pthread_cond_t trn;                 /* Variável de condição para acordar o treinador */
pthread_cond_t jgr;                 /* Variável de condição para fazer o jogador esperar a resposta do treinador */


int nCampo;                         /* Nº de jogadores em campo */
int newMsg = 1;                         /* (0 ou 1) Determina se há uma nova mensagem */
int ack = 1;                            /* (0 ou 1) Determina se o treinador recebeu a mensagem */
int jgrBanco = 1;                       /* (0 ou 1) Determina se o jogador do banco deve esperar */

void treinador() {
    int jogador;
    int energia;


    while(1){
        /* Vê se há uma nova mensagem */
        pthread_mutex_lock(&msg);
        while(newMsg) {
            printf("Treinador: Vou esperar um jogador ficar cansado.\n");
            pthread_cond_wait(&trn, &msg);
        }
         
        /* Atualiza os dados da mensagem */

        jogador = mensagem.jogador;
        energia = mensagem.energia;

        newMsg = 1;    
        pthread_mutex_unlock(&msg);
                    
        if(energia <= ENERGIA/5) {
            printf("Treinador: Vou tirar um jogador do banco.\n");

            pthread_mutex_lock(&msgBanco);

            jgrBanco = 0;
            pthread_cond_signal(&banco);

            pthread_mutex_unlock(&msgBanco);
        
        }
        
    }
}

void* jogador(void *arg) {
    int id = *((int*) arg);
    int jogando = 0;
    int energia = ENERGIA;
    /* Jogadores entram em campo */
    pthread_mutex_lock(&cmp);
        if (nCampo < NJ) {
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
            energia -= rand()%21;
            sleep(2);
        /* Se a ernegia está ficando baixa avisa o treinador */

            if(energia < ENERGIA/5) {
                printf("Jogador %d: Minha energia está baixa: %d. \n", id, energia);
                pthread_mutex_lock(&msg);           /* É possível implementar com try lock para os jogadores não pararem */
                mensagem.jogador = id;
                mensagem.energia = energia;
                
                newMsg = 0;
                pthread_cond_signal(&trn);                
                pthread_mutex_unlock(&msg);

                printf("Jogador %d: Entrando no banco.\n", id);

                jogando = 0;

            }

        } else {
        /* Jogadores no banco descansam */
            
            if(energia <= ENERGIA-25) {
                energia += rand()%21 + 5;
                sleep(2);
            } else {
                printf("Jogador %d: Estou descansado: %d. Vou esperar o treinador me chamar. \n", id, energia);
                pthread_mutex_lock(&msgBanco);
                
                while(jgrBanco) {

                    pthread_cond_wait(&banco, &msgBanco);

                }

                jgrBanco = 1;

                pthread_mutex_unlock(&msgBanco);

                printf("Jogador %d: Entrando em campo.\n", id);

                jogando = 1;
            }
        }

    }
    pthread_exit(0);
}

void main() {
    pthread_mutex_init(&cmp, NULL);
    pthread_mutex_init(&msg, NULL);
    pthread_mutex_init(&ans, NULL);
    pthread_mutex_init(&msgBanco, NULL);
    pthread_cond_init(&banco, NULL);
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
