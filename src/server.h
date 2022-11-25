#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>

// CONSTANTES (máximo de conexões, tamanho máximo da linha de requisição HTTP (GET), URL e buffer)
#define MAX_CONNECTIONS 16
#define HEADER_SIZE 1024
#define PATH_SIZE 128
#define BUFFER_SIZE 128

// MENSAGENS DE TESTE
#define TEST1 "GET ../files/m.png HTTP/1.1"
#define TEST2 "GET ../files/cat.jpg HTTP/1.1"
#define TEST3 "GT ../files/f1.gif HTTP/1.1"

// CONTROLE DAS THREADS
pthread_mutex_t n_control;
u_int32_t requests_queue[BUFFER_SIZE];
pthread_mutex_t queue_control[BUFFER_SIZE];
pthread_t threads[MAX_CONNECTIONS];

// RESPOSTA
typedef struct response {
    char *response_buffer;
    u_int32_t message_length;
    int status;
} response;

// REQUISIÇÃO
typedef struct request {
    int bytes_read;
    char header[HEADER_SIZE];
} request;

// ACUSAÇÃO DE ERROS E CRIAÇÃO DAS CONEXÕES (SOCKETS)
void error(const char *msg);
int socket_creation(int port_number, int n_connections);

// MANIPULAÇÃO DAS MENSAGENS HTTP
request *read_header(int client_socket);
response *load_file(char *path);
response *http_parser(request *r);
void client_response(int client_socket_fd);

// SERVIDORES
void iterative_server(int port_number);
void fork_server(int port_number);
void consumer();
void thread_server(int port_number);
void concurrent_server(int port_number);

// TESTE AUXILIAR
void test_parser();
