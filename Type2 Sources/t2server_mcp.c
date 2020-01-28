//2020 1/27最新版

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
	char tweet;
	char usernumber;
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
void getAllTweet(char *number, char *tweet); //ユーザの全ツイート保存
int getUserList(void); //ユーザ数数える関数
void mFollowFlag(int followusernumber, int followerusernumber,char *data); //フォローフラグ管理用関数　ポインタリストでユーザごとに管理

int getUserList(void){
	int number=0; //ユーザ数数える
	USER *user;
	user=uhead; //先頭からデータを見る
	while(user->next!=NULL) //次のデータが入ってないところまで数える
		number++;
	return number;
}

//void getAllTweet(char *number, char *tweet){ //ツイート情報を得る
//	TIMELINE *newTweet;
//	/*メモリ領域確保*/
//	newTweet=(TIMELINE *)malloc(sizeof(TIMELINE));
//	/*ツイートデータの格納*/
//	newTweet->tweet = tweet;
//	printf("%s\n",tweet);
//	newTweet->usernumber = number;
//	if((thead==NULL)&&(ttail==NULL)){
//		//リスト空のとき、新しいノードが先頭かつ末尾
//		thead=newTweet;
//		ttail=newTweet;
//	}
//	else{
//		ttail->next=newTweet;
//		ttail=ttail->next;
//	}
//	ttail->next=NULL;
//}

