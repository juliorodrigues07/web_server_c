#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT_NUMBER 2000
#define MAX_CONNECTIONS 16
#define HEADER_SIZE 1024
#define PATH_SIZE 128
#define BUFFER_SIZE 128

u_int32_t n_requests;
pthread_mutex_t n_control;
u_int32_t requests_queue[BUFFER_SIZE];
pthread_mutex_t queue_control[BUFFER_SIZE];
pthread_t threads[MAX_CONNECTIONS];

typedef struct response {
    char *response_buffer;
    u_int32_t message_length;
    int status;
} response;

typedef struct request {
    int bytes_read;
    char header[HEADER_SIZE];
} request;

void error(const char *msg);
int socket_creation(int n_connections);
request *read_header(int client_socket);
response *load_file(char *path);
response *http_parser(request *r);
void client_response(int client_socket_fd);
void iterative_server();
void fork_server();
void thread_server();
void consumer();