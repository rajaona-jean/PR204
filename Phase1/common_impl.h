#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

int  DSM_NODE_NUM ;
int  size_buff;
char buffer[512];

/* autres includes (eventuellement) */

#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}


struct father_info {
   char* ip_addr;
   int port;
};
typedef struct father_info father_info;

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   /* a completer */
   char* ip_addr;
   int* port;
};
typedef struct dsm_proc_conn dsm_proc_conn_t; 

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {   
  pid_t pid;
  dsm_proc_conn_t connect_info;
};

typedef struct dsm_proc dsm_proc_t;



int creer_socket(int num_procs, father_info *f_info);

//int do_socket();
//void init_server_addr(char* addr,int port,struct sockaddr_in server_sock);

void setsock(int socket_fd);
int do_connect(char* ip_addr,char* port);
void do_read(int client_sock,int server_sock);
void do_write(int client_sock,int server_sock);
