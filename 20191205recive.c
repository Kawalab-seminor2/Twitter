#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include <string.h>
#include <stdlib.h>
enum user{
    ユーザA,
    reユーザA,


};




int main(){
    int id;
    char *adr,A[60],B[60],C[60],D[60],E[60],userA[140],userB[140];
    char *ptr;
    *userA=NULL;
    if((id=shmget(IPC_PRIVATE,512,IPC_CREAT|0666))==-1){
        perror("shmat");
        exit(-1);
    }
    printf("共有メモリID=%d\n",id);
    if((adr=shmat(id,0,0))==-1){
        perror("shmat");
    }else{
printf("111共有メモリID=%d\n",id);
        //strcpy(adr,"Initial");
        //strcpy(A,"Initial");
        strcpy(B,"a");
        strcpy(E,"b");
        strcpy(C,"A");
        strcpy(D,"B");
        printf("%s\n",adr);
        while(1){ printf("%s \n",adr);
printf("222共有メモリID=%d\n",id);
            if((strcmp(A,adr)!=0)&&(adr!=NULL)){
                
                strtok(adr,",");
                printf("%s \n",adr);

                if(strcmp(B,adr)==0){
                        adr=strtok(NULL,",");
                        printf("userA=%s\n",adr);
                        if(adr!=NULL){
                        strcpy(userA,adr);
                        }
                        
                    
                        printf("%s\n",adr);
                        
                        
                }else if(strcmp(C,adr)==0){
                    ptr=strtok(NULL,",");
                    if(*userA==NULL){
                        strcpy(adr,"ユーザAのツイートはありません");
                    }
                        strcpy(adr,userA);
                        printf("%s\n",adr);

                }else if(strcmp(D,adr)==0){
                    ptr=strtok(NULL,",");
                    if(*userB==NULL){
                        strcpy(adr,"ユーザBのツイートはありません");
                    }
                        strcpy(adr,userB);
                        printf("%s\n",adr);
                }else if(strcmp(E,adr)==0){
                        adr=strtok(NULL,",");
                        printf("userB=%s\n",adr);
                        if(adr!=NULL){
                        strcpy(userB,adr);
                        }
                        
                    
                        printf("%s\n",adr);
                        
                        
                }else{
                    printf("判定ミス");
                    
                }  
                
            }else if(strcmp(adr,"end")==0){
                            break;
                }
            strcpy(A,adr); 
            *adr=NULL;
            sleep(3);

        }
            if(shmdt(adr)==-1){
                perror("shmdt");
            }
            
    }
        if(shmctl(id,IPC_RMID,0)==-1){
            perror("shmctl");
        }
    
    return 0;
}