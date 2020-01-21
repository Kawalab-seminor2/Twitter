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
    int     shmid, option=1, followno=0;
    key_t   key;
    char   *data, tweet[999];
    int     followflag=0;
    char   sendpid[30];
    int     pid=0, userno=0;
    char    *check, rec[5], *rectwe;
    int     i;
    
    pid=getpid();                   //pid取得
    printf("pid=%d\n",pid);         //pid表示
 
    //共有メモリの設定
    if ((key = ftok("shm.dat", 'R')) == -1) {
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
    userno=atoi(data);                      //受け取った文字列を数値に
    sleep(1);
    printf("あなたはユーザ%dです\n", userno);       //ユーザ番号表示

    sprintf(rec, "%s%d", "r", userno);      //ツイート受け取り識別子作成
                                            //自分がユーザ1ならr1というdataを受け取る
    //printf("%s\n", rec);
 

 while(1){              //ここからがツイッターのメインの動作
    
    //自分のユーザ番号と選択肢の表示、この辺いじったらちゃんとツイート受け取り出来そう
    printf("user %d\n", userno);
    printf("1:tweet 2:follow 9:exit -> ");    //↑ユーザがツイート表示状態になっていないとフォローしたユーザのツイートは自動で表示されない、プログラムの順番変えたりしたら行けるかも

    while(1){       //選択肢やツイート受信のためのループ
      
      if (kbhit()){           //入力がある時この関数に入る
	scanf("%d", &option);   //からの選択肢読み取り
	getchar();
	sleep(1);
	
	if(option==1 || option==2 || option==9){    //選択肢の数値が来たらループ抜け出して
	  break;                                  //switch文へ
	}
	printf("選択肢の数値を入力して下さい\n");     //選択肢の数値以外が入力されたら表示、数字以外が入力されたら選択肢1に入ってしまう。(文字コードか何かの問題？)
	printf("\n1:tweet 2:follow 9:exit -> ");
      }
      
      if(strstr(data, rec)!=NULL){                //自分の受け取り識別子を含むdataを受け取ると表示
	//if (strncmp(data, "r1,", 3) == 0) {    
	//printf("%s\n",data);
        //sleep(1);
        
        rectwe=strtok(data, ",");               //ここからdata分割
        
	while(rectwe != NULL) {
	  rectwe = strtok(NULL, ",");
	  if(rectwe != NULL) {
	    printf("\n%s\n", rectwe);      //分割した結果を表示
	  }                               //user 1 :～みたいな感じの表示
	}
      }
    }
    
    switch(option){        //選択肢の結果による処理
    case 1:
      //tweet
      option = 0;
      printf("Tweet : ");
      fgets(tweet, 999, stdin);   //ツイート入力
      //printf("%s\n",tweet);
      char *s = strchr(tweet, '\n');  //改行文字を終端文字に
      if(s != NULL) *s='\0';
      else{
	while(1){
	  char c =getchar();
	  if(c == '\n' || c == EOF) break;
      	}
      }
      sprintf(data, "%d,1,%s", userno, tweet);  //「i(自分の番号),1,tweet文」
      sleep(1);                                       //て感じでdataに格納
      break;
      
    case 2:
      //follow
      option = 0;
      printf("フォローするユーザを指定して下さい(今回は1or2)\nフォロー済みの場合はフォローを解除します\n->user ");
      scanf("%d", &followno);
      sprintf(data, "%d,2,%d", userno, followno);   //「i(自分の番号),2,follow対象」
      sleep(1);                                           //て感じでdataに格納

      if(userno == followno){
	printf("自分はフォローできません\n");
      }
      else if(followflag==0){
	printf("user %dをフォロー\n", followno);
	followflag=1;
      }else if(followflag==1){
	printf("user %dのフォローを解除\n", followno);
	followflag=0;
      }
      break;
      
    case 9:
      option = 0;
      sprintf(data, "%d%s", userno, ",9,");           //「i,9」をdataに格納
      sleep(1);                                       //そのあとbreakで抜け出す
      printf("Twitterを閉じます\n");
      exit(1);
      
    }    
 }
 
 /* dettach the segment to data space */
 if (shmdt(data) == -1){
   perror("shmdt");
   exit(1);
 }
 
 return 0;
}
