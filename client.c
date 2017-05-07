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

#define max(x, y) (((x) > (y)) ? (x) : (y))

void str_cli(FILE *fp, int sockfd)
{
  int maxfdp1, stdineof;
  fd_set rset;
  char sendline[MAX_SIZE], recvline[MAX_SIZE];
  stdineof = 0; /* use for test readable, 0: connect */
  FD_ZERO(&rset);
  for ( ; ; ) {
    if (stdineof == 0) {
      FD_SET(fileno(fp), &rset);
      FD_SET(sockfd, &rset);
      maxfdp1 = max(fileno(fp), sockfd) + 1;
      select(maxfdp1, &rset, NULL, NULL, NULL);
    }
    if (FD_ISSET(sockfd, &rset)) {                           /* socket is readable */
      if (read(sockfd, recvline, MAX_SIZE) == 0) {
        if (stdineof == 1) return;
	else printf("str_cli: server terminated prematurely\n");
	exit(0);
      }
      fputs(recvline, stdout);
    }
    if (FD_ISSET(fileno(fp), &rset)) {                       /* input is readable */
      if (fgets(sendline, MAX_SIZE, fp) ==  NULL) {     //EOF
        stdineof = 1;
        shutdown(sockfd, SHUT_WR);               /* send FIN */
	FD_CLR(fileno(fp), &rset);
	continue;
      }
    }
    write(sockfd, sendline, strlen(sendline));
    memset(sendline, '\0', MAX_SIZE);
  }
}

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen)
{ 
  int n;
  char sendline[MAX_SIZE], recvline[MAX_SIZE + 1]; 
  while (fgets(sendline, MAX_SIZE, fp) != NULL) {
    sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
     n = recvfrom(sockfd, recvline, MAX_SIZE, 0, NULL, NULL);
    recvline[n] = 0;        /* null terminate */
    fputs(recvline, stdout);
  } 
} 

int main(int argc, char **argv)
{ 
  int sockfd; 
  struct sockaddr_in servaddr;
  if(argc < 4) {
    printf("Usage: ./<IP> <TCP/UDP> <Port>\n");
    exit(0);
  }
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET; 
  servaddr.sin_port = htons(atoi(argv[3]));
  inet_pton(AF_INET, argv[2], &servaddr.sin_addr);
  if(strcmp(argv[1],"TCP")==0) {
    printf("TCP client created\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    str_cli(stdin, sockfd);
  }
  else {
    printf("UDP client created\n");
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    dg_cli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
  }
  exit(0);
} 
