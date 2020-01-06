#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//必要なライブラリの導入



int main(){
    int id;
    char *adr,A[60],B[60],BB[60],C[60],D[60],E[60],EE[60],userA[140],userB[140];
    char *ptr,Q[10];//変数定義、変数名も少し整理する必要あり。使ってない変数もあるかもしれない。
    
    if((id=shmget(IPC_PRIVATE,512,IPC_CREAT|0666))==-1){//共有メモリの確保。共有メモリとは何か、各自で調べてください。
        perror("shmat");
        exit(-1);
    }
    printf("共有メモリID=%d\n",id);//共有メモリIDの表示。usrA.cとusrB.cを実行する際はこのIDを引数にしてください。
    if((adr=(char *)shmat(id,0,0))==(void *)-1){//共有メモリをプロセスに付与。
        perror("shmat");
    }else{
        printf("111共有メモリID=%d\n",id);
        
        strcpy(B,"a");
        strcpy(BB,"aa");
        strcpy(EE,"bb");
        strcpy(E,"b");
        strcpy(C,"userA");
        strcpy(D,"userB");
        printf("%s\n",adr);//変数の初期化。
        while(1){ 
            
            printf("1 %s; \n",adr);
            printf("222共有メモリID=%d\n",id);
            
                printf("good0") ; 
                if(strcmp(C,adr)==0){//BにAがフォローされた時
                    
                    printf("good1") ; 
                    printf("%s",userA);
                    
                        strcpy(adr,userA);//共有メモリをAの最新ツイートに切り替える。
                        sleep(3);
                        shmdt(adr);//共有メモリをプロセスから切り離し
                        if((adr=(char *)shmat(id,0,0))==(void *)-1){//共有メモリをプロセスに付与。
                            perror("shmat");
                        }
                        //*adr=userA[0];
                        printf("%s\n",adr);
                        //memset((void *) *adr, 0,256);
                }else if(strcmp(D,adr)==0){//AにBがフォローされた時
                    
                    printf("%s",userB);
                    printf("good2") ;  
                        strcpy(adr,userB);
                        sleep(3);
                        shmdt(adr);//共有メモリをプロセスから切り離し
                        if((adr=(char *)shmat(id,0,0))==(void *)-1){
                            perror("shmat");
                        }
                        //*adr=userA[0];
                        printf("%s\n",adr);
                        //memset((void *) *adr, 0,256);
                }else{//ツイートされた時
                printf("good3") ; 
                strtok(adr,",");//ヘッダーの識別子を保管しておく
                printf("%s \n",adr);

                if((strcmp(B,adr)==0)||(strcmp(BB,adr)==0)){//Aがツイートした時
                    adr=strtok(NULL,",");
                    printf("userA=%s\n",adr);
                    
                    strcpy(userA,adr);//最新ツイートを入れるに保存
                    
                    sleep(1);
                    shmdt(adr);//共有メモリをプロセスから切り離し
                    if((adr=(char *)shmat(id,0,0))==(void *)-1){//共有メモリをプロセスに付与。
                        perror("shmat");
                    }
                    printf("%s\n",userA);
                    
                    *adr=Q[0];//共有メモリを初期化
                    shmdt(adr);//共有メモリをプロセスから切り離し
                    if((adr=(char *)shmat(id,0,0))==(void *)-1){//共有メモリをプロセスに付与。
                        perror("shmat");
                    }
                    //memset((void *) *adr, 0,256);
                }else if((strcmp(E,adr)==0)||(strcmp(EE,adr)==0)){//Bがツイートした時
                    adr=strtok(NULL,",");
                    printf("userB=%s\n",adr);
                    
                    strcpy(userB,adr);
                    
                    sleep(1);
                    shmdt(adr);//共有メモリをプロセスから切り離し
                    if((adr=(char *)shmat(id,0,0))==(void *)-1){//共有メモリをプロセスに付与。
                        perror("shmat");
                    }
                    printf("%s\n",userB);
                    
                    *adr=Q[0];//共有メモリを初期化
                    shmdt(adr);//共有メモリをプロセスから切り離し
                    if((adr=(char *)shmat(id,0,0))==(void *)-1){//共有メモリをプロセスに付与。
                        perror("shmat");
                    }
                        //memset((void *) *adr, 0,256);
                }else{//default
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