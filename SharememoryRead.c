#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void main(){
    int id;
    char *adr;
    char prevtweet[140];

    if((id=shmget(IPC_PRIVATE,512,IPC_CREAT|0666))==-1){
        perror("shmget");
        exit(-1);
    }

    printf("共有メモリID=%d\n",id);

    if((adr=shmat(id,0,0))==-1){
        perror("shmat");
    }
    else{
        strcpy(adr,"Initial");
        strcpy(prevtweet,"Initial");
        printf("%s\n",adr);
        while(1){
            if(strcmp(prevtweet,adr)!=0){
                printf("%s\n",adr);
            }
            else if(strcmp(adr,"end")==0){
                break;
            }
            strcpy(prevtweet,adr);
            sleep(3);
        
        }

        if(shmdt(adr)==-1){
            perror("shmdt");
        }
    }
    if(shmctl(id,IPC_RMID,0)==-1){
        perror("shmctl");
    }
}