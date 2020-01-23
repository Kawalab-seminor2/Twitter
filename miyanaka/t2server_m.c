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

typedef struct followflag{//フォローフラグ用構造体
  int followflag;
  struct followflag *next;
}FF;

typedef struct user{ //ユーザ情報の管理用構造体
	int userpid;
	//ユーザリストごとにポインタリストの作成
	FF followflag;
	struct user *next;
}USER;

typedef struct alltimeline{ //全体ツイート保存用
	char *tweet[999];
	int usernumber;
	struct alltimeline *next;
}TIMELINE; //usernumberで表示するユーザの選別を行えばよろしいのでは

FF *fhead=NULL;
FF *ftail=NULL;
USER *uhead=NULL;
USER *utail=NULL;
TIMELINE *thead=NULL;
TIMELINE *ttail=NULL;

FF *createFollowFlag(); //フォローフラグ管理用
USER *createNewUser(int pid); //ユーザデータのノード作成
TIMELINE *getAllTweet(int number, char tweet); //ユーザの全ツイート保存
int getUserList(void); //ユーザ数数える関数
void mFollowFlag(int followusernumber, int followerusernumber,char *data); //フォローフラグ管理用関数　ポインタリストでユーザごとに管理

int getUserList(void){
	int number; //ユーザ数数える
	USER *user;
	user=uhead; //先頭からデータを見る
	while(1){
		if(user->next!=NULL){ //作成済みの領域=その分ユーザがいる
			number++;
		}
		else{
			break;
		}
	}
	return number;
}

void mFollowFlag(int followusernumber, int followerusernumber,char *data){
	//フォローフラグ用構造体再定義
	//followerusernumberの位置までリストの場所移動
	//その場所のuser->followflagを1にする or 0にする
	int i;
  	FF uFollowFlag;
 	USER followuser;
  	uFollowFlag=fhead;  //先頭から見る
  	followuser=uhead;  //こちらも同様
  	for(i=1;i<=followusernumber;i++){
    	followuser=followuser->next;  //フォロー側ユーザのところまで飛ぶ
  	}
  	for(i=1;i<=followusernumber;i++){
    	uFollowFlag=uFollowFlag->next;  //フォローしたいユーザのフォローフラグまで飛ぶ
  	}
  	if(followflag->followflag==0){
    	uFollowFlag->followflag=1;
    	printf("user %d がuser %d をフォロー\n",followusernumber,followerusernumber);
     	sprintf(data, "%s%s", "r1r2,", "user 1 がuser 2 をフォロー\n");
  	}
  	else{
    	uFollowFlag->followflag=0;
    	printf("user %d がuser %d のフォローを解\n",followusernumber,followerusernumber);
    	sprintf(data, "%s%s", "r1r2,", "user 1 がuser 2 のフォローを解除\n");
  	}
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
    int     i, j=1,k=1,number=0; //iはforループ用、jはwhileループ用kはユーザナンバー決定用、numberはユーザの数
    int     userpid[2]={0,0};
    char    *oripid;
    char    *tweet1, *tweet2, *user1tweet, *user2tweet;
    char userno; //char('0' + i)変換用
    USER userdata;
    TIMELINE tl;
    userdata=uhead;
    tl=thead;
    //それぞれヘッドから見る必要あり
    
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
            
            //headとtailが同じ値のとき最初のやつ追加
            if(uhead==utail){
            	userdata=createNewUser(pid);
            	strcpy(data,k);
            	sleep(1);
            }
            while(userdata!=NULL){ //最初に1つ入ってないとダメでは?
            	if(userdata->userpid==0){
                	userdata=createNewUser(pid);              //ユーザ登録
                	//cliにユーザ番号通知
                	//printf("register%d\n", userpid[0]);
                	userno=char('0' + k);
                	strcpy(data, userno);              //ユーザk登録
                	sleep(1);
                	break;
                }
                else{
                	k++;
                }
            }
            k=1;
    	}
    	/*ユーザ数数える関数必要
    	whileで作った領域まで回して変数numberをインクリメント*/
    	number=getUserList();
       for(i=1;i<=number;i++){//ツイート、フォロー、通信切断処理を行うforループ
       	userno=char('0'+i);
        if (strncmp(data, userno,"1,", 4) == 0) {   //ユーザ1のツイート処理
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
                
                /*tweetを構造体に格納する(全体用)*/
                /*tweetを構造体に格納(個人&共有用)*/
                tl=getAllTweet(i,tweet1);
                
                //sprintf(data, "%s%s", "user1 : ", tweet1);
                sprintf(user1tweet, "%s%s", "user1 : ", tweet1);
                //sleep(1);
                //printf("%s\n", data);
            }
			//whileで誰をフォローしているかの確認を行う
			while(userdata!=NULL){
            	if(userdata->followflag==1){             //ツイート受け取り識別子を付加してdataに格納
            	/*構造体内のfollowflagに従ってTL表示を指定*/
                	sprintf(data, "%s%s", "r1r2,", user1tweet);
                	sleep(1);
            	}else{
                	sprintf(data, "%s%s", "r1,", user1tweet);
                	sleep(1);
            	}
            }
        }
        
        else if (strncmp(data, userno,"2,", 4) == 0) {       //フォロー処理
        		/*どのユーザをフォローしたか判別する必要あり→フォローしたユーザの番号を引数にポインタリスト探索行う
        		ヘッダからスタート→指定されたユーザ番号分だけリストを動かす、followflagに0を格納→止まったリストの位置のfollowflagを1にする*/
        		/*followflagはwhile文で回してどのユーザをフォローしたか判別*/
   			char *words2[3]; //フォロー状態保存のためのポインタ配列
   			words2[0]=strtok(data, ","); //dataの1,1,を分割して
   			for(i=1; i<3; i++) //誰をフォローするかを把握
     		words2[i]=strtok(NULL,",");
   			j=atoi(words2[2]); //誰をフォローするかorリムーブするか
   			printf("check : %s\n",words1[2]); //words1[2]にツイート内容が入っている
   			mFollowFlag(i,j);
        }
        
        else if (strncmp(data, userno,"9,", 4) == 0) {   //切断処理
            printf("ユーザ%d　切断処理\n",i);
            userdata->userpid=0;                          //pid情報を削除
            printf("ユーザ%d　切断\n",i);
        }
        else if (strcmp(data, userno) == 0) {          //登録中の時
            printf("ユーザ%d登録中\n",i);
        }
        sleep(1);
       }
      }
