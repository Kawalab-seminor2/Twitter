#include<stdio.h>
#include<strings.h>
#include<rpc/rpc.h>

#define BUF_SIZE    8192
#define RLS_PROG    ((u_long)0x20000000)
#define RLS_VER     ((u_long)1)
#define RLS_PROC    ((u_long)1)

void main(argc,argv)
int argc;
char    *argv[];
{
char    buf[BUF_SIZE];

        if(argc<=2){
                fprintf(stderr,"rls hostname path\n");
                exit(-1);
        }
        strcpy(buf,argv[2]);
        call_rls(argv[1],buf);
        printf("%s\n",buf);
        exit(0);
}

int call_rls(host,buf)
char    *host;
char    *buf;
{
bool_t  xdr_rls();
enum clnt_stat  clnt_stat;

        clnt_stat=callrpc(host,RLS_PROG,RLS_VER,RLS_PROC,xdr_rls,buf,xdr_rls,buf);
        if(clnt_stat!=0){
            clnt_perrno(clnt_stat);
        }

        return(0);
}

bool_t xdr_rls(xdrs,objp)
XDR *xdrs;
char    *objp;
{
    return(xdr_string(xdrs,&objp,BUF_SIZE));
}