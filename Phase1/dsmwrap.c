#include "common_impl.h"

char buffer[512];
const int buff_size = 512;

int do_read(int client_sock){

	strcat(buffer,"\0");
	int bit_rcv=0;
	int *size_txt = malloc(sizeof(int));
	*size_txt = 0;
	memset(buffer,'\0',buff_size);

	do{
		bit_rcv += recv(client_sock,size_txt+bit_rcv,sizeof(int)-bit_rcv,0);
	}while(bit_rcv != sizeof(int));

	//	printf(" dsmwrap.c: do_read: 115: size_txt: %d\n",*size_txt);
	//	fflush(stdout);

	if(bit_rcv==-1){
		//perror("recv");
		close(client_sock); return 1;
	}

	bit_rcv = 0;
	do{
		bit_rcv += recv(client_sock,buffer+bit_rcv,*size_txt-bit_rcv,0);
	}while(bit_rcv != *size_txt);

	if(bit_rcv==-1){
		//perror("recv");
		close(client_sock); return 1;
	}

//			printf(" dsmwrap.c: do_read: 127: buffer: %s\n", buffer);
//			fflush(stdout);

	free(size_txt);
	return 0;
}

void do_write(int client_sock){

	strcat(buffer,"\0");
	int *size_txt = malloc(sizeof(int));
	*size_txt =0;
	*size_txt = strlen(buffer);
	int bit_sent = 0;

	do{
		bit_sent += send(client_sock,size_txt+bit_sent,sizeof(int)-bit_sent,0);
	}while(bit_sent != sizeof(int));

	//	printf(" dsmwrap.c: do_write: 141: size_txt: %d\n",*size_txt);
	//	fflush(stdout);

	bit_sent = 0;

	do{
		bit_sent += send(client_sock,buffer+bit_sent,*size_txt-bit_sent,0);
	}while(bit_sent != *size_txt);

	if(bit_sent==-1){
		perror("send");close(client_sock);exit(EXIT_FAILURE);
	}

//	printf(" dsmwrap.c: do_write: 150: bufffer: %s\n", buffer);
//	fflush(stdout);

	memset(buffer,'\0',buff_size);
	free(size_txt);
}

int do_wait_co(int client_sock){ //Verification que le accept à fonctionné, envoie d'une confirmation

	int bit_rcv;
	int *size_txt = malloc(sizeof(int));
	*size_txt = 0;

	bit_rcv = recv(client_sock,size_txt,sizeof(int),MSG_DONTWAIT);

	//	printf(" dsmwrap.c: do_read: 115: size_txt: %d\n",*size_txt);
	//	fflush(stdout);

	if(bit_rcv==-1){
		//perror("recv");
		close(client_sock); return 1;
	}
	return 0;
}

int do_check_co(int client_sock){ // Verification que le connect à fonctionné, tentative d'envoie de donné
	int bit_sent;
	int *size_txt = malloc(sizeof(int));
	*size_txt = 0;

	bit_sent = send(client_sock,size_txt,sizeof(int),MSG_DONTWAIT);

	//	printf(" dsmwrap.c: do_read: 115: size_txt: %d\n",*size_txt);
	//	fflush(stdout);

	if(bit_sent==-1){
		//perror("recv");
		close(client_sock); return 1;
	}
	return 0;
}


int rank_0(dsm_proc_t info_all_proc[]){
	int i =0;
	char h_name[128];
	gethostname(h_name,128);

	while(info_all_proc[i].connect_info.rank != 0){
		i++;
		//		printf(" dsmwrap.c: rank0 name: %s \n",info_all_proc[i].connect_info.name);
		//		fflush(stdout);
	}

	if(strcmp(h_name,info_all_proc[i].connect_info.name)==0)
		return 1;

	return 0;
}

int my_rank(dsm_proc_t info_all_proc[]){
	int i =0;
	int rank;
	char h_name[128];
	gethostname(h_name,128);

	while(strcmp(h_name,info_all_proc[i].connect_info.name)!=0){
		i++;
	}

	rank = info_all_proc[i].connect_info.rank;

	printf(" dsmwrap.c: my_rank: %d  my_name: %s\n",rank,h_name);
	fflush(stdout);

	return rank;
}

dsm_proc_t info_rank(int rank,dsm_proc_t info_all_proc[]){
	int i =0;

	while(rank != info_all_proc[i].connect_info.rank){
		i++;
	}

	//	printf(" dsmwrap.c: my_rank: %d  my_name: %s\n",rank,h_nam);
	//	fflush(stdout);

	return info_all_proc[i];
}

struct pollfd* fill_fds(int socket,struct pollfd fds[]){
	int j;
	int ok;

	j = 0;
	ok = 0;
	while(ok==0 && j<DSM_NODE_NUM){// On regarde si il y a de la place pour un nouveau client
		j++;
		if (fds[j].fd == 0){
			fds[j].fd = socket ;
			ok = 1;
		}
	}
	return fds;
}

void fill_write_socket(int client_sock, dsm_proc_t info_all_proc[], int rank){
	int i =0;

	while(rank != info_all_proc[i].connect_info.rank){
		i++;
	}

	info_all_proc[i].connect_info.write_sock = client_sock;
}

void fill_read_socket(int client_sock, dsm_proc_t info_all_proc[], int rank){
	int i =0;

	while(rank != info_all_proc[i].connect_info.rank){
		i++;
	}

	info_all_proc[i].connect_info.read_sock = client_sock;
}

