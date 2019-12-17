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
char A[60],b[140],c[140];
char *x,*d;
char *C;
int count=0;
strcpy(c,"b");
C="userA,";

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
                d="b";
                printf("%s:",d);
                scanf("%s", b);
                strcat(adr,d);
                if(count>=1){
                    strcat(adr,d);
                }
                strcat(adr,",");
                strcat(adr,b);
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
        }
		break;

	case 2:
    id=atoi(argv[1]);

        if((adr=shmat(id,0,0))==-1){
            perror("shmat");
        }else{
    

    
        strcat(adr, C);
        while(1){
            strtok(adr,",");
            printf("%s \n",adr);

            if(strcmp("a",adr)==0){
                adr=strtok(NULL,",");
                printf("userA=%s\n",adr);
            
                break;
            }else if(strcmp(adr,"end")==0){
                    break;
                }
            
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