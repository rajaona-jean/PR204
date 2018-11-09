#include "common_impl.h"

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

int creer_socket(int prop, int *port_num) 
{
	int fd;
  struct sockaddr_in sin;
  struct pollfd fds[DSM_NODE_NUM];


  sin.sin_port = port_num;
   
   /* fonction de creation et d'attachement */
   /* d'une nouvelle socket */
   /* renvoie le numero de descripteur */
   /* et modifie le parametre port_num */
  poll(fd,DSM_NODE_NUM+1,  -1);

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1)
  {
    perror("socket()");
    exit(0);
  }

  memset(fds,'\0',sizeof(fds));
  fds[0].fd = sock;
  fds[0].events = POLLIN;

  int bnd = bind(sock, (struct sockaddr*)&sin, sizeof(sin));
  if ( bnd == -1 ){
    perror("bind()");
    exit(0);
  }

  int lst = listen(sock, DSM_NODE_NUM);
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
