//2020 1/29最新版
//複数ユーザのフォローに対応
//リツイート対応、リプライ未実装
//ユーザ認証対応

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <fcntl.h>

#define SHMSZ     512
//実行するプログラムと同じパスにshm.datをいれておくか、サーバの起動時に共有メモリのためのファイルを作成し、サーバ切断の時にファイルを消す手段もある
int kbhit(void)     //何か押したら入る関数
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}


int main()  //メイン
{
    int     shmid, option=0, followno=0;
    key_t   key;
    char   *data, tweet[999];
    int     followflag[6]={0,0,0,0,0,0};
    char   sendpid[30];
    int     pid=0, userno=0;
    char    *check, rec[5], *rectwe;
    int     i;
    int     ID, no=0;
    char    PASS[16];
    int     passError=0, idError=0, login=0;
    int  t1count,t2count;
    char *log,*use;
    char *nal,*rep;

    pid=getpid();                   //pid取得
    printf("pid=%d\n",pid);         //pid表示

    //共有メモリの設定
    if ((key = ftok("shm.dat.txt", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }


    if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
        perror("shmget");
        exit(1);
    }


    data = (char *)shmat(shmid, (void *)0, 0);
    if (data == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    //pidをサーバへ送信(dataに格納)
    sprintf(sendpid, "%s,%d", "pid", pid);
    printf("送信pid : %s\n", sendpid);
    strcpy(data, sendpid);

    printf("認証中");           //この辺のsleepでサーバ側がdataを書き換える
        printf(".");sleep(1);
          printf(".");sleep(1);
            printf(".\n");sleep(1);
            while(1){
        printf("ユーザIDを入力してください(数字1桁) -> ");    //ID
        scanf("%d", &ID);
        printf("パスワードを入力してください(英数字) -> ");    //PASS
        scanf("%s", PASS);

        sprintf(data, "%s,%d,%s", "user", ID, PASS);
        //printf("%s\n",data);
        printf("認証中");           //この辺のsleepでサーバ側がdataを書き換える
         printf(".");sleep(1);
          printf(".");sleep(1);
           printf(".\n");sleep(1);

           if(strncmp(data, "11", 2)==0){
             printf("パスワードが異なります\n");
           }else if(strncmp(data, "21", 2)==0){
             printf("IDは数字１桁で入力してください\n");
           }else{
             printf("data = %s\n",data);
             userno=atoi(data);              //受け取った文字列を数値に
             sleep(1);
             printf("ユーザ%dでログインしました\n", userno);
             ID=0;
             PASS[0]='\0';
             break;
           }
            }
    printf("あなたはユーザ%dです\n", userno);       //ユーザ番号表示

    sprintf(rec, "%s%d", "r", userno);      //ツイート受け取り識別子作成
                                            //自分がユーザ1ならr1というdataを受け取る
    //printf("%s\n", rec);


 while(1){              //ここからがツイッターのメインの動作

    //自分のユーザ番号と選択肢の表示、この辺いじったらちゃんとツイート受け取り出来そう
    printf("user %d\n", userno);
    printf("1:tweet 2:follow 7: retweet 9:exit\n");    //↑ユーザがツイート表示状態になっていないとフォローしたユーザのツイートは自動で表示されない、プログラムの順番変えたりしたら行けるかも

    while(1){       //選択肢やツイート受信のためのループ

    if (kbhit()){           //入力がある時この関数に入る
    scanf("%d", &option);   //からの選択肢読み取り
    getchar();
    sleep(1);

    if(option==1 || option==2 || option==7 || option==9){    //選択肢の数値が来たらループ抜け出して
    	break;                                  //switch文へ
    }
    printf("選択肢の数値を入力して下さい\n");     //選択肢の数値以外が入力されたら表示、数字以外が入力されたら選択肢1に入ってしまう。(文字コードか何かの問題？)
    }

    if(strstr(data, rec)!=NULL){                //自分の受け取り識別子を含むdataを受け取ると表示
    //if (strncmp(data, "r1,", 3) == 0) {
		//printf("%s\n",data);
        //sleep(1);

        rectwe=strtok(data, ",");               //ここからdata分割

            while(rectwe != NULL) {
               rectwe = strtok(NULL, ",");
                if(rectwe != NULL) {
    		       printf("%s\n", rectwe);      //分割した結果を表示
                }                               //user 1 :～みたいな感じの表示
            }

    }
    }
    //ツイート受信したときにフォローしてるユーザのツイートを保存する

    switch(option){        //選択肢の結果による処理
    case 1:
    //tweet
    printf("Tweet : ");
    fgets(tweet, 999, stdin);   //ツイート入力
    //printf("%s\n",tweet);
    char *s = strchr(tweet, '\n');  //改行文字を終端文字に
    if(s != NULL) *s='\0';
    else while(1){
    	int c =getchar();
    	if(c == '\n' || c == EOF) break;
    	}
    sprintf(data, "%d%s%s", userno, ",1,", tweet);  //「i(自分の番号),1,tweet文」
    sleep(1);                                       //て感じでdataに格納
   	break;

    case 2:
    //follow
    printf("フォローするユーザを指定して下さい\nフォロー済みの場合はフォローを解除します\n->user ");
    scanf("%d", &followno);
    sprintf(data, "%d%s%d", userno, ",2,", followno);   //「i(自分の番号),2,follow対象」
    sleep(1);                                           //て感じでdataに格納
    if(followflag[followno-1]==0){
    printf("user %dをフォロー\n", followno);
    followflag[followno-1]=1;
    printf("%d\n",followno);
    }else if(followflag[followno-1]==1){
    printf("user %dのフォローを解除\n", followno);
    followflag[followno-1]=0;
    }
    break;

    case 7:
    sprintf(data, "%d%s", userno, ",sevensub,");
    sleep(1);
    if(0!=strncmp(data,"0",1)){
        printf("No data\n");
        break;
    }
    printf("ツイート番号  ユーザ名．ツイート内容\n%s",data);
    log = (char *)malloc(200);
    printf("何番のツイートを表示？");
    scanf("%s",log);
     sprintf(data, "%d%s%s", userno, ",7,",log);
    sleep(1);
    printf("これをリツイート→%s\n",data);
    free(log);
    use = (char *)malloc(200);
    strcpy(use,data);
     sprintf(data, "%d%s%s", userno, ",1,", use);  //「i(自分の番号),1,tweet文」
       sleep(1);                                       //て感じでdataに格納
    break;

       case 8:
           nal = (char *)malloc(140);
           rep = (char *)malloc(140);
           printf("どのようなツイートですか？");
           scanf("%s",nal);
           printf("replyの内容を");
           scanf("%s",rep);
       sprintf(data, "%d%s%s,%s", userno, ",8,",nal,rep);

           sleep(3);

           printf("reply→%s\n",data);
           free(nal);
           free(rep);
           break;

    case 9:
    sprintf(data, "%d%s", userno, ",9,");           //「i,9」をdataに格納
    sleep(1);                                       //そのあとbreakで抜け出す
    printf("Twitterを閉じます\n");
    	break;

 		}

			if(option==9)
            break;

}

    /* dettach the segment to data space */
    if (shmdt(data) == -1){
        perror("shmdt");
        exit(1);
    }

    return 0;
}
