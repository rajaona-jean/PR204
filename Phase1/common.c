#include "common_impl.h"


void setsock(int socket_fd){
	int option = 1;
	int error = setsockopt(socket_fd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option));
	if(error == -1){
		perror("setsockopt");
	}
}

void just_connect(int client_socket,char* ip_addr,int port){
	struct sockaddr_in server_sock;

	memset(&server_sock,'\0',sizeof(struct sockaddr_in));
	server_sock.sin_family = AF_INET;
	server_sock.sin_port = htons(port);
	server_sock.sin_addr.s_addr = inet_addr(ip_addr);

	int err = connect(client_socket,(struct sockaddr *)&server_sock,sizeof(struct sockaddr_in));
	if(err == -1){
		perror("connect");close(client_socket);exit(EXIT_FAILURE);
	}
	else{
		printf(" Connecting to server... done!\n");
		fflush(stdout);
	}
}

int do_connect(char* ip_addr,char* port){
	int client_sock = -1;
	struct addrinfo hints;
	struct addrinfo *server,*result;
	int s; // getaddrinfo return


	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    /* Allow IPv4 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;


	s = getaddrinfo(ip_addr,port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	for (server = result; server != NULL; server = server->ai_next) {
		client_sock = socket(server->ai_family, server->ai_socktype,
				server->ai_protocol);
		setsock(client_sock);
		if (client_sock == -1)
			continue;

		if ( connect(client_sock, server->ai_addr, server->ai_addrlen) != -1)
			break;                  /* Success */

		close(client_sock);
	}

	return client_sock;
}


int creer_socket(/*int prop*/int num_procs, server_info *s_info,char* port) //prop pour propriétés bloquantes ou non bloquantes
{
	int sock = 0;
	struct sockaddr_in *sin;
	struct addrinfo hints;
	struct addrinfo *info,*result;
	char h_name[128];
	int s; // getaddrinfo return


	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    /* Allow IPv4 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;


	/* fonction de creation et d'attachement */
	/* d'une nouvelle socket */
	/* renvoie le numero de descripteur */
	/* et modifie le parametre port_num */


	gethostname(h_name, 128);
	s = getaddrinfo(h_name,port, &hints, &result);

	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	for (info = result; info != NULL; info = info->ai_next) {
		sock = socket(info->ai_family, info->ai_socktype,
				info->ai_protocol);
		setsock(sock);
		sin = (struct sockaddr_in*)info->ai_addr;
		s_info->port = ntohs(sin->sin_port);
		s_info->ip_addr = inet_ntoa(sin->sin_addr);
		if (sock == -1)
			continue;

		if (bind(sock, info->ai_addr, info->ai_addrlen) == 0)
			break;                  /* Success */

		close(sock);
	}
	freeaddrinfo(result);

	int lst = listen(sock, num_procs);
	if ( lst== -1 ){
		perror("listen");
		exit(0);
	}
	return sock;
}


/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */



