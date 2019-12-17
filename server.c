#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>




int main(){
    int id;
    char *adr,A[60],B[60],C[60],D[60],E[60],userA[140],userB[140];
    char *ptr,Q[10];
   
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
        strcpy(C,"userA");
        strcpy(D,"userB");
        printf("%s\n",adr);
        while(1){ printf("%s; \n",adr);
        
            printf("222共有メモリID=%d\n",id);
            if(strcmp(A,adr)!=0){printf("good0") ; 
                if(strcmp(C,adr)==0){
                    
                   printf("good1") ; 
                    printf("%s",userA);
                     printf("good2") ;  
                        strcpy(adr,userA);
                        printf("%s\n",adr);

                }else if(strcmp(D,adr)==0){
                    ptr=strtok(NULL,",");
                    
                        strcpy(adr,userB);
                        printf("%s\n",adr);

                }else{
                printf("good3") ; 
                strtok(adr,",");
                printf("%s \n",adr);

                if(strcmp(B,adr)==0){
                        adr=strtok(NULL,",");
                        printf("userA=%s\n",adr);
                        
                        strcpy(userA,adr);
                        
                        
                    
                        printf("%s\n",userA);
                        *adr=Q[0];
                        
                }else if(strcmp(E,adr)==0){
                        adr=strtok(NULL,",");
                        printf("userB=%s\n",adr);
                        
                        strcpy(userB,adr);
                        
                        
                    
                        printf("%s\n",userB);
                        *adr=Q[0];
                        
                }else{
                    printf("判定ミス");
                    
                }  *adr=Q[0]; 
                }
            }else if(strcmp(adr,"end")==0){
                            break;
                }

            
            strcpy(A,adr); 
            
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