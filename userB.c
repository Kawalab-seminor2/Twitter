#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
int main(argc,argv)
int argc;
char *argv[];
{
int id,a;
char *adr; 
char A[60],b[140];
char *x;
char *C;
x="b";
C="A";

while(1){
    printf("1:tweet 2:follow 9:exit -> ");
    scanf("%d", &a);
switch (a) {
	case 1:
		if(argc<=1){
            fprintf(stderr,"共有メモリIDを引数に指定すること。\n");
            exit(-1);
        }

        id=atoi(argv[1]);

        if((adr=shmat(id,0,0))==-1){
            perror("shmat");
        }else{
            while(1){
                x="b";
                printf("%s:",x);
                scanf("%s", b);
                strcpy(adr,x);
                strcat(adr,",");
                strcat(adr,b);
                if(strcmp(adr,"end")==0){
                    break;
                }
                break;
            }

            if(shmdt(adr)==-1){
                perror("shmdt");
            }
            break;
        }
		break;

	case 2:
    id=atoi(argv[1]);

        if((adr=shmat(id,0,0))==-1){
            perror("shmat");
        }else{
    
        strcpy(adr,C);
        while(1){
            if(strcmp(C,adr)!=0){
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