/*
	Programa cliente para manipular tabela de hash remota.
	Os comandos introduzido no programa não deverão exceder
	80 carateres.

	Uso: table-client <ip servidor>:<porta servidor>
	Exemplo de uso: ./table_client 10.101.148.144:54321
*/

#include "network_client-private.h"

int main(int argc, char **argv){
	struct server_t *server;
	char input[81];
	struct message_t *msg_out, *msg_resposta;

	/* Testar os argumentos de entrada */
	if (argc != 4){
		printf("Uso: ./client <ip_servidor> <porto_servidor> <string>\n");
		printf("Exemplo de uso: ./client 127.0.0.1 12345 olacomovais\n")
		return -1;
	}

	strcat(argv[1], ":");
	strcat(argv[1], argv[2]);

	/* Usar network_connect para estabelcer ligação ao servidor */
	server = network_connect(argv[1]);
	char* opcao;
	/* Fazer ciclo até que o utilizador resolva fazer "quit" */
 	while (/* condição */){

		printf(">>> "); // Mostrar a prompt para inserção de comando

		/* Receber o comando introduzido pelo utilizador
		   Sugestão: usar fgets de stdio.h
		   Quando pressionamos enter para finalizar a entrada no
		   comando fgets, o carater \n é incluido antes do \0.
		   Convém retirar o \n substituindo-o por \0.
		*/
		scanf("%s", &opcao);
		//fgets(opcao, ,)
		/* Verificar se o comando foi "quit". Em caso afirmativo
		   não há mais nada a fazer a não ser terminar decentemente.
		 */
		if (strcmp(opcao, "quit")==0){
			free(opcao);
			return 0;
		}
		/* Caso contrário:

			Verificar qual o comando;

			Preparar msg_out;

			Usar network_send_receive para enviar msg_out para
			o server e receber msg_resposta.
		*/
		char* op= strtok(opcao, " ");
		int tabela = atoi(strok(NULL, " "));
		if (strcmp(op, "size")==0){
			
		}
		if (strcmp(op, "put")==0){

		}
	}
  	return network_close(server);
}
