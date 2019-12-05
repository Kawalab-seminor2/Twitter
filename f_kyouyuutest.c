#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <string.h>

int main()
{
  int                shmid;       /* セグメントID */
  int                child_cnt;

  /* 共有メモリ・セグメントを新規作成 */
  if ((shmid = shmget(IPC_PRIVATE, 100, 0600)) == -1){
    perror('main : shmget ');
    exit(EXIT_FAILURE);
  }

  /* １つ目の子プロセスを生成 */
  if (fork() == 0) {
    /* 子プロセス */
    char      *shmaddr;

    printf('子プロセス１開始\n');

    /* 共有メモリ・セグメントをプロセスのアドレス空間に付加 */
    if ((shmaddr = shmat(shmid, NULL, 0)) == (void *)-1) {
      perror('子プロセス１ : shmat ');
      exit(EXIT_FAILURE);
    }

    /* 共有メモリ・セグメントに文字列をコピー */
    strcpy(shmaddr, 'Hello.\nBy.\n');

    /* 共有メモリ・セグメントを分離 */
    if (shmdt(shmaddr) == -1) {
      perror('子プロセス１ : shmdt ');
      exit(EXIT_FAILURE);
    }

    printf('子プロセス１終了\n');
    exit(EXIT_SUCCESS);
  }

  /* ２つ目の子プロセスを生成 */
  if (fork() == 0) {
    /* 子プロセス */
    char      *shmaddr;

    printf('子プロセス２開始\n');

    /* 待ち合わせ */
    sleep(1);

    /* 共有メモリ・セグメントをプロセスのアドレス空間に付加 */
    if ((shmaddr = shmat(shmid, NULL, SHM_RDONLY)) == (void *)-1) {
      perror('子プロセス２ : shmat ');
      exit(EXIT_FAILURE);
    }

    printf('子プロセス２：共有メモリの内容を表示します\n');
    /* 共有メモリ・セグメントの内容を表示 */
    printf('%s', shmaddr);

     /* 共有メモリ・セグメントを分離 */
    if (shmdt(shmaddr) == -1) {
      perror('子プロセス２ : shmdt ');
      exit(EXIT_FAILURE);
    }

    printf('子プロセス２終了\n');
    exit(EXIT_SUCCESS);
  }

  /* 親プロセス。子プロセスの終了を待つ */
  for (child_cnt = 0; child_cnt < 2; ++child_cnt) {
    wait(NULL);
  }

  /* 共有メモリ・セグメントを破棄 */
  if (shmctl(shmid, IPC_RMID, NULL) == -1){
    perror('main : shmctl ');
    exit(EXIT_FAILURE);
  }

  printf('親プロセス終了\n');
  return EXIT_SUCCESS;
}