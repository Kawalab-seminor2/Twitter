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
int id,a;
char *adr; 
char A[60],b[140],c[140];
char *x,*d;
char *C;
char D[60];
int count=0;
strcpy(c,"a");
C="userB";

while(1){
    printf("1:tweet 2:follow 9:exit -> ");
    scanf("%d", &a);
switch (a) {
	case 1:
		if(argc<=1){
            fprintf(stderr,"共有メモリIDを引数に指定すること。\n");
            exit(-1);
        }

        //id=atoi(argv[1]);

        //if((adr=(char *)shmat(id,0,0))==(void *)-1){
           // perror("shmat");
        //}else{
            while(1){
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

	case 2:
    id=atoi(argv[1]);

        if((adr=(char *)shmat(id,0,0))==(void *)-1){
            perror("shmat");
        }else{
    

        D[0]="userA";
        adr=&D[0];
        while(1){
            if(strcmp(&D[0],adr)!=0){
                printf("%s\n",adr);
                break;
            }else if(strcmp(adr,"end")==0){
                    break;
                }
            strcpy(A,adr); 
            sleep(3);

        }

        }	
		break;
	case 9:
		exit(1);
		break;
	default:
		printf("others\n");
		break;
}

}

return 0;
}