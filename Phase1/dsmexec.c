#include "common_impl.h"


/* variables globales */
#define PAGE_NUMBER 10
#define PAGE_SIZE

volatile int  DSM_NODE_NUM ;
volatile int  DSM_NODE_ID ;
volatile int* BASE_ADDR ;
volatile int* TOP_ADDR ;
int STATUS = 0 ; 

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
	//int nb_mot = -1;
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


int main(int argc, char *argv[]){

	char* path = "machines.txt";
	struct dsm_proc dsm_proc_t;
	struct dsm_proc_conn connect_info;
	

	if (argc < 1){
		usage();
	} else {
		pid_t *pid= NULL;
		int num_procs = 0;
		int i;

		/* Mise en place d'un traitant pour recuperer les fils zombies*/
		/* XXX.sa_handler = sigchld_handler; */
		struct sigaction * sig_zombie = malloc(sizeof(struct sigaction));
		memset (sig_zombie, 0 , sizeof(struct sigaction) );
		sig_zombie -> sa_handler = sigchld_handler ;

		/* lecture du fichier de machines */
		/* 1- on recupere le nombre de processus a lancer */
		num_procs = nb_of_user(path);
		printf("  %d\n",num_procs);

		/* 2- on recupere les noms des machines : le nom de */
		/* la machine est un des elements d'identification */
		machines_names = malloc(num_procs*sizeof(char*));
		for (i = 0; i < num_procs; i++)
			machines_names[i] = malloc(sizeof(char) * 50);
		init_names(path,machines_names,num_procs);
		/* creation de la socket d'ecoute */

		/* + ecoute effective */
    	int * port_num = malloc(sizeof(int));
    	*port_num = 8080;
    	int FD = creer_socket(num_procs, port_num);

		/* creation des fils */
		for(i = 0; i <= num_procs ; i++) {

			/* creation du tube pour rediriger stdout */
			int tube_stdout[2] = {0,1};
			/* creation du tube pour rediriger stderr */
			int tube_stderr[2] = {0,1};

			pid[i] = fork();

			//WAIT_STATUS status;
			if(pid[i] == -1) ERROR_EXIT("fork");
			//printf("pid: %d\n",pid);

			if (pid[i] == 0) { /* fils */
				int j;
				/* redirection stdout */
				dup2(STDOUT_FILENO,tube_stdout[0]);
				close(tube_stdout[0]); //suppression tube en lecture


				/* redirection stderr */
				dup2(STDERR_FILENO,tube_stderr[0]);
				close(tube_stderr[0]);


				/* Creation du tableau d'arguments pour le ssh */

				void* newargv[argc];
				printf(" argc: %d\n",argc);
				for(j=0; j<argc; j++){
					newargv[j]=argv[j+1];
					printf(" %s: %s\n",machines_names[i],newargv[j]);
				}


				/* jump to new prog : */
				/* execvp("ssh",newargv); */
				char* dsm[3];
				dsm[0] = "ssh";
				dsm[1] = machines_names[i];
				dsm[2] = "pwd";
				execvp("ssh",dsm);


			} else  if(pid > 0) { /* pere */
				/* fermeture des extremites des tubes non utiles */
				close(tube_stdout[1]);
				close(tube_stderr[1]);
				num_procs_creat++;
				//wait(status);
			}
		}


		struct sockaddr_in sin; 
		int size = sizeof(sin);
		int len;
		int csock ;
		struct pollfd fds;
		fds.events = POLLIN;

		//struct pollfd fds[num_procs]; 

		for(i = 0; i <= num_procs ; i++){
			/* on accepte les connexions des processus dsm */
			csock = accept(FD,(struct sockaddr*)&sin,(socklen_t*) &size);

			/*  On recupere le nom de la machine distante */

			/* 1- d'abord la taille de la chaine */
			/* 2- puis la chaine elle-meme */

			len= strlen(machines_names[i]);
			char * name= malloc (sizeof(char)* len);
			name = machines_names[i];
			

			/* On recupere le pid du processus distant  */
			
			dsm_proc_t.pid = getpid();
			connect_info.rank = pid[i];
			connect_info.IPaddr = *gethostbyname(name) ;

			/* On recupere le numero de port de la socket */
			/* d'ecoute des processus distants */
			connect_info.port = port_num;
			dsm_proc_t.connect_info = connect_info;

		}

		/* envoi du nombre de processus aux processus dsm*/
		int * buf[4];
		int *buf1;
		buf1 = &num_procs;
		write(FD,buf1, len+1);
		printf("le nombre de processus est%d\n", num_procs);
		*buf[0]= *buf1;
		/* envoi des rangs aux processus dsm */
		int *buf2;
		buf2 = &connect_info.rank;
		write(FD,buf2, len+1 );
		printf("le rang du processus \n" );
		*buf[1] =*buf2;
		/* envoi des infos de connexion aux processus */
		
		int *buf3 ;
		buf3 = &connect_info.IPaddr;
		*buf[2] = *buf3;
		write(FD,buf3, len+1);

		int *buf4 ;
		buf4 = connect_info.port;
		write(FD,buf4, len+1);
		printf("informations de connexion\n" );
		*buf[3] = *buf4;

		/* gestion des E/S : on recupere les caracteres */
		/* sur les tubes de redirection de stdout/stderr */
		/*while(1)
         {
            je recupere les infos sur les tubes de redirection
            jusqu'à ce qu'ils soient inactifs (ie fermes par les
            processus dsm ecrivains de l'autre cote ...)

         }
		*/

		/* on attend les processus fils */

		/* on ferme les descripteurs proprement */

		/* on ferme la socket d'ecoute */
	}
	exit(EXIT_SUCCESS);
}

