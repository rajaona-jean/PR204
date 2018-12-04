#include "common_impl.h"

int main(int argc, char **argv)
{
	int client_sock;
	int j;
	struct sockaddr_in server_sock;
	char* true_arg[argc];
	char* ip_addr = argv[1];
	int port = atoi(argv[2]);

	printf("DSMWRAP\n");
	printf(" argc: %d\n",argc);
	fflush(stdout);

	for(j=0; j<argc; j++){
		printf(" arg[%d]: %s\n",j,(char*)argv[j]);
		fflush(stdout);

	}

	/* processus intermediaire pour "nettoyer" */
	/* la liste des arguments qu'on va passer */
	/* a la commande a executer vraiment */
	for(j=0; j<argc-3; j++){
		true_arg[j] = argv[j+4];
		printf(" true_arg[%d]: %s\n",j,(char*)true_arg[j]);
		fflush(stdout);

	}

	/* creation d'une socket pour se connecter au */
	/* au lanceur et envoyer/recevoir les infos */
	/* necessaires pour la phase dsm_init */

	printf("ip addr = %s\n port = %d\n",ip_addr,port);
	//init_server_addr(ip_addr,port,server_sock);
	do_connect(ip_addr,argv[2]);

	/* Envoi du nom de machine au lanceur */

	/* Envoi du pid au lanceur */

	/* Creation de la socket d'ecoute pour les */
	/* connexions avec les autres processus dsm */

	/* Envoi du numero de port au lanceur */
	/* pour qu'il le propage à tous les autres */
	/* processus dsm */

	/* on execute la bonne commande */
	return 0;
}
