/*
   Programa que implementa um servidor de uma tabela hash com chainning.
   Uso: table-server <port> <table1_size> [<table2_size> ...]
   Exemplo de uso: ./table_server 54321 10 15 20 25
*/
#include <error.h>

#include "inet.h"
#include "table-private.h"
#include "message-private.h"


/* Função para preparar uma socket de receção de pedidos de ligação.
*/
int make_server_socket(short port){
  int socket_fd;
  struct sockaddr_in server;

  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("Erro ao criar socket");
    return -1;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0){
      perror("Erro ao fazer bind");
      close(socket_fd);
      return -1;
  }

  if (listen(socket_fd, 0) < 0){
      perror("Erro ao executar listen");
      close(sfd);
      return -1;
  }
  return socket_fd;
}


/* Função que recebe uma tabela e uma mensagem de pedido e:
	- aplica a operação na mensagem de pedido na tabela;
	- devolve uma mensagem de resposta com oresultado.
*/
struct message_t *process_message(struct message_t *msg_pedido, struct table_t *tabela){
	struct message_t *msg_resposta;

	/* Verificar parâmetros de entrada */
  if ( msg_pedido == NULL || tabela == NULL)
    return NULL;

  msg_resposta = malloc((struct message_t*) sizeof(struct message_t));
  if (msg_resposta == NULL)
    return NULL;

	/* Verificar opcode e c_type na mensagem de pedido */
  short opcode = msg_pedido->opcode;
  short c_type = msg_pedido->c_type;

	/* Aplicar operação na tabela */
  switch(opcode){
    case OC_SIZE:
      msg_resposta->opcode = OC_SIZE+1;
      msg_resposta->c_type = CT_RESULT;
      msg_resposta->content.result = table_size(tabela);
      if (msg_resposta->content.result == -1){
        perror("Operacao deu erro");
        return NULL;
      }
      break;
    case OC_UPDATE:
      msg_resposta->opcode = OC_UPDATE+1;
      msg_resposta->c_type = CT_RESULT;
      msg_resposta->content.result = table_update(tabela, msg_pedido->key, msg_pedido->data);
      if (msg_resposta->content.result == -1){
        perror("Operacao deu erro");
        return NULL;
      }
      break;
    case OC_GET:
      msg_resposta->opcode = OC_GET+1;
      if(strcmp(msg_pedido->key,"*")==0){
        msg_resposta->c_type = CT_KEYS;
        msg_resposta->result.keys = table_get_keys(table);
        if (msg_resposta->result.keys == NULL){
          perror("Operacao deu erro");
          return NULL;
        }
      }
      else{
        msg_resposta->c_type = CT_VALUE;
        msg_resposta->result.data = table_get(tabela, msg_pedido->key);
      }
      break;
    case OC_PUT:
      msg_resposta->opcode = OC_PUT+1;
      msg_resposta->c_type = CT_RESULT;
      msg_resposta->content.result = table_put(tabela, msg_pedido->key, msg_pedido->data);
      if (msg_resposta->content.result == -1){
        perror("Operacao deu erro");
        return NULL;
      }
      break;
    case OC_COLLS:
      msg_resposta->opcode = OC_COLLS+1;
      msg_resposta->c_type = CT_RESULT;
      msg_resposta->content.result = table_num_colls(tabela);
      if (msg_resposta->content.result == -1){
        perror("Operacao deu erro");
        return NULL;
      }
      break;
    default:
      return NULL;
  }
	/* Preparar mensagem de resposta */
	return msg_resposta;
}


/* Função "inversa" da função network_send_receive usada no table-client.
   Neste caso a função implementa um ciclo receive/send:

	Recebe um pedido;
	Aplica o pedido na tabela;
	Envia a resposta.
*/
int network_receive_send(int sockfd, struct table_t *tables){
  char *message_resposta, *message_pedido;
  int msg_length;
  int message_size, msg_size, result;
  struct message_t *msg_pedido, *msg_resposta;

	/* Verificar parâmetros de entrada */
  if(tables == NULL)
    return -1;
	/* Com a função read_all, receber num inteiro o tamanho da
	   mensagem de pedido que será recebida de seguida.*/
	result = read_all(sockfd, (char *) &msg_size, _INT);

	/* Verificar se a receção teve sucesso */
  if(result < 0)
    return -1;

	/* Alocar memória para receber o número de bytes da
	   mensagem de pedido. */
  msg_pedido = (struct message_t*) malloc (sizeof(struct message_t));

  if(msg_pedido == NULL)
    return -1;

	/* Com a função read_all, receber a mensagem de resposta. */
	result = read_all(sockfd, message_pedido, result);

	/* Verificar se a receção teve sucesso */
  if(result < 0){
    free(msg_pedido);
    return -1;
  }

	/* Desserializar a mensagem do pedido */
	msg_pedido = buffer_to_message(message_pedido,result);

	/* Verificar se a desserialização teve sucesso */
  if(msg_pedido == NULL){
    free(msg_pedido);
    return -1;
  }
	/* Processar a mensagem */
	msg_resposta = process_message(msg_pedido, tables[msg_pedido->table_num]);

	/* Serializar a mensagem recebida */
	message_size = message_to_buffer(msg_resposta, &message_resposta);

	/* Verificar se a serialização teve sucesso */
  if(message_size == -1){
    free(msg_pedido);
    return -1;
  }
	/* Enviar ao cliente o tamanho da mensagem que será enviada
	   logo de seguida
	*/
	msg_size = htonl(message_size);
 	result = write_all(server->sockfd, (char *) &msg_size, _INT));

	/* Verificar se o envio teve sucesso */
  if(result < 0){
    free(msg_pedido);
    return -1;
  }
	/* Enviar a mensagem que foi previamente serializada */
	result = write_all(server->sockfd, message_resposta, message_size));

	/* Verificar se o envio teve sucesso */
  if(result < 0){
    free(msg_pedido);
    return -1;
  }

	/* Libertar memória */
  free(msg_pedido);
	return 0;
}



int main(int argc, char **argv){
	int listening_socket, connsock, result;
	struct sockaddr_in client;
	socklen_t size_client;
	struct table_t *tables;

	if (argc < 3){
	printf("Uso: ./server <porta TCP> <port> <table1_size> [<table2_size> ...]\n");
	printf("Exemplo de uso: ./table-server 54321 10 15 20 25\n");
	return -1;
	}

	if ((listening_socket = make_server(atoi(argv[1]))) < 0) return -1;

	/*********************************************************/
	/* Criar as tabelas de acordo com linha de comandos dada */
	/*********************************************************/

	while ((connsock = accept(listening_socket, (struct sockaddr *) &client, &size_client)) != -1) {
		printf(" * Client is connected!\n");

		while (true){

			/* Fazer ciclo de pedido e resposta */
			network_receive_send(connsock, tables) < 0);

			/* Ciclo feito com sucesso ? Houve erro?
			   Cliente desligou? */

		}
	}
}
