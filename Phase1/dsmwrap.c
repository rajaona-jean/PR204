#include "common_impl.h"

int main(int argc, char **argv)
{
	int client_sock;
	struct sockaddr_in server_sock;

	printf("DSMWRAP\n");
	fflush(stdout);
   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */
   

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */   
	client_sock = do_socket();
	init_server_addr(argv[1],atoi(argv[2]),server_sock);
	do_connect(client_sock,server_sock);

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