int main(int argc, char **argv){

	int client_sock;
	int j;
	int i;
	int num_procs=atoi(argv[3]);
	DSM_NODE_NUM = num_procs;
	int p;
	int procs_co = 0;
	int first;
	int erreur = 1;

	char* port = argv[2];
	char* ip_addr = argv[1];
	char* true_arg[argc];
	char h_name[128] ;

	pid_t pid;
	server_info my_ser_info;
	dsm_proc_t info_all_proc[num_procs];
	dsm_proc_t info_cur_proc;
	struct pollfd fds[num_procs];
	struct sockaddr_in sin;
	int size_sin = sizeof(sin);


	for(i=0;i<num_procs;i++){
		fds[i].fd = -1;
		fds[i].revents = 0;
		fds[i].events = POLLIN;
	}


	for(i=0;i<num_procs;i++){
		info_all_proc[i].connect_info.name = malloc(buff_size*sizeof(char));
		info_all_proc[i].connect_info.ip_addr = malloc(buff_size*sizeof(char));
	}



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
	do_write(client_sock);

	/* Envoi du pid au lanceur */
	sprintf(buffer, "%d",pid);
	do_write(client_sock);

	/* Creation de la socket d'ecoute pour les */
	/* connexions avec les autres processus dsm */
	int serv_sock = creer_socket(num_procs,&my_ser_info,"8092");
	//fcntl(serv_sock,F_SETFL,O_NONBLOCK);

	fds[0].fd = serv_sock;

	/* Envoi du numero de port au lanceur */
	/* pour qu'il le propage à tous les autres */
	/* processus dsm */

	sprintf(buffer, "%d",my_ser_info.port);
	do_write(client_sock);

	strcpy(buffer,my_ser_info.ip_addr);
	do_write(client_sock);

	// on recupere les infos des autres processus

	for(i=0;i<num_procs;i++){
		do_read(client_sock);
		info_all_proc[i].pid = atoi(buffer);

		do_read(client_sock);
		info_all_proc[i].connect_info.rank = atoi(buffer);

		do_read(client_sock);
		strcpy(info_all_proc[i].connect_info.name, buffer);

		do_read(client_sock);
		strcpy(info_all_proc[i].connect_info.ip_addr, buffer);

		do_read(client_sock);
		info_all_proc[i].connect_info.port = atoi(buffer);
	}
	//	printf("DSMWRAP.C:\n\n machine name: %s \n rank: %d\n pid: %d\n ip: %s\n port: %d\n",info_all_proc[0].connect_info.name,info_all_proc[0].connect_info.rank,info_all_proc[0].pid,info_all_proc[0].connect_info.ip_addr,info_all_proc[0].connect_info.port);
	//	fflush(stdout);

	for(i=0;i<num_procs;i++){

		info_cur_proc = info_rank(i,info_all_proc);

//		printf(" name: %s, Condition: %s\n",info_cur_proc.connect_info.name,h_name);
//		fflush(stdout);


		if(strcmp(h_name,info_cur_proc.connect_info.name)!=0){ // c'est pas à moi d'accepter les connexions, je me connecte aux autres
			ip_addr = info_cur_proc.connect_info.ip_addr;
			sprintf(port,"%d",info_cur_proc.connect_info.port);

//			printf(" ip_addr: %s, port: %s \n",ip_addr,port);
//			fflush(stdout);
//			printf(" WAIT CO: %s ; connect: socket: %d\n",h_name,fds[i].fd);
//			fflush(stdout);

			erreur = 1;
			while(erreur == 1){
//				printf(" name: %s : port: %s\n",h_name,port);
//				fflush(stdout);
				client_sock = do_connect(ip_addr,port);
				erreur = do_check_co(client_sock);
				//				printf(" client_socket : %d ; erreur: %d\n",client_sock,erreur);
				//				fflush(stdout);
			}

			fill_write_socket(client_sock, info_all_proc, i);
			printf(" connected : %d!!!\n",client_sock);
			fflush(stdout);

			sprintf(buffer, " HELLO my name is %s, i am connected to you: %s\n",h_name,info_cur_proc.connect_info.name);
			do_write(client_sock);
		}
		else{ // j'accepte les connexions
			for(j=0;j<num_procs-1;j++){
//				printf(" name : %s ; ATENTE\n",h_name);
//				fflush(stdout);
				client_sock = accept(serv_sock,(struct sockaddr*)&sin,(socklen_t*) &size_sin);
				do_wait_co(client_sock); // Valide la connexion
				fds[i].fd = client_sock;
//				printf(" accepted !!!\n");
//				fflush(stdout);
				do_read(client_sock);
				fill_read_socket(client_sock,info_all_proc,i);
				printf(" buffer: %s\n",buffer);
				fflush(stdout);
			}
		}

	}// fin des connexions (fin du for)

	for(i=0;i<num_procs;i++){
		close(fds[i].fd);
	}


	for(i=0;i<num_procs;i++){
		free(info_all_proc[i].connect_info.name);
		free(info_all_proc[i].connect_info.ip_addr);
	}


	/* on execute la bonne commande */
	//execvp("./Documents/C/Projet/PR204_Dsm/Phase1/bin/truc",true_arg);
	return 0;
}
