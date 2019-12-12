#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

void main(argc,argv)
int argc;
char *argv[];
{
int id,a;
char *adr ,*b;
char x=1;
x=(char)getpid();


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
                printf("%s:",x);
                
                scanf("%s", b);

                *adr=strcat(x,b);
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
		printf("a = 3\n");
		break;
	case 9:
		exit(1);
		break;
	default:
		printf("others\n");
		break;
}

}
}