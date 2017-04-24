#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>   

#define MAX_SIZE 2048
#define MAX_CONNECTION 100

#define max(x, y) (((x) > (y)) ? (x) : (y))

void(*signal(int signo,void(*func)(int)))(int);
typedef void Sigfunc(int);

Sigfunc *signal(int signo, Sigfunc *func) {
  struct sigaction act,oact;
  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if(signo == SIGALRM){
    #ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
    #endif	
  }else{
    #ifdef SA_RESTART
    act.sa_flags |= SA_RESTART;
    #endif	
  }
  if (sigaction(signo, &act, &oact)<0) return(SIG_ERR);
  return (oact.sa_handler);
}

void sig_chld(int signo){
  pid_t pid;
  int stat;
  while((pid = waitpid(-1,&stat,WUNTRACED))>0){
	printf("child %d terminated \n",pid);
  }
}

void str_echo(int sockfd){
  ssize_t n;
  int read_bytes;
  char buf[MAX_SIZE];
  memset(buf, '\0', MAX_SIZE);
  again:
    while((read_bytes = read(sockfd,buf,MAX_SIZE))>0) {
      buf[read_bytes] = '\0';
      printf("TCP Echo: %s", buf);
      write(sockfd,buf,strlen(buf));
      memset(buf, '\0', MAX_SIZE);
    }
    if(n<0 && errno == EINTR) goto again;
    else if (n<0) printf("str_echo:read error");
}

int main(int argc, char **argv)
{
  int listenfd, connfd, udpfd, nready, maxfdp1;
  char mesg[MAX_SIZE];
  pid_t childpid;
  fd_set rset;
  ssize_t n;
  socklen_t len;
  const int on = 1;
  struct sockaddr_in cliaddr, servaddr;
  void sig_chld(int);

  if(argc < 2) {
    printf("Usage: ./server <Port>\n");
    exit(1);
  }

  /* for create listening TCP socket */
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
  listen(listenfd, MAX_CONNECTION);

  /* for create UDP socket */
  udpfd = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(atoi(argv[1]));
  bind(udpfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
  signal(SIGCHLD, sig_chld);  /* must call waitpid() */

  FD_ZERO(&rset);
  maxfdp1 = max(listenfd, udpfd) + 1;
  for ( ; ; ) {
    FD_SET(listenfd, &rset);
    FD_SET(udpfd, &rset);
    if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0)
    {
      if (errno == EINTR) continue; /* back to for() */
      else printf("select error");
    }
    if (FD_ISSET(listenfd, &rset)) {
      len = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
      if ( (childpid = fork()) == 0) { /* child process */
        printf("connection from %s %d %d by TCP\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port, getpid());
        close(listenfd); /* close listening socket */
        str_echo(connfd); /* process the request */
        exit(0);
      }
      close(connfd); /* parent closes connected socket */
    }
    if (FD_ISSET(udpfd, &rset)) {
      len = sizeof(cliaddr);
      n = recvfrom(udpfd, mesg, MAX_SIZE, 0, (struct sockaddr *) &cliaddr, &len);
      printf("connection from 127.0.0.1 %d by UDP\n", cliaddr.sin_port);
      printf("UDP Echo: %s", mesg);
      sendto(udpfd, mesg, n, 0, (struct sockaddr *) &cliaddr, len);
      memset(mesg, '\0', MAX_SIZE);
    }
  }
}

