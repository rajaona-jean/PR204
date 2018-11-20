#include "common_impl.h"



void setsock(int socket_fd){
	int option = 1;
	int error = setsockopt(socket_fd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option));
	if(error == -1){
		perror("setsockopt");
	}
}

void init_server_addr(char* addr,int port,struct sockaddr_in server_sock){
	memset(&server_sock,'\0',sizeof(server_sock));
	server_sock.sin_family = AF_INET;
	server_sock.sin_port = htons(port);
	inet_aton(addr,&server_sock.sin_addr);
	//return server_sock;
}

void do_connect(int client_socket,struct sockaddr_in server_sock){
	int err = connect(client_socket,(struct sockaddr *)&server_sock,sizeof(server_sock));
	if(err == -1){
		ERROR_EXIT("connect");
	}
}

int do_socket(){
	int s = socket(AF_INET,SOCK_STREAM,0);
	if (s == -1){
		fprintf(stdout , " client: Erreur création de socket 2\n");
		fflush(stdout);
		ERROR_EXIT("socket");
	}
	else{
		fprintf(stdout , " client: Socket créée %d\n", s);
		fflush(stdout);
	}
	return s;
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
		perror("socket");
		exit(0);
	}

	fd = sock;


	int bnd = bind(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
	if ( bnd == -1 ){
		perror("bind");
		exit(0);
	}

	int lst = listen(sock, num_procs);
	if ( lst== -1 ){
		perror("listen");
		exit(0);
	}

	return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */



