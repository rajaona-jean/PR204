#include "common_impl.h"


void setsock(int socket_fd){
	int option = 1;
	int error = setsockopt(socket_fd,SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option));
	if(error == -1){
		perror("setsockopt");
	}
}


int do_connect(char* ip_addr,char* port){
	int client_sock = 0;
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
	}else{
		printf("listen ok\n");
	}

	return sock;
}

void* do_read_struct(int client_sock, void* objet){

	int bit_rcv;
	void* objet_recu;




	bit_rcv = recv(client_sock,objet_recu,sizeof(objet),0);

	//	printf(" dsmexec.c: do_read: 115: size_txt: %d\n",*size_txt);
	//	fflush(stdout);

	if(bit_rcv==-1){
		perror("recv");close(client_sock); exit(EXIT_FAILURE);
	}

	return objet;
}

void do_write_struct(int client_sock, void* objet){

	int bit_sent = 0;
	int taille = sizeof(objet);

	while(bit_sent<taille)
		bit_sent = bit_sent + send(client_sock,objet,sizeof(objet),0);

	//	printf(" dsmexec.c: do_read: 115: size_txt: %d\n",*size_txt);
	//	fflush(stdout);

	if(bit_sent==-1){
		perror("send");close(client_sock); exit(EXIT_FAILURE);
	}
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */



