#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <fcntl.h>
#include <termios.h>
 
#define SHMSZ     512
//実行するプログラムと同じパスにshm.datをいれておくか、サーバの起動時に共有メモリのためのファイルを作成し、サーバ切断の時にファイルを消す手段もある
int u8len(const char *tweetlen);        //UTF-8の場合はこちらを使用
int sjlen(const char *tweetlen);        //SHIFT_JISの場合はこちらを使用

int kbhit(void)                         //入力があるとこの関数に入る
{                                       //今回はサーバを停止させるのに使用
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

/*int adduser(int pid);

int adduser(int pid){
    int i;
    int user[2];

    for(i=0; i<2; i++){
        if(user[i]==0){
            user[i]=pid;
            return user[i];
        }
        else
            user[i+1]=pid;
            return user[i+1];
    }

}*/
 
int main()          //メイン関数
{
    int     shmid, pid=0, tweet1len=0, tweet2len=0;
    int     follow1flag=0, follow2flag=0;
    key_t   key;
    char   *data;
    char	 tmp[140];
    int     i, j;
    int     userpid[2]={0,0};
    char    *oripid;
    char    *tweet1, *tweet2, *user1tweet, *user2tweet;
    
     printf("クライアントを起動してください\n");
     
    //共有メモリ設定
    if ((key = ftok("shm.dat", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }
 
    
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
 
    
    data = (char *)shmat(shmid, (void *)0, 0);
    if (data == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    while(1) {          //dataの内容を常に監視

        if (kbhit()) {      //何か入力があればサーバ切断
            printf("\nサーバを切断します\n");
            sleep(1);
            break;
        }

    	if(strncmp(data, "pid,", 4)==0){        //pidを受け取ると
            oripid=strtok(data, ",");           //ユーザ登録
                                                //今はまだ2つ分しか作っていない
            while(oripid != NULL) {
               oripid = strtok(NULL, ",");
                if(oripid != NULL) {
                   pid=atoi(oripid);
    		       printf("受信したpid : %d\n", pid);
                }
            }
            
            if(userpid[0]==0){
                userpid[0]=pid;              //ユーザ1登録
                //cliにユーザ番号通知
                //printf("register%d\n", userpid[0]);
                strcpy(data, "1");              //ユーザ1登録
                sleep(1);
                }
                else{
                userpid[1]=pid;             //ユーザ2登録
                //cliにユーザ番号通知
                //printf("register%d\n", userpid[1]);
                strcpy(data, "2");            
                sleep(1);
            }
    	}
       
        else if (strncmp(data, "1,1,", 4) == 0) {   //ユーザ1のツイート処理
            printf("ユーザ1　ツイート処理\n");         
    		printf("ユーザ1 : %s\n",data);
            //strcpy(tmptweet, data);

            //char **words1 = malloc(sizeof(char*)*3);
            char *words1[3];                    //ツイート保存のためのポインタ配列

            words1[0]=strtok(data, ",");        //dataの1,1,を分割して
            for(i=1; i<3; i++)                  //ツイートの内容を受け取る
                words1[i]=strtok(NULL,",");

                for(i=0; i<3; i++)
                    printf("%s\n", words1[i]);


            printf("check : %s\n",words1[2]);   //words1[2]にツイート内容が入っている

            const char *tweetlen = words1[2];       //文字の長さ判断のため
            tweet1len = u8len(tweetlen);                    //UTF-8の場合はこちらを使用
	        printf("文字数 = %d\n", tweet1len);

            if(tweet1len > 140){
                //sprintf(data, "%s", "user1 : ツイートは140字以内で入力してください");
                sprintf(user1tweet, "%s", "user1 : ツイートの制限文字数(140文字)を超えました");
                //sleep(1);
                //printf("%s\n", data);
            }else{
                tweet1 = (char*)malloc(sizeof(char) * sizeof(words1));
                strcpy(tweet1, words1[2]);
                //sprintf(data, "%s%s", "user1 : ", tweet1);
                sprintf(user1tweet, "%s%s", "user1 : ", tweet1);
                //sleep(1);
                //printf("%s\n", data);
            }

            if(follow2flag==1){             //ツイート受け取り識別子を付加してdataに格納
                sprintf(data, "%s%s", "r1r2,", user1tweet);
                sleep(1);
            }else{
                sprintf(data, "%s%s", "r1,", user1tweet);
                sleep(1);
            }
        }
        
        else if (strncmp(data, "1,2,", 4) == 0) {       //フォロー処理
               if(follow1flag==0){
                follow1flag=1;                  //1が2をフォローしてなかったらフラグを1に
                printf("user 1 がuser 2 をフォロー\n");
                sprintf(data, "%s%s", "r1r2,", "user 1 がuser 2 をフォロー\n");
                sleep(1);
            }else{
                follow1flag=0;
                printf("user 1 がuser 2 のフォローを解除\n");
                sprintf(data, "%s%s", "r1r2,", "user 1 がuser 2 のフォローを解除\n");
                sleep(1);
            }
        }
        
        else if (strncmp(data, "1,9,", 4) == 0) {   //切断処理
            printf("ユーザ1　切断処理\n");
            userpid[0]=0;                          //pid情報を削除
            printf("ユーザ1　切断\n");
        }

        else if (strncmp(data, "2,1,", 4) == 0) {   //ユーザ1と同じように
            printf("ユーザ2　ツイート処理\n");
            printf("ユーザ2 : %s\n",data);
            
            //char **words1 = malloc(sizeof(char*)*3);
            char *words2[3];

            words2[0]=strtok(data, ",");
            for(i=1; i<3; i++)
                words2[i]=strtok(NULL,",");

                for(i=0; i<3; i++)
                    printf("%s\n", words2[i]);


            printf("check : %s\n",words2[2]);

            const char *tweetlen = words2[2];
            tweet2len = u8len(tweetlen);                  //UTF-8の場合はこちらを使用
	        printf("文字数 = %d\n", tweet2len);
            
            if(tweet2len > 140){
                //sprintf(data, "%s", "user1 : ツイートは140字以内で入力してください");
                sprintf(user2tweet, "%s", "user2 : ツイートの制限文字数(140文字)を超えました");
                //sleep(1);
                //printf("%s\n", data);
            }else{
                tweet2 = (char*)malloc(sizeof(char) * sizeof(words2));
                strcpy(tweet2, words2[2]);
                //sprintf(data, "%s%s", "user1 : ", tweet1);
                sprintf(user2tweet, "%s%s", "user2 : ", tweet2);
                //sleep(1);
                //printf("%s\n", data);
            }

            if(follow1flag==1){
                sprintf(data, "%s%s", "r1r2,", user2tweet);
                sleep(1);
            }else{
                sprintf(data, "%s%s", "r2,", user2tweet);
                sleep(1);
            }
        }
        
        else if (strncmp(data, "2,2,", 4) == 0) {   //ユーザ1と同じように
            printf("ユーザ2　フォロー処理\n");
                if(follow2flag==0){
                follow2flag=1;
                printf("user 2 がuser 1 をフォロー\n");
                sprintf(data, "%s%s", "r1r2,", "user 2 がuser 1 をフォロー\n");
                sleep(1);
            }else{
                follow2flag=0;
                printf("user 2 がuser 1 のフォローを解除\n");
                sprintf(data, "%s%s", "r1r2,", "user 2 がuser 1 のフォローを解除\n");
                sleep(1);
            }
        }
        
        else if (strncmp(data, "2,9,", 4) == 0) {   //ユーザ1と同じように
            printf("ユーザ2　切断処理\n");
            userpid[1]=0;
            printf("ユーザ2　切断\n");
        }

        else if (strcmp(data, "1") == 0) {          //登録中の時
            printf("ユーザ1登録中\n");
        }
        
        else if (strcmp(data, "2") == 0) {          //登録中の時
            printf("ユーザ2登録中\n");
        }
        sleep(2);
    }



    printf("サーバ切断\n");
 

    /* dettach the segment to data space */
    if (shmdt(data) == -1){
        perror("shmdt");
        exit(1);
    }
    /* Destroy the segment */
    if (shmctl(shmid, IPC_RMID, 0) == -1){
        perror("shmctl");
        exit(1);
    }
 
    return 0;
}


//文字数カウント関数(日本語も英語も1文字ずつでカウントさせる)
//UTF-8の場合はこちらを使用
int u8len(const char *tweetlen)
{
	int cnt = 0;
	while (*tweetlen != '\0') {
		if ((*tweetlen & 0xC0) != 0x80) {
			cnt++;
		}
		tweetlen++;
	}
	return cnt;
}


//SHIFT_JISの場合はこちらを使用
int sjlen(const char *tweetlen)
{
	int cnt = 0;
	while (*tweetlen != '\0'){
		cnt++;
		if ((*tweetlen & 0xE0) == 0x80 || (*tweetlen & 0xE0) == 0xE0) {
			tweetlen++;
		}
		tweetlen++;
	}
	return cnt;
}
