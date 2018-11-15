#include "common_impl.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void setsock(int socket_fd){
	int option = 1;
	int error = setsockopt(socket_fd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option));
	if(error == -1){
		perror("setsockopt");
	}
}

int creer_socket(/*int prop*/int num_procs, int *port_num) //prop pour propriétés bloquantes ou non bloquantes
{

	int fd = 0;
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family=AF_INET;
	sin.sin_port = htons(*port_num);

	/* fonction de creation et d'attachement */
	/* d'une nouvelle socket */
	/* renvoie le numero de descripteur */
	/* et modifie le parametre port_num */

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	setsock(sock);
	if(sock == -1)
	{
		perror("socket()");
		exit(0);
	}

	memset(fds,'\0',sizeof(fds));
	fd = sock;


	int bnd = bind(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
	if ( bnd == -1 ){
		perror("bind()");
		exit(0);
	}

	int lst = listen(sock, num_procs);
	if ( lst== -1 ){
		perror("listen()");
		exit(0);
	}

	return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
