#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

void main(){
    int id;
    char *adr,A[10];
    if((id=shmget(IPC_PRIVATE,512,IPC_CREAT|0666))==-1){
        perror("shmat");
        exit(-1);
    }
    printf("共有メモリID=%d\n",id);
    if((adr=shmat(id,0,0))==-1){
        perror("shmat");
    }else{
       
        strcpy(adr,"Initial");
        strcpy(A,"Initial");
        while(1){ 
            
            if(strcmp(A,adr)!=0){
                 printf("%s\n",adr);
                if(strcmp(adr,"end")==0){
                break;
                }
                strcpy(A,adr);
            }
            
            
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