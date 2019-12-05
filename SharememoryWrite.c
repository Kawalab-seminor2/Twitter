#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


void main(argc,argv)
int   argc;
char *argv[];
{
int   id;
char  *adr;

    if(argc<=1){
        fprintf(stderr,"共有メモリIDを引数に指定すること。\n");
        exit(-1);
    }

    id=atoi(argv[1]);

    if((adr=shmat(id,0,0))==-1){
        perror("shmat");
    }
    else{
        while(1){
            gets(adr);
            if(strcmp(adr,"end")==0){
                break;
            }
        }
        if(shmdt(adr)==-1){
            perror("shmdt");
        }
    }
}