#include "common_impl.h"

char buffer[512];
const int buff_size = 512;

void do_read(int client_sock){

	strcat(buffer,"\0");
	int bit_rcv;
	int *size_txt = malloc(sizeof(int));
	*size_txt = 0;
	memset(buffer,'\0',buff_size);

	bit_rcv = recv(client_sock,size_txt,sizeof(int),0);

//	printf(" dsmexec.c: do_read: 115: size_txt: %d\n",*size_txt);
//	fflush(stdout);

	if(bit_rcv==-1){
		perror("recv");close(client_sock); exit(EXIT_FAILURE);
	}

	bit_rcv = recv(client_sock,buffer,*size_txt,0);
	if(bit_rcv==-1){
		perror("recv");close(client_sock); exit(EXIT_FAILURE);
	}

//	printf(" dsmexec.c: do_read: 127: buffer: %s\n", buffer);
//	fflush(stdout);

	free(size_txt);
}

void do_write(int client_sock){

	strcat(buffer,"\0");
	int *size_txt = malloc(sizeof(int));
	*size_txt =0;
	*size_txt = strlen(buffer);
	int bit_sent = 0;


	bit_sent = send(client_sock,size_txt,sizeof(int),0);

//	printf(" dsmexec.c: do_write: 141: size_txt: %d\n",*size_txt);
//	fflush(stdout);

	bit_sent = send(client_sock,buffer,*size_txt,0);
	if(bit_sent==-1){
		perror("send");close(client_sock);exit(EXIT_FAILURE);
	}

//	printf(" dsmexec.c: do_write: 150: bufffer: %s\n", buffer);
//	fflush(stdout);

	memset(buffer,'\0',buff_size);
	free(size_txt);
}


int main(int argc, char **argv){

	int client_sock;
	int j;
	int num_procs=atoi(argv[3]);

	char* port = argv[2];
	char* ip_addr = argv[1];
	char* true_arg[argc];
	char h_name[128] ;

	pid_t pid;
	server_info my_ser_info;

	gethostname(h_name,128);
	pid = getpid();

	printf("DSMWRAP\n");
	printf(" argc: %d\n",argc);
	printf(" hostname: %s\n",h_name);
	fflush(stdout);

//		for(j=0; j<argc; j++){
//			printf(" arg[%d]: %s\n",j,(char*)argv[j]);
//			fflush(stdout);
//
//		}

	/* processus intermediaire pour "nettoyer" */
	/* la liste des arguments qu'on va passer */
	/* a la commande a executer vraiment */
	for(j=0; j<argc-3; j++){
		true_arg[j] = argv[j+3];
		//		printf(" true_arg[%d]: %s\n",j,(char*)true_arg[j]);
		//		fflush(stdout);
	}

	/* creation d'une socket pour se connecter au */
	/* au lanceur et envoyer/recevoir les infos */
	/* necessaires pour la phase dsm_init */

	client_sock = do_connect(ip_addr,port);

	/* Envoi du nom de machine au lanceur */


	strcpy(buffer,h_name);
	printf(" dsmwrap.c:49,  buffer: %s\n",buffer);
	fflush(stdout);
	do_write(client_sock);

	/* Envoi du pid au lanceur */
	sprintf(buffer, "%d",pid);
	printf(" dsmwrap.c:81,  buffer: %s\n",buffer);
	fflush(stdout);
	do_write(client_sock);

	/* Creation de la socket d'ecoute pour les */
	/* connexions avec les autres processus dsm */
 	int serv_sock = creer_socket(num_procs,&my_ser_info,"8081");

	/* Envoi du numero de port au lanceur */
	/* pour qu'il le propage à tous les autres */
	/* processus dsm */

 	sprintf(buffer, "%d",my_ser_info.port);
 	do_write(client_sock);

 	strcpy(buffer,my_ser_info.ip_addr);
 	do_write(client_sock);

	/* on execute la bonne commande */
	return 0;
}
