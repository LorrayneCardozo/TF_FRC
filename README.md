# Fundamentos de Redes de Computadores (FRC) - 2023/1
## Trabalho Final
<hr>

## Alunos

- Gustave A. Persijn – 190046091
- Daniel Vinícius R. A. – 190026375
- Lorrayne A. Cardozo – 190032863

## Como rodar

### Servidor
Compile o código e inicie o servidor, com IP e Porta como argumentos.

```bash
$ gcc -o chat chat.c
$ ./chat <ip> <porta>
```

### Clientes
Após iniciar o servidor, conecte-se executando:

```bash
$ telnet <ip> <porta>
```
Em seguida, insira seu nome no terminal para ser identificado.
```
$ Fernando
```

Insira -1 para criar uma nova sala, e em seguida, o limite máximo de integrantes na sala.
```bash
$ -1
$ 8
```

Por fim, pode enviar mensagens no chat.
```bash
$ Bom dia!
```
Para listar os integrantes conectados na sala, execute:
```bash
$ /integrantes_conectados
```
Para trocar de sala, execute o comando e o número da sala em que deseja ser inserido:
```bash
$ /trocar_de_sala
$ 4
```
E, por fim, para sair de uma sala:
```bash
$ /sair
```