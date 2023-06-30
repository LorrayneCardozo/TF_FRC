#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "chat.h"

// envia mensagem para o socket descritor
void send_message(int sd, int server_sd, int sala_id, int cliente_id) {
    printf("Enviando mensagem do file descriptor %d para a sala %d\n", sd, sala_id);
    for (int j = 0; j <= fdmax; j++)
        if (FD_ISSET(j, &salas[sala_id].sala_fd))
            if (j != sd && j != server_sd) {
                char mensagem[500] = "[";
                strcat(mensagem, salas[sala_id].clientes[cliente_id].nome);
                strcat(mensagem, "] => ");
                strcat(mensagem, buf);
                send(j, mensagem, 500, 0);
            }
}

// remove o descriptor da sala e da master
void exit_chat_room(int sd, int sala_id, int cliente_id, int remover) {
    salas[sala_id].clientes[cliente_id].status = 0;
    salas[sala_id].quantidade--;
    
    if (remover == 1){
        FD_CLR(sd, &master);
    }
    
    FD_CLR(sd, &salas[sala_id].sala_fd);
    if (salas[sala_id].quantidade == 0) {
        free(salas[sala_id].clientes);
        salas[sala_id].status = 0;
    }
    printf("O file descriptor %d saiu da sala %d\n", sd, sala_id);
}

// cria salas 
int create_chat_room(int limite) {
    int sala;
    for (sala = 0; sala < MAX_SALAS; sala++)
        if (salas[sala].status == 0)
            break;

    salas[sala].status = 1;
    salas[sala].limite = limite;
    salas[sala].clientes = malloc(limite * sizeof(cliente));
    for (int i = 0; i < limite; i++){
        salas[sala].clientes[i].status = 0;
    }
    printf("A sala %d foi criada com sucesso\n", sala);
    return sala;
}

// insere novo integrante no bate-papo
void insert_chat_room(int sd, int sala_id, char nome[], int tam_nome) {
    printf("File descriptor %d entrando na sala %d\n", sd, sala_id);
    FD_SET(sd, &salas[sala_id].sala_fd);
    salas[sala_id].quantidade++;
    for (int i = 0; i < salas[sala_id].limite; i++) {
        if (salas[sala_id].clientes[i].status == 0) {
            salas[sala_id].clientes[i].cliente_sd = sd;
            salas[sala_id].clientes[i].status = 1;
            strncpy(salas[sala_id].clientes[i].nome, nome, tam_nome);
            break;
        }
    }
}

// trata os comandos para: sair, trocar de sala, ou listar integrantes da sala
void control(int sd, int sala_id, int cliente_id) {
    buf[strlen(buf) - 2] = '\0';
    printf("Comando \"%s\" acionado na sala %d pelo file descriptor %d\n", buf, sd, sala_id);
    char resp_buf[256];

    if (strncmp(buf+1, "sair", 4) == 0) {
        printf("Desconectando file descriptor %d\n...", sd);
        strcpy(resp_buf, "Desconectado!\n");
        send(sd, resp_buf, strlen(resp_buf), 0);
        close(sd);
        exit_chat_room(sd, sala_id, cliente_id, 1);
    }

    if (strncmp(buf+1, "integrantes_conectados", 22) == 0) {
        send(sd, "\nIntegrantes conectados na sala:", 31, 0);
        for (int i = 0; i < salas[sala_id].limite; i++) {
            cliente c = salas[sala_id].clientes[i];
            if (c.status == 1 && c.cliente_sd != sd) {
                char nome[] = "\n";
                strcat(nome, c.nome);
                send(sd, nome, strlen(nome), 0);
            }
            else if (c.status == 1 && c.cliente_sd == sd) { 
                char nome[] = "\n[";
                strcat(nome, c.nome);
                strcat(nome, "]");
                send(sd, nome, strlen(nome), 0);
            }
        }
        send(sd, "\n\n", 2, 0);
    }

    if (strncmp(buf+1, "trocar_de_sala", 14) == 0) {
        recv(sd, buf, 256, 0);
        int nova_sala = atoi(buf);
        char nome[256];
        strcpy(nome, salas[sala_id].clientes[cliente_id].nome);
        exit_chat_room(sd, sala_id, cliente_id, 0);
        insert_chat_room(sd, nova_sala, nome, strlen(nome));
    }
}

// inicia servidor
void init_server() {
    for (int i = 0; i < MAX_SALAS; i++) {
        FD_ZERO(&salas[i].sala_fd);
        salas[i].limite = 0;
        salas[i].quantidade = 0;
        salas[i].status = 0;
    }
}

int main(int argc, char *argv[]) {
    
    if (argc < 3) {
        printf("Digite IP e Porta para este servidor\n");
        exit(1);
    }

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    init_server();
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = inet_addr(argv[1]);
    myaddr.sin_port = htons(atoi(argv[2]));
    
    memset(&(myaddr.sin_zero), 0, 8);
    bind(sd, (struct sockaddr *)&myaddr, sizeof(myaddr));
    listen(sd, 10);

    FD_SET(sd, &master);
    FD_SET(STDIN, &master);

    fdmax = sd;
    addrlen = sizeof(remoteaddr);

    int sala;

    for( ; ; ) {
        read_fds = master;
        select(fdmax+1, &read_fds, NULL, NULL, NULL);

        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == sd) {
                    newfd = accept(sd, (struct sockaddr *)&remoteaddr, &addrlen);
                    FD_SET(newfd, &master);

                    int limite, tam_nome;
                    char nome[256];
                    tam_nome = recv(newfd, nome, 256, 0);
                    tam_nome -= 2;
                    recv(newfd, buf, 256, 0);
                    sala = atoi(buf);
                    
                    if (sala == -1) {
                        recv(newfd, buf, 256, 0);
                        limite = atoi(buf);
                        sala = create_chat_room(limite);
                    }

                    insert_chat_room(newfd, sala, nome, tam_nome);

                    if (newfd > fdmax)
                        fdmax = newfd;
                }
                else {
                    memset(&buf, 0, sizeof(buf));
                    nbytes = recv(i, buf, sizeof(buf), 0);
                    int sala_id;
                    for (sala_id = 0; sala_id < MAX_SALAS; sala_id++)
                        if (FD_ISSET(i, &salas[sala_id].sala_fd))
                            break;

                    int cliente_id;
                    for (cliente_id = 0; cliente_id < salas[sala_id].limite; cliente_id++)
                        if (salas[sala_id].clientes[cliente_id].cliente_sd == i)
                            break;

                    if (nbytes == 0) {
                        printf("Desconectando fd: %d\n", i);
                        exit_chat_room(i, sala_id, cliente_id, 1);
                    }

                    if (buf[0] == '/')
                        control(i, sala_id, cliente_id);
                    else
                        send_message(i, sd, sala_id, cliente_id);
                }
            }
        }
    }

    return 0;
}