/*
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
        else if (strcmp(data, "2") == 0) {          //登録中の時
            printf("ユーザ2登録中\n");
        }
        sleep(2);
    }
*/
        
        
        



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
FF *createFollowFlag(){
  FF *pFollowFlag; //メモリ領域の確保
  pFollowFlag=(FF *)malloc(sizeof(FF));
  pFollowFlag->followflag=0;
  if((fhead==NULL)&&(ftail==NULL)){
    //リスト空のとき、新しいノードが先頭かつ末尾
    fhead=pFollowFlag;
    ftail=pFollowFlag;
  }
  else{
    ftail->next=pFollowFlag;
    ftail=ftail->next;
  }
  ftail->next=NULL;
  return pFollowFlag;
}

USER *createNewUser(int pid){ //ノード作成用
	USER *pNewUser;
	/*メモリ領域確保*/
	pNewUser=(USER *)malloc(sizeof(USER));
	/*各追加データ格納*/
	pNewUser->userpid=pid;
	if((uhead==NULL)&&(utail==NULL)){
		//リスト空のとき、新しいノードが先頭かつ末尾
		uhead=pNewUser;
		utail=pNewUser;
	}
	else{
		utail->next=pNewUser;
		utail=utail->next;
	}
	utail->next=NULL;
	return pNewUser;
}

TIMELINE *getAllTweet(int number, char tweet){ //ツイート情報を得る
	TIMELINE *newTweet;
	/*メモリ領域確保*/
	newTweet=(TIMELINE *)malloc(sizeof(TIMELINE));
	/*ツイートデータの格納*/
	newTweet->usernumber=number;
	strcpy(newTweet->tweet,tweet);
	if((thead==NULL)&&(ttail==NULL)){
		//リスト空のとき、新しいノードが先頭かつ末尾
		thead=newTweet;
		ttail=newTweet;
	}
	else{
		ttail->next=newTweet;
		ttail=ttail->next;
	}
	ttail->next=NULL;
	return newTweet;
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
