#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(argc,argv)
int argc;
char *argv[];
{
int id,a;//変数定義
char *adr; 
char A[60],b[140],c[140];
char *x,*d,*e ;
char *C,Q[10];
int count=0;
strcpy(c,"a");
if(argc<=1){
            fprintf(stderr,"共有メモリIDを引数に指定すること。\n");
            exit(-1);
        }

while(1){
    
    printf("1:tweet 2:follow 9:exit -> ");//メニュー表示
    scanf("%d", &a);

switch (a) {
	case 1://メニューの１を選んだ時
		

        //id=atoi(argv[1]);

        //if((adr=(char *)shmat(id,0,0))==(void *)-1){
           // perror("shmat");
        //}else{
            while(1){//ヘッダー識別子とツイート文を共有メモリに書き込む処理
                id=atoi(argv[1]);
                if((adr=(char *)shmat(id,0,0))==(void *)-1){
                    perror("shmat");
                }
                d="a";
                printf("%s:",d);
                scanf("%s", b);
                strcat(adr,d);
                if(count>=0){
                    strcat(adr,d);
                }
                strcat(adr,","); printf("1");
                strcat(adr,b);
                printf("1");
                //shmdt(adr);
                count ++;
                if(strcmp(adr,"end")==0){
                    break;
                }
                break;
            }

            if(shmdt(adr)==-1){
                perror("shmdt");
            }
            break;
       // }
		break;
        case 2://メニューの2を選んだ時
            //if(strcmp(A,adr)!=0){
            id=atoi(argv[1]);//フォロー要求をサーバに送り、サーバから最新ツイートが返ってきたらそれを表示する処理

        if((adr=(char *)shmat(id,0,0))==(void *)-1){
            perror("shmat");
        }else{
    
        printf("1 %s\n",adr);
        //*adr=Q[0]; 
        printf("2 %s\n",adr);
        e="userB"; 
        strcpy(adr,e);
            //if(strcmp(A,adr)!=0){
        printf("3%s\n",adr);
        sleep(1);
        shmdt(adr);
if((adr=(char *)shmat(id,0,0))==(void *)-1){
            perror("shmat");
        }

        while(1){
            if(strcmp(e,adr)!=0){
                printf("%s\n",adr);
                break;
            }else if(strcmp(adr,"end")==0){
                    break;
                }
            //strcpy(A,adr); 
            sleep(3);
            //if(strcmp(A,adr)!=0){

        }
        if(shmdt(adr)==-1){
                perror("shmdt");
            }

        }	
		break;
	case 9://メニューの9を選んだ時
            //if(strcmp(A,adr)!=0){
		exit(1);
		break;
	default:
		printf("others\n");
		break;
}

}

return 0;
}