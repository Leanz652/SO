#ifndef SERVER_UTILS_H_
#define SERVER_UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<string.h>
#include<shared_utils.h>

t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(char*, char*);
int esperar_cliente(int);
int recibir_operacion(int);

#endif /* SERVER_UTILS_H_ */
