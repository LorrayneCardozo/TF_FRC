#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#ifndef SERVER_H
#define SERVER_H

#define STDIN 0
#define MAX_SALAS 100

fd_set  master, read_fds;
struct  sockaddr_in myaddr, remoteaddr;
int     fdmax, newfd, nbytes, yes = 1, addrlen;
char    buf[256];

typedef struct {
    int cliente_sd;
    char nome[256];
    int status;
} cliente;

typedef struct {
    fd_set sala_fd;
    int limite;
    int quantidade;
    int status;
    cliente *clientes;
} sala;

sala salas[MAX_SALAS];

void init_server();
void send_message(int sd, int server_sd, int sala_id, int cliente_id);
void exit_chat_room(int sd, int sala_id, int cliente_id, int sair);
int create_chat_room(int limite);
void insert_chat_room(int sd, int sala_id, char nome[], int tam_nome);
void control(int sd, int sala_id, int cliente_id);

#endif