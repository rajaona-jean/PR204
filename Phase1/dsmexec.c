#include "common_impl.h"


/* variables globales */
#define PAGE_NUMBER 10
#define PAGE_SIZE


volatile int  DSM_NODE_ID ;
volatile int* BASE_ADDR ;
volatile int* TOP_ADDR ;

int STATUS = 0 ; 
char buffer[512];
const int buff_size = 512;

/* un tableau gerant les infos d'identification */
char** machines_names ;

/* des processus dsm */
dsm_proc_t *proc_array = NULL;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;



int nb_of_user(char* path){
	FILE* fich;
	int nb_mot = -1;
	char* l = malloc(sizeof(char));
	int fin = 0;
	fich = fopen(path,"r");
	while(fin == 0){
		fread(l,1,1,fich);
		if(l[0] == '\n')
			nb_mot++;
		fin = feof(fich);
	}

	fclose(fich);
	return nb_mot;
}

void init_names(char* path,char** machines_names, int nb_mach){
	FILE* fich;
	char* l = malloc(sizeof(char));
	int fin = 0;
	fich = fopen(path,"r");
	if(fich==NULL){
		printf("oups");
	}

	int i=0; //ligne
	int j=0; //colonne
	fin = 0;

	//char mots[nb_mot][28];
	char** mots = malloc(nb_mach*sizeof(char*));
	for(i=0; i<nb_mach;i++){
		mots[i] = malloc(28*sizeof(char));
	}


	fich = fopen(path,"r");
	i=0;
	while(fin == 0 && i!=nb_mach){

		fread(l,1,1,fich);
		if(l[0] == '\n'){
			mots[i][j]='\0';
			i++;
			j=0;
		}
		else{
			mots[i][j]=l[0];
			j++;
		}

		fin = feof(fich);
	}

	fclose(fich);
	for(i=0;i<nb_mach;i++){
		if(strcmp(mots[i],"localhost")==0)
			gethostname(mots[i],128);
		strcpy(machines_names[i],mots[i]);
	}

}


void usage(void)
{
	fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
	fflush(stdout);
	exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
	/* on traite les fils qui se terminent */
	/* pour eviter les zombies */
	wait(NULL);
	//STATUS = 1 ;

}

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

int find_rank(char* machine_name){
	int i =0;
	while( i<DSM_NODE_NUM){
		if(strcmp(machine_name,machines_names[i])==0)
			break;
		i++;
	}
	return i;
}

