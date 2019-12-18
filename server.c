#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>




int main(){
    int id;
    char *adr,A[60],B[60],BB[60],C[60],D[60],E[60],EE[60],userA[140],userB[140];
    char *ptr,Q[10];
    
    if((id=shmget(IPC_PRIVATE,512,IPC_CREAT|0666))==-1){
        perror("shmat");
        exit(-1);
    }
    printf("共有メモリID=%d\n",id);
    if((adr=(char *)shmat(id,0,0))==(void *)-1){
        perror("shmat");
    }else{
        printf("111共有メモリID=%d\n",id);
        //strcpy(adr,"Initial");
        //strcpy(A,"Initial");
        strcpy(B,"a");
        strcpy(BB,"aa");
        strcpy(EE,"bb");
        strcpy(E,"b");
        strcpy(C,"userA");
        strcpy(D,"userB");
        printf("%s\n",adr);
        while(1){ 
            //strtok(adr,",");
            printf("1 %s; \n",adr);
            printf("222共有メモリID=%d\n",id);
            //if(strcmp(A,adr)!=0){
                printf("good0") ; 
                if(strcmp(C,adr)==0){
                    
                    printf("good1") ; 
                    printf("%s",userA);
                    printf("good2") ;  
                        strcpy(adr,userA);
                        sleep(3);
                        shmdt(adr);
                        if((adr=(char *)shmat(id,0,0))==(void *)-1){
                            perror("shmat");
                        }
                        //*adr=userA[0];
                        printf("%s\n",adr);
                        //memset((void *) *adr, 0,256);
                }else if(strcmp(D,adr)==0){
                    ptr=strtok(NULL,",");
                    
                        strcpy(adr,userB);
                        printf("%s\n",adr);

                }else{
                printf("good3") ; 
                strtok(adr,",");
                printf("%s \n",adr);

                if((strcmp(B,adr)==0)||(strcmp(BB,adr)==0)){
                        adr=strtok(NULL,",");
                        printf("userA=%s\n",adr);
                        
                        strcpy(userA,adr);
                        
                        sleep(1);
                    shmdt(adr);
if((adr=(char *)shmat(id,0,0))==(void *)-1){
            perror("shmat");
        }
                        printf("%s\n",userA);
                        
                        *adr=Q[0];
                        shmdt(adr);
if((adr=(char *)shmat(id,0,0))==(void *)-1){
            perror("shmat");
        }
                        //memset((void *) *adr, 0,256);
                }else if((strcmp(E,adr)==0)||(strcmp(EE,adr)==0)){
                        adr=strtok(NULL,",");
                        printf("userB=%s\n",adr);
                        
                        strcpy(userB,adr);
                        
                        
                    
                        printf("%s\n",userB);
                        *adr=Q[0];
                        //memset((void *) *adr, 0,256);
                        
                }else{
                    printf("判定ミス\n");
                    
                }  *adr=Q[0]; 
                //memset((void *) *adr, 0,256);
                }
            //}else 
            if(strcmp(adr,"end")==0){
                            break;
                }

            

            sleep(1);
            
            
            //strcpy(A,adr);
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