void mFollowFlag(int followusernumber, int followerusernumber,char *data){
	//フォローフラグ用構造体再定義
	//followerusernumberの位置までリストの場所移動
	//その場所のuser->followflagを1にする or 0にする
	int i;
  	FF *uFollowFlag;
 	USER *followuser;
  	uFollowFlag=fhead;  //先頭から見る
  	followuser=uhead;  //こちらも同様
  	for(i=1;i<=followusernumber;i++){
    	followuser=followuser->next;  //フォロー側ユーザのところまで飛ぶ
  	}
  	for(i=1;i<=followusernumber;i++){
    	uFollowFlag=uFollowFlag->next;  //フォローしたいユーザのフォローフラグまで飛ぶ
  	}
  	if(uFollowFlag->followflag==0){
    	uFollowFlag->followflag=1;
    	printf("user %d がuser %d をフォロー\n",followusernumber,followerusernumber);
     	sprintf(data, "r%dr%d%s", followusernumber,followerusernumber, "user 1 がuser 2 をフォロー\n");
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
    int     shmid, pid=0, tweet1len=0, tweet2len=0,tweet3len=0,tweet4len=0,tweet5len=0;
    int     follow1flag=0, follow2flag=0;
    key_t   key;
    char   *data;
    char	 tmp[140];
    int     i, j,k=1,number=0,ntmp; //iはforループ用、jはwhileループ用kはユーザナンバー決定用、numberはユーザの数
    char *u1info,*u2info,*u3info,*u4info,*u5info; //,より前はid,あとはパスワード
    char    *oripid;
    char    *tweet1, *tweet2,*tweet3,*tweet4,*tweet5,*user1tweet, *user2tweet,*user3tweet,*user4tweet,*user5tweet;
    char userno1[10],userno2[10]; //1は承認用、2はツイート用
    char alltweet[20][140];
    char *toptweet;
    char *u1t="1.",*u2t="2.",*u3t="3.",*u4t="4.",*u5t="5.";
    int u1ff[5]={0,0,0,0,0},u2ff[5]={0,0,0,0,0},u3ff[5]={0,0,0,0,0},u4ff[5]={0,0,0,0,0},u5ff[5]={0,0,0,0,0};
    int tweetnumber=0;
    USER *userdata,*utop;
    TIMELINE *tl;
    FF *fflag;
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
            if(uhead==NULL){
            	userdata=createNewUser(pid);
            	//utop=userdata;
            	//sleep(1);
            	//sleep(1);
            	//sleep(1);
            	//sleep(1);
            	//sleep(1);
            	//strcpy(u1info,data);
		/*if(u1id==NULL){
		  strcpy(u1id,data);
		  sleep(1);
		  strcpy(u1ps,data);
		  printf("ユーザ1登録\n");
		}*/
		/*else{
		  if(strcmp(u1id,data)!=0)
                  printf("idが違います\n");
		  else if(strcmp(u1ps,data)!=0)
                  printf("パスワードが違います\n");
		}*/
            	sprintf(data,"%s","1");
            	number++;
            	printf("ユーザ1登録\n");
            	sleep(1);
            }
            else{
            	while(userdata!=NULL){ //最初に1つ入ってないとダメでは?
            		if(userdata->next==NULL){
            			number++;
                		userdata=createNewUser(pid);              //ユーザ登録
                		if(number==2){
                			//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//strcpy(u2info,data);
                			strcpy(data,"2");
                			printf("ユーザ2登録\n");
            				sleep(1);
            				break;
                		}
                		else if(number==3){
                			//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//strcpy(u3info,data);
                			strcpy(data,"3");
                			printf("ユーザ3登録\n");
                			sleep(1);
                			break;
                		}
                		else if(number==4){
                			//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//strcpy(u4info,data);
                			strcpy(data,"4");
                			printf("ユーザ4登録\n");
                			sleep(1);
                			break;
                		}
                		else if(number==5){
                			//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//sleep(1);
            				//strcpy(u5info,data);
                			strcpy(data,"5");
                			printf("ユーザ5登録\n");
                			sleep(1);
                			break;
                		}
                		else{
                			userdata=utop;
                			while(1){
                				if(userdata->userpid==0){
                					userdata->userpid=1;
                					break;
                				}
                				else{
                					userdata=userdata->next;
                				}
                			}
                		}
                	}
                	else{
                		number++;
                	}
                }
            }
    	}
        else if (strncmp(data, "1,1,", 4) == 0) {   //ユーザ1のツイート処理
            printf("ユーザ1　ツイート処理\n");
    		printf("ユーザ1 : %s\n",data);
            //strcpy(tmptweet, data);

            //char **words1 = malloc(sizeof(char*)*3);
            char *words11[3];                    //ツイート保存のためのポインタ配列
	    

            words11[0]=strtok(data, ",");        //dataの1,1,を分割して
            for(i=1; i<3; i++)                  //ツイートの内容を受け取る
                words11[i]=strtok(NULL,",");

                for(i=0; i<3; i++)
                    printf("%s\n", words11[i]);


            printf("check : %s\n",words11[2]);   //words1[2]にツイート内容が入っている

            const char *tweetlen = words11[2];       //文字の長さ判断のため
            tweet1len = u8len(tweetlen);                    //UTF-8の場合はこちらを使用
	        printf("文字数 = %d\n", tweet1len);

            if(tweet1len > 140){
                //sprintf(data, "%s", "user1 : ツイートは140字以内で入力してください");
                sprintf(user1tweet, "%s", "user1 : ツイートの制限文字数(140文字)を超えました");
                //sleep(1);
                //printf("%s\n", data);
            }else{
                tweet1 = (char*)malloc(sizeof(char) * sizeof(words11));
				user1tweet = (char*)malloc(sizeof(char) * sizeof(words11));
            	strcpy(tweet1, words11[2]);
		
		
                sprintf(alltweet[tweetnumber],"%s%s",u1t,tweet1);
				for(i=0;i<=tweetnumber;i++)
                	printf("%s\n",alltweet[i]);
         
                tweetnumber++;

                //sprintf(data, "%s%s", "user1 : ", tweet1);
                
                sprintf(user1tweet, "%s%s", "user1 : ", tweet1);
                //sleep(1);
                //printf("%s\n", data);
		if(u2ff[0]==1){
			sprintf(data, "%s%s", "r1r2,", user1tweet);
                	sleep(1);
		}
		else if(u3ff[0]==1){
			sprintf(data, "%s%s", "r1r3,", user1tweet);
                	sleep(1);
		}
		else if(u4ff[0]==1){
			sprintf(data, "%s%s", "r1r4,", user1tweet);
                	sleep(1);
		}
		else if(u5ff[0]==1){
			sprintf(data, "%s%s", "r1r5,", user1tweet);
                	sleep(1);
		}
		else{
			sprintf(data, "%s%s", "r1,", user1tweet);
                	sleep(1);
		}
            }
        }

        else if (strncmp(data, "1,2,", 4) == 0) {       //フォロー処理
        		/*followflagはwhile文で回してどのユーザをフォローしたか判別*/
        		//誰のフォローフラグを1にするか指定する
   			char *words12[3]; //フォロー状態保存のためのポインタ配列
   			words12[0]=strtok(data, ","); //dataの1,1,を分割して
   			for(i=1; i<3; i++) //誰をフォローするかを把握
     		words12[i]=strtok(NULL,",");
   			j=atoi(words12[2]); //誰をフォローするかorリムーブするか
   			printf("check : %s\n",words12[2]); //words1[2]にツイートhigh内容が入っている
   			printf("%d\n",j);
			ntmp=j-1;
   			if(u1ff[ntmp]==0){
				u1ff[ntmp]=1;
				printf("ユーザ1がユーザ%dをフォロー\n",j);
			}
			else if(u1ff[ntmp]==1){
				u2ff[ntmp]=0;
				printf("ユーザ1がユーザ%dをリムーブ\n",j);
			}
        }

        else if (strncmp(data, "1,9,", 4) == 0) {   //切断処理
            printf("ユーザ1　切断処理\n");
            userdata->userpid=0;                          //pid情報を削除
            printf("ユーザ1　切断\n");
        }
        else if (strcmp(data, "1") == 0) {          //登録中の時
            printf("ユーザ1登録中\n");
        }
        else if (strncmp(data, "2,1,", 4) == 0) {   //ユーザ1のツイート処理
            printf("ユーザ2　ツイート処理\n");
    		printf("ユーザ2 : %s\n",data);
            //strcpy(tmptweet, data);

            //char **words1 = malloc(sizeof(char*)*3);
            char *words21[3];                    //ツイート保存のためのポインタ配列

            words21[0]=strtok(data, ",");        //dataの1,1,を分割して
            for(i=1; i<3; i++)                  //ツイートの内容を受け取る
                words21[i]=strtok(NULL,",");

                for(i=0; i<3; i++)
                    printf("%s\n", words21[i]);


            printf("check : %s\n",words21[2]);   //words1[2]にツイート内容が入っている

            const char *tweetlen = words21[2];       //文字の長さ判断のため
            tweet2len = u8len(tweetlen);                    //UTF-8の場合はこちらを使用
	        printf("文字数 = %d\n", tweet2len);

            if(tweet2len > 140){
                //sprintf(data, "%s", "user1 : ツイートは140字以内で入力してください");
                sprintf(user2tweet, "%s", "user2 : ツイートの制限文字数(140文字)を超えました");
                //sleep(1);
                //printf("%s\n", data);
            }else{
                tweet2 = (char*)malloc(sizeof(char) * sizeof(words21));
		user2tweet = (char*)malloc(sizeof(char)*sizeof(words21));
                strcpy(tweet2, words21[2]);

				sprintf(alltweet[tweetnumber],"%s%s",u2t,tweet2);
				for(i=0;i<=tweetnumber;i++)
                	printf("%s\n",alltweet[i]);
                tweetnumber++;

                //sprintf(data, "%s%s", "user1 : ", tweet1);
                sprintf(user2tweet, "%s%s", "user2 : ", tweet2);
                //sleep(1);
                //printf("%s\n", data);
		if(u1ff[1]==1){
			sprintf(data, "%s%s", "r1r2,", user2tweet);
                	sleep(1);
		}
		else if(u3ff[1]==1){
			sprintf(data, "%s%s", "r2r3,", user2tweet);
                	sleep(1);
		}
		else if(u4ff[1]==1){
			sprintf(data, "%s%s", "r2r4,", user2tweet);
                	sleep(1);
		}
		else if(u5ff[1]==1){
			sprintf(data, "%s%s", "r2r5,", user2tweet);
                	sleep(1);
		}
		else{
			sprintf(data, "%s%s", "r2,", user2tweet);
                	sleep(1);
		}
            }
        }

        else if (strncmp(data, "2,2,", 4) == 0) {       //フォロー処理
        		/*どのユーザをフォローしたか判別する必要あり→フォローしたユーザの番号を引数にポインタリスト探索行う
        		ヘッダからスタート→指定されたユーザ番号分だけリストを動かす、followflagに0を格納→止まったリストの位置のfollowflagを1にする*/
        		/*followflagはwhile文で回してどのユーザをフォローしたか判別*/
   			char *words22[3]; //フォロー状態保存のためのポインタ配列
   			words22[0]=strtok(data, ","); //dataの1,1,を分割して
   			for(i=1; i<3; i++) //誰をフォローするかを把握
     		words22[i]=strtok(NULL,",");
   			j=atoi(words22[2]); //誰をフォローするかorリムーブするか
   			printf("check : %s\n",words22[2]); //words1[2]にツイート内容が入っている
			ntmp=j-1;
   			if(u2ff[ntmp]==0){
				u2ff[ntmp]=1;
				printf("ユーザ2がユーザ%dをフォロー\n",j);			
			}
			else if(u2ff[ntmp]==1){
				u2ff[ntmp]=0;
				printf("ユーザ2がユーザ%dをリムーブ\n",j);
			}
        }

        else if (strncmp(data, "2,9,", 4) == 0) {   //切断処理
            printf("ユーザ2　切断処理\n");
            userdata->userpid=0;                          //pid情報を削除
            printf("ユーザ2　切断\n");
        }
        else if (strcmp(data, "2") == 0) {          //登録中の時
            printf("ユーザ2登録中\n");
        }
        else if (strncmp(data, "3,1,", 4) == 0) {   //ユーザ1のツイート処理
            printf("ユーザ3　ツイート処理\n");
    		printf("ユーザ3 : %s\n",data);
            //strcpy(tmptweet, data);

            //char **words1 = malloc(sizeof(char*)*3);
            char *words31[3];                    //ツイート保存のためのポインタ配列

            words31[0]=strtok(data, ",");        //dataの1,1,を分割して
            for(i=1; i<3; i++)                  //ツイートの内容を受け取る
                words31[i]=strtok(NULL,",");

                for(i=0; i<3; i++)
                    printf("%s\n", words31[i]);


            printf("check : %s\n",words31[2]);   //words1[2]にツイート内容が入っている

            const char *tweetlen = words31[2];       //文字の長さ判断のため
            tweet3len = u8len(tweetlen);                    //UTF-8の場合はこちらを使用
	        printf("文字数 = %d\n", tweet3len);

            if(tweet3len > 140){
                //sprintf(data, "%s", "user1 : ツイートは140字以内で入力してください");
                sprintf(user3tweet, "%s", "user3 : ツイートの制限文字数(140文字)を超えました");
                //sleep(1);
                //printf("%s\n", data);
            }else{
                tweet3 = (char*)malloc(sizeof(char) * sizeof(words31));
		user3tweet = (char*)malloc(sizeof(char)*sizeof(words31));
                strcpy(tweet3, words31[2]);
                
				sprintf(alltweet[tweetnumber],"%s%s",u3t,tweet3);
				for(i=0;i<=tweetnumber;i++)
                	printf("%s\n",alltweet[i]);
                tweetnumber++;

                //sprintf(data, "%s%s", "user1 : ", tweet1);
                sprintf(user3tweet, "%s%s", "user3 : ", tweet3);
                //sleep(1);
                //printf("%s\n", data);
		if(u1ff[2]==1){
			sprintf(data, "%s%s", "r1r3,", user3tweet);
                	sleep(1);
		}
		else if(u2ff[2]==1){
			sprintf(data, "%s%s", "r2r3,", user3tweet);
                	sleep(1);
		}
		else if(u4ff[2]==1){
			sprintf(data, "%s%s", "r3r4,", user3tweet);
                	sleep(1);
		}
		else if(u5ff[2]==1){
			sprintf(data, "%s%s", "r3r5,", user3tweet);
                	sleep(1);
		}
		else{
			sprintf(data, "%s%s", "r3,", user3tweet);
                	sleep(1);
		}
            }
        }

        else if (strncmp(data, "3,2,", 4) == 0) {       //フォロー処理
        		/*どのユーザをフォローしたか判別する必要あり→フォローしたユーザの番号を引数にポインタリスト探索行う
        		ヘッダからスタート→指定されたユーザ番号分だけリストを動かす、followflagに0を格納→止まったリストの位置のfollowflagを1にする*/
        		/*followflagはwhile文で回してどのユーザをフォローしたか判別*/
   			char *words32[3]; //フォロー状態保存のためのポインタ配列
   			words32[0]=strtok(data, ","); //dataの1,1,を分割して
   			for(i=1; i<3; i++) //誰をフォローするかを把握
     			words32[i]=strtok(NULL,",");
   			j=atoi(words32[2]); //誰をフォローするかorリムーブするか
   			printf("check : %s\n",words32[2]); //words1[2]にツイート内容が入っている
			ntmp=j-1;
   			if(u3ff[ntmp]==0){
				u3ff[ntmp]=1;
				printf("ユーザ3がユーザ%dをフォロー\n",j);
			}
			else if(u3ff[ntmp]==1){
				u3ff[ntmp]=0;
				printf("ユーザ3がユーザ%dをリムーブ\n",j);
			}
        }

        else if (strncmp(data, "3,9,", 4) == 0) {   //切断処理
            printf("ユーザ3　切断処理\n");
            userdata->userpid=0;                          //pid情報を削除
            printf("ユーザ3　切断\n");
        }
        else if (strcmp(data, "3") == 0) {          //登録中の時
            printf("ユーザ3登録中\n");
        }
        else if (strncmp(data, "4,1,", 4) == 0) {   //ユーザ1のツイート処理
            printf("ユーザ4　ツイート処理\n");
    		printf("ユーザ4 : %s\n",data);
            //strcpy(tmptweet, data);

            //char **words1 = malloc(sizeof(char*)*3);
            char *words41[3];                    //ツイート保存のためのポインタ配列

            words41[0]=strtok(data, ",");        //dataの1,1,を分割して
            for(i=1; i<3; i++)                  //ツイートの内容を受け取る
                words41[i]=strtok(NULL,",");

                for(i=0; i<3; i++)
                    printf("%s\n", words41[i]);


            printf("check : %s\n",words41[2]);   //words1[2]にツイート内容が入っている

            const char *tweetlen = words41[2];       //文字の長さ判断のため
            tweet4len = u8len(tweetlen);                    //UTF-8の場合はこちらを使用
	        printf("文字数 = %d\n", tweet4len);

            if(tweet1len > 140){
                //sprintf(data, "%s", "user1 : ツイートは140字以内で入力してください");
                sprintf(user4tweet, "%s", "user1 : ツイートの制限文字数(140文字)を超えました");
                //sleep(1);
                //printf("%s\n", data);
            }else{
                tweet4 = (char*)malloc(sizeof(char) * sizeof(words41));
		user4tweet=(char*)malloc(sizeof(char)*sizeof(words41));
                strcpy(tweet4, words41[2]);
				sprintf(alltweet[tweetnumber],"%s%s",u4t,tweet4);
				for(i=0;i<=tweetnumber;i++)
                	printf("%s\n",alltweet[i]);
                tweetnumber++;

                //sprintf(data, "%s%s", "user1 : ", tweet1);
                sprintf(user4tweet, "%s%s", "user4 : ", tweet4);
                //sleep(1);
                //printf("%s\n", data);
		if(u1ff[3]==1){
			sprintf(data, "%s%s", "r1r4,", user4tweet);
                	sleep(1);
		}
		else if(u2ff[3]==1){
			sprintf(data, "%s%s", "r2r4,", user4tweet);
                	sleep(1);
		}
		else if(u3ff[3]==1){
			sprintf(data, "%s%s", "r3r4,", user4tweet);
                	sleep(1);
		}
		else if(u5ff[3]==1){
			sprintf(data, "%s%s", "r4r5,", user4tweet);
                	sleep(1);
		}
		else{
			sprintf(data, "%s%s", "r4,", user4tweet);
                	sleep(1);
		}
            }
        }

        else if (strncmp(data, "4,2,", 4) == 0) {       //フォロー処理
        		/*どのユーザをフォローしたか判別する必要あり→フォローしたユーザの番号を引数にポインタリスト探索行う
        		ヘッダからスタート→指定されたユーザ番号分だけリストを動かす、followflagに0を格納→止まったリストの位置のfollowflagを1にする*/
        		/*followflagはwhile文で回してどのユーザをフォローしたか判別*/
   			char *words42[3]; //フォロー状態保存のためのポインタ配列
   			words42[0]=strtok(data, ","); //dataの1,1,を分割して
   			for(i=1; i<3; i++) //誰をフォローするかを把握
     		words42[i]=strtok(NULL,",");
   			j=atoi(words42[2]); //誰をフォローするかorリムーブするか
   			printf("check : %s\n",words42[2]); //words1[2]にツイート内容が入っている
			ntmp=j-1;
   			if(u4ff[ntmp]==0){
				u4ff[ntmp]=1;
				printf("ユーザ4がユーザ%dをフォロー\n",j);
			}
			else if(u4ff[ntmp]==1){
				u4ff[ntmp]=0;
				printf("ユーザ4がユーザ%dをリムーブ\n",j);
			}
        }

        else if (strncmp(data, "4,9,", 4) == 0) {   //切断処理
            printf("ユーザ4　切断処理\n");
            userdata->userpid=0;                          //pid情報を削除
            printf("ユーザ4　切断\n");
        }
        else if (strcmp(data, "4") == 0) {          //登録中の時
            printf("ユーザ4登録中\n");
        }
        else if (strncmp(data, "5,1,", 4) == 0) {   //ユーザ1のツイート処理
            printf("ユーザ5　ツイート処理\n");
    		printf("ユーザ5 : %s\n",data);
            //strcpy(tmptweet, data);

            //char **words1 = malloc(sizeof(char*)*3);
            char *words51[3];                    //ツイート保存のためのポインタ配列

            words51[0]=strtok(data, ",");        //dataの1,1,を分割して
            for(i=1; i<3; i++)                  //ツイートの内容を受け取る
                words51[i]=strtok(NULL,",");

                for(i=0; i<3; i++)
                    printf("%s\n", words51[i]);


            printf("check : %s\n",words51[2]);   //words1[2]にツイート内容が入っている

            const char *tweetlen = words51[2];       //文字の長さ判断のため
            tweet5len = u8len(tweetlen);                    //UTF-8の場合はこちらを使用
	        printf("文字数 = %d\n", tweet5len);

            if(tweet5len > 140){
                //sprintf(data, "%s", "user1 : ツイートは140字以内で入力してください");
                sprintf(user5tweet, "%s", "user1 : ツイートの制限文字数(140文字)を超えました");
                //sleep(1);
                //printf("%s\n", data);
            }else{
                tweet5 = (char*)malloc(sizeof(char) * sizeof(words51));
		user5tweet=(char*)malloc(sizeof(char)*sizeof(words51));
                strcpy(tweet5, words51[2]);
				sprintf(alltweet[tweetnumber],"%s%s",u5t,tweet5);
				for(i=0;i<=tweetnumber;i++)
                	printf("%s\n",alltweet[i]);
                tweetnumber++;

                //sprintf(data, "%s%s", "user1 : ", tweet1);
                sprintf(user5tweet, "%s%s", "user5 : ", tweet5);
                //sleep(1);
                //printf("%s\n", data);
		if(u1ff[4]==1){
			sprintf(data, "%s%s", "r1r5,", user5tweet);
                	sleep(1);
		}
		else if(u2ff[4]==1){
			sprintf(data, "%s%s", "r2r5,", user5tweet);
                	sleep(1);
		}
		else if(u3ff[4]==1){
			sprintf(data, "%s%s", "r3r5,", user5tweet);
                	sleep(1);
		}
		else if(u4ff[4]==1){
			sprintf(data, "%s%s", "r4r5,", user5tweet);
                	sleep(1);
		}
		else{
			sprintf(data, "%s%s", "r5,", user5tweet);
                	sleep(1);
		}
            }
        }

        else if (strncmp(data, "5,2,", 4) == 0) {       //フォロー処理
        		/*どのユーザをフォローしたか判別する必要あり→フォローしたユーザの番号を引数にポインタリスト探索行う
        		ヘッダからスタート→指定されたユーザ番号分だけリストを動かす、followflagに0を格納→止まったリストの位置のfollowflagを1にする*/
        		/*followflagはwhile文で回してどのユーザをフォローしたか判別*/
   			char *words52[3]; //フォロー状態保存のためのポインタ配列
   			words52[0]=strtok(data, ","); //dataの1,1,を分割して
   			for(i=1; i<3; i++) //誰をフォローするかを把握
     		words52[i]=strtok(NULL,",");
   			j=atoi(words52[2]); //誰をフォローするかorリムーブするか
   			printf("check : %s\n",words52[2]); //words1[2]にツイート内容が入っている
			ntmp=j-1;
   			if(u5ff[ntmp]==0){
				u5ff[ntmp]=1;
				printf("ユーザ5がユーザ%dをフォロー\n",j);
			}
			else if(u5ff[ntmp]==1){
				u5ff[ntmp]=0;
				printf("ユーザ5がユーザ%dをリムーブ\n",j);
			}
        }

        else if (strncmp(data, "5,9,", 4) == 0) {   //切断処理
            printf("ユーザ5　切断処理\n");
            userdata->userpid=0;                          //pid情報を削除
            printf("ユーザ5　切断\n");
        }
        else if (strcmp(data, "5") == 0) {          //登録中の時
            printf("ユーザ5登録中\n");
        }
        
        //七番
        else if(strncmp(data,"1,7,",4)==0){
            char *log,*log2,*kesin;
            int high;
            int b=0,o;
            char yoritweet[20][100]={};         
  
            log=strtok(data,"1,7,");
           high=(int)(*log)-48;
           
           for(i=0;i<tweetnumber;i++){
           if(u1ff[0]==0||u1ff[0]==1){ 
           if(strncmp(alltweet[i], "1.", 2)==0){
           kesin=strtok(alltweet[i],"1.");        
      sprintf(yoritweet[b],"%s",kesin);
           b++;
           }
           }       
           if(u1ff[1]==1){     
           if(strncmp(alltweet[i], "2.", 2)==0){
            kesin=strtok(alltweet[i],"2.");
             sprintf(yoritweet[b],"%s",kesin);
           b++;
           }
           }
           if(u1ff[2]==1){     
           if(strncmp(alltweet[i], "3.", 2)==0){
            kesin=strtok(alltweet[i],"3.");
             sprintf(yoritweet[b],"%s",kesin);
           b++;
           }
           }
           if(u1ff[3]==1){     
           if(strncmp(alltweet[i], "4.", 2)==0){
            kesin=strtok(alltweet[i],"4.");
             sprintf(yoritweet[b],"%s",kesin);
           b++;
           }
           }
           if(u1ff[4]==1){     
           if(strncmp(alltweet[i], "5.", 2)==0){
            kesin=strtok(alltweet[i],"5.");
             sprintf(yoritweet[b],"%s",kesin);
           b++;
           }
           }
          }
           sprintf(data,"%s",yoritweet[b-high]);
            sleep(1);
        }
        
        sleep(1);
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