int main(int argc, char *argv[]){
	char* path = "./machines.txt"; //argv[1];
	char* int_to_char=malloc(sizeof(int));
	int status;
	struct sigaction * sig_zombie = malloc(sizeof(struct sigaction));

	int num_procs = 0;
	num_procs = nb_of_user(path);
	pid_t pid[num_procs];

	if (argc < 1){
		usage();
	} else {
		int i;
		int j;

		/* Mise en place d'un traitant pour recuperer les fils zombies*/
		/* XXX.sa_handler = sigchld_handler; */

		memset (sig_zombie, 0 , sizeof(struct sigaction) );
		sig_zombie -> sa_handler = sigchld_handler ;

		/* lecture du fichier de machines */
		/* 1- on recupere le nombre de processus a lancer */

		DSM_NODE_NUM = num_procs;

		printf("  %d\n",num_procs);

		/* 2- on recupere les noms des machines : le nom de */
		/* la machine est un des elements d'identification */
		machines_names = malloc(num_procs*sizeof(char*));
		for (i = 0; i < num_procs; i++)
			machines_names[i] = malloc(sizeof(char) * 50);
		init_names(path,machines_names,num_procs);

		/* creation de la socket d'ecoute */
		/* + ecoute effective */
		server_info father_info;
		int FD = creer_socket(num_procs, &father_info,"8079");


		/* creation du tube pour rediriger stdout */
		int tube_stdout[2][num_procs];
		/* creation du tube pour rediriger stderr */
		int tube_stderr[2][num_procs];

		/* creation des fils */
		for(i = 0; i < num_procs ; i++) {

			/* creation du tube pour rediriger stdout */
			tube_stdout[0][i] = 4;
			tube_stdout[1][i] = 5;
			/* creation du tube pour rediriger stderr */
			tube_stderr[0][i] = 4;
			tube_stderr[1][i] = 5;


			pid[i] = fork();


			if(pid[i] == -1) ERROR_EXIT("fork");
			//printf("pid: %d\n",pid);

			if (pid[i] == 0) { /* fils */
				int j;
				/* redirection stdout */
				dup2(STDOUT_FILENO,tube_stdout[0][i]);
				close(tube_stdout[0][i]); //suppression tube en lecture


				/* redirection stderr */
				dup2(STDERR_FILENO,tube_stderr[0][i]);
				close(tube_stderr[0][i]);


				/* Creation du tableau d'arguments pour le ssh */

				char* newargv[6+argc];
				newargv[4] = malloc(sizeof(int));
				newargv[5] = malloc(sizeof(int));

				newargv[0] = "ssh";
				newargv[1] = machines_names[i];
				newargv[2] = "./Documents/C/Projet/PR204_Dsm/Phase1/bin/dsmwrap";
				newargv[3] = father_info.ip_addr;
				sprintf(int_to_char, "%d", father_info.port);
				strcpy(newargv[4],int_to_char);
				sprintf(int_to_char, "%d", num_procs);
				strcpy(newargv[5], int_to_char);
				for(j=1; j<=argc; j++){
					newargv[j+5]=argv[j];
				}

				execvp("ssh",newargv);


			} else  if(pid[i] > 0) { /* pere */
				/* fermeture des extremites des tubes non utiles */
				close(tube_stdout[1][i]);
				close(tube_stderr[1][i]);
				num_procs_creat++;
			}
		}


		struct sockaddr_in sin;
		dsm_proc_t info_process_distant[num_procs];

		for(i=0;i<num_procs;i++){
			info_process_distant[i].connect_info.name = malloc(buff_size*sizeof(char));
			info_process_distant[i].connect_info.ip_addr = malloc(buff_size*sizeof(char));
		}

		int rank;
		int size = sizeof(sin);
		int len;
		int csock ;
		pid_t pid_proc_dist;
		struct pollfd fds[num_procs];


		//Initis the pollfd structure
		memset(fds, 0,sizeof(fds));
		fds[0].fd = FD;
		fds[0].events = POLLIN;
		for(i=0;i<num_procs;i++)
			fds[i].events = POLLIN;


		for(i = 0; i < num_procs ; i++){
			/* on accepte les connexions des processus dsm */
			csock = accept(FD,(struct sockaddr*)&sin,(socklen_t*) &size);

			if(csock == -1){
				perror("accept");exit(0);
			}
			else{
				printf("accept\n");
				fflush(stdout);
			}

			fds[i].fd = csock;
			/*  On recupere le nom de la machine distante */

			/* 1- d'abord la taille de la chaine */
			do_read(csock);
			len= strlen(buffer);
			/* 2- puis la chaine elle-meme */
			char name[len];
			strcpy(name,buffer);
			rank = find_rank(name);


			/* On recupere le pid du processus distant  */
			do_read(csock);
			//			printf(" PID: %s\n",buffer);
			//			fflush(stdout);
			pid_proc_dist = atoi(buffer);

			info_process_distant[i].connect_info.rank = rank;
			info_process_distant[i].pid = pid_proc_dist;
			//info_process_distant[i].connect_info.name = name;
			strcpy(info_process_distant[i].connect_info.name, name);

			//			printf("DSMEXEC.C: machine name: %s \n rank: %d\n pid: %d\n",info_process_distant[i].connect_info.name,info_process_distant[i].connect_info.rank,info_process_distant[i].pid = pid_proc_dist);
			//			fflush(stdout);

			/* On recupere le numero de port de la socket */
			/* d'ecoute des processus distants */
			do_read(csock);
			info_process_distant[i].connect_info.port = atoi(buffer);

			do_read(csock);
			strcpy(info_process_distant[i].connect_info.ip_addr,buffer);


		}

		/* envoi du nombre de processus aux processus dsm*/
		// Deja fait. Le nombre de processus est une variable envoyé dès le lancement de dsmwrap.

		/* envoi des infos de connexion aux processus */
		for(j=0;j<num_procs;j++){
			csock = fds[j].fd;
			for(i=0;i<num_procs;i++){
				sprintf(buffer, "%d",info_process_distant[i].pid);
				do_write(csock);

				sprintf(buffer, "%d",info_process_distant[i].connect_info.rank);
				do_write(csock);

				strcpy(buffer,info_process_distant[i].connect_info.name);
				do_write(csock);

				strcpy(buffer,info_process_distant[i].connect_info.ip_addr);
				do_write(csock);

				sprintf(buffer, "%d",info_process_distant[i].connect_info.port);
				do_write(csock);
			}
		}




		//		/* gestion des E/S : on recupere les caracteres */
		//		/* sur les tubes de redirection de stdout/stderr */
		//		/*while(1)
		//         {
		//            je recupere les infos sur les tubes de redirection
		//            jusqu'à ce qu'ils soient inactifs (ie fermes par les
		//            processus dsm ecrivains de l'autre cote ...)
		//
		//         }
		//		 */
		//
		//		/* on attend les processus fils */
		for(i=0;i<num_procs;i++){
			waitpid(pid[i],&status,0);
		}
		//		/* on ferme les descripteurs proprement */
		//
		//		/* on ferme la socket d'ecoute */

		for(i=0;i<num_procs;i++){
			free(info_process_distant[i].connect_info.name);
			free(info_process_distant[i].connect_info.ip_addr);
		}
	}
	exit(EXIT_SUCCESS);
}

