#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>

#define SEMAPHORE_CLIENT_NOTIFICATION "/semaphore_for_client_notification"
#define SEMAPHORE_SERVER_NOTIFICATION "/semaphore_for_server_notification"
extern int errno;
using namespace std;

int main(){

  sem_t *sem_client,*sem_server; //идентификатор семафора
  pid_t pid;                     //идентификатор процесса
  int channel;                   //идентификатор канала

  char input[]="Ввод пользователя";

 /*Если канал с таким именем существует, удалим его*/
 unlink("/tmp/fifo0001.1");

 /*Создаем канал*/
 if((mkfifo("/tmp/fifo0001.1", O_RDWR)) == -1)
 {
   fprintf(stderr, "Невозможно создать fifo\n");
   exit(0);
 }

 /* Открываем уже существующий семафор для уведомления сервера */
 if ( (sem_server = sem_open(SEMAPHORE_SERVER_NOTIFICATION, 0)) == SEM_FAILED ) {
       perror("sem_open");
       return 1;
   }
   /* Создаем семафор для уведомления клиента */
   if ( (sem_client = sem_open(SEMAPHORE_CLIENT_NOTIFICATION, O_CREAT, 0777, 0)) == SEM_FAILED ) {
         perror("sem_open");
         return 1;
     }

 pid = fork();   // создаем клиентский процесс и перенаправляем его в другое окно

 if(pid == -1)
  { perror("fork"); exit(0);}

 if(pid > 0) {  // блок кода для серверного процесса

/*Открываем fifo для записи*/
 if((channel=open("/tmp/fifo0001.1", O_WRONLY)) == - 1)
 {
   fprintf(stderr, "Невозможно открыть fifo\n");
   exit(0);
 }

 while(1)
 {  // бесконечный цикл ожидания серверным процессом ввода следующей строки и т.д.

 cout << "Пожалуйста,введи строку для клиента" << endl;
 cin >> input;         // полученная от пользователя строка
 sem_post(sem_client);  // клиентский процесс уведомляется о том, что серверный процесс готов начать передачу данных (синхронизация)
 printf("Сервер готов.\n");
 write(channel,input,strlen(input)) ; // серверный процесс передает полученную от пользователя строку клиентскому процессу, используя канал
 printf("Ждем уведомления от клиента об успешном получении строки.\n");
     if (sem_wait(sem_server) < 0 ) //
         perror("sem_wait");
 printf("Уведомление от клиента об успешном получении строки получено.\n");
  }
            }
  if (pid == 0)      //блок кода для клиентского процесса
  {
    if(execlp("/usr/bin/xterm","xterm","-e","./client",NULL) == -1) // перенаправляем клиентский процесс и перенаправляем его в другое окно xterm
        perror("execlp");
  }

 close(channel);
 return 0;
}
