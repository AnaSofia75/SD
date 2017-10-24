#include "network_client-private.h"

#include <stdlib.h>


int write_all(int sock, char *buf, int len){

	int bufsize = len;

	while(len>0) {
		int res = write(sock, buf, len);
		if(res<0) {
			if(errno==EINTR) continue;
			perror(“write failed:”);
			return res;
		}
		buf += res;
		len -= res;
	}
	return bufsize;
}



int read_all(int sock, char *buf, int len){

	int bufsize = len;

	while(len>0) {
		int res = read(sock, buf,len);
		if(res <0){
			if(errno==EINTR) continue;
			perror(“read failed:”);
			return res;
		}
		buf += res;
		len -= res;
	}
	return bufsize;
}


struct server_t *network_connect(const char *address_port){
	if (address_port == NULL)
		return NULL;

	struct server_t *server = malloc(sizeof(struct server_t));

	/* Verificar parâmetro da função e alocação de memória */
	if (server == NULL)
		return NULL;

	int sockfd, i=0;
	char str[MAX_MSG];

	/* Estabelecer ligação ao servidor:

		Preencher estrutura struct sockaddr_in com dados do
		endereço do servidor.

		Criar a socket.

		Estabelecer ligação.
	*/
	char* str_aux = strtok(address_port,":");
	server->socket->sin_family = AF_INET;
	if (inet_pton(AF_INET,str_aux,&server->socket->sin_addr) < 1){
		free(server);
		free(str_aux);
		printf("Erro ao converter IP\n");
		return NULL;
	}
	server->socket->sin_port = htons(atoi(strtok(NULL,":")));

	if ((server->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		free(server->socket);
		free(server);
		free(str_aux);
		perror("Erro ao criar socket TCP");
		return NULL;
	}

	/* Se a ligação não foi estabelecida, retornar NULL */
	if (connect(server->sockfd,(struct sockaddr *)&server->socket->sin_addr, sizeof(struct server_t)) < 0) {
		perror("Erro ao conectar-se ao servidor");
		close(server->sockfd);
		free(server->socket);
		free(server);
		free(str_aux);
		return NULL;
	}

	free(str_aux);
	return server;
}

struct message_t *network_send_receive(struct server_t *server, struct message_t *msg){
	char *message_out;
	int message_size, msg_size, result;
	struct message_t msg_resposta;

	/* Verificar parâmetros de entrada */
	if (server == NULL || msg == NULL)
		return NULL;

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg, &message_out));

	/* Verificar se a serialização teve sucesso */
	if (message_size < 0 || message_size > MAX_MSG)
		return NULL;

	/* Enviar ao servidor o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);
 	result = write_all(server->sockfd, (char *) &msg_size, _INT));

	/* Verificar se o envio teve sucesso */
	if (result < 0)
		return NULL;

	/* Enviar a mensagem que foi previamente serializada */
	result = write_all(server->sockfd, message_out, message_size));

	/* Verificar se o envio teve sucesso */
	if (result < 0){
		free(message_out);
		return NULL;
	}

	/* De seguida vamos receber a resposta do servidor:

		Com a função read_all, receber num inteiro o tamanho da
		mensagem de resposta.

		Alocar memória para receber o número de bytes da
		mensagem de resposta.

		Com a função read_all, receber a mensagem de resposta.

	*/
	result = read_all(server->sockfd,(char *) &msg_size,_INT);
	if (result != _INT){
		perror("Erro ao receber dados do servidor");
		close(server->sockfd);
		free(message_out);
		return NULL;
	}
	message_size = ntohl(msg_size);

	result = read_all(server->sockfd,message_out,message_size);

	if (result != message_size){
		close(server->sockfd);
		free(msg_resposta);
		free(message_out);
		return NULL;
	}

	&msg_resposta = malloc(sizeof(message_size));
	if (msg_resposta == NULL){
		close(server->sockfd);
		free(message_out);
		return NULL;
	}

	/* Desserializar a mensagem de resposta */
	msg_resposta = buffer_to_message(message_out,message_size);

	/* Verificar se a desserialização teve sucesso */
	if (msg_resposta == NULL){
		close(server->sockfd);
		free(msg_resposta);
		free(message_out);
		return NULL;
	}

	/* Libertar memória */
	free(message_out);

	return msg_resposta;
}

int network_close(struct server_t *server){
	/* Verificar parâmetros de entrada */
	if (server == NULL){
		return -1;
	}

	/* Terminar ligação ao servidor */
	close(server->sockfd);

	/* Libertar memória */
	free(server->socket);
	free(server);

	return 0;
}
