#include "server.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int socket_creation(int n_connections){

    bool check = true;
    struct sockaddr_in serv_addr;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
        error("ERROR: Opening!\n");

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &check, sizeof(int)) == -1)
        error("ERROR: Already in use!\n");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER);

    if (bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        close(socket_fd);
        error("ERROR: Binding!\n");
    }

    if (listen(socket_fd, n_connections) < 0)
        error("ERROR: Listening!\n");

    return socket_fd;
}

response *load_file(char *path){

    FILE *reader = fopen(path, "r");
    response *file = (response *) malloc(sizeof(response));

    // Caso em que o arquivo não é encontrado (404)
    if (reader == NULL){

        printf("ERROR: Not found!\n");
        file->status = 404;

        reader = fopen ("../files/404.html", "r");
        fseek(reader, 0, SEEK_END);

        file->message_length = ftell(reader);
        file->response_buffer = malloc(file->message_length);

        rewind(reader);
        fread(file->response_buffer, 1, file->message_length, reader);

        fclose(reader);
        return file;
    }

    fseek(reader, 0, SEEK_END);
    file->message_length = ftell(reader);
    file->response_buffer = malloc(file->message_length);

    // Caso em que não existe memória suficiente para o arquivo (413)
    if (file->response_buffer == NULL){

        printf("ERROR: Payload too large!\n");
        file->status = 413;
        fclose(reader);

        reader = fopen("../files/413.html", "r");
        fseek(reader, 0, SEEK_END);

        file->message_length = ftell(reader);
        file->response_buffer = malloc (file->message_length);

        rewind(reader);
        fread (file->response_buffer, 1, file->message_length, reader);

        fclose(reader);
        return file;
    }

    // Caso geral: carrega o arquivo solicitado
    rewind(reader);
    fread(file->response_buffer, 1, file->message_length, reader);
    fclose(reader);

    file->status = 200;
    return file;
}

response *http_parser(request *r){

    bool check = true;
    int i, j, h_size;

    // Arrays para manipular as mensagens
    char method[4];
    char header[128] = {0};

    // -9 -> Bytes warning -> '../files/'
    char path[PATH_SIZE - 9] = {0};

    response *file;
    response *answer = (response *) malloc(sizeof(response));

    // Checa se o método da linha de requisição é o GET
    for (i = 0; r->header[i] != ' '; i++)
        method[i] = r->header[i];
    method[i] = r->header[i];

    if (strcmp(method, "GET ") != 0)
        check = false;

    // Caso em que a requisição está bem formatada
    if (check) {
        i++;
        j = 0;

        // Obtém o caminho do arquivo na url
        while (r->header[i] != ' ') {

            if (j > 0 || r->header[i] != '/') {
                path[j] = r->header[i];
                j++;
            }
            i++;
        }

        DIR *d = opendir(path);

        // Caso em que o arquivo requisitado é index.html ou outro qualquer
        if (path[0] == '/' && path[1] == 0 || path[0] == 0 || d != NULL) {
            closedir(d);
            file = load_file("../files/index.html");
        } else {
            // Caso em que o caminho não possui o prefixo '../files/'
            if ((int) strncmp("../files/", path, 9)) {
                char tmp[PATH_SIZE] = {0};
                sprintf(tmp, "../files/%s", path);
                file = load_file(tmp);
            } else
                file = load_file(path);
        }
    } else {
        file = (response *) malloc(sizeof(response));
        printf("ERROR: Bad Request!\n");
        file->status = 400;

        FILE *reader = fopen ("../files/400.html", "r");
        fseek(reader, 0, SEEK_END);

        file->message_length = ftell(reader);
        file->response_buffer = malloc(file->message_length);

        rewind(reader);
        fread(file->response_buffer, 1, file->message_length, reader);
        fclose(reader);
    }

    // Geração das respostas
    if (file->status == 404)
        sprintf(header, "HTTP/1.1 404 Not Found\nContent-Length: %d\r\n\r\n", file->message_length);
    else if (file->status == 413)
        sprintf(header, "HTTP/1.1 413 Payload Too Large\nContent-Length: %d\r\n\r\n", file->message_length);
    else if (file->status == 200)
        sprintf(header, "HTTP/1.1 200 OK\nContent-Length: %d\r\n\r\n", file->message_length);
    else if (file->status == 400)
        sprintf(header, "HTTP/1.1 400 Bad Request\nContent-Length: %d\r\n\r\n", file->message_length);

    h_size = (int) strlen(header);
    answer->response_buffer = malloc(h_size + file->message_length);

    memcpy(answer->response_buffer, header, h_size);
    memcpy(h_size + answer->response_buffer, file->response_buffer, file->message_length);
    answer->message_length = h_size + file->message_length;

    free(file->response_buffer);
    free(file);

    return answer;
}

request *read_header(int client_socket){

    request *r = (request *) malloc(sizeof(request));

    r->bytes_read = 0;
    r->bytes_read = (int) read(client_socket, r->header, HEADER_SIZE);

    return r;
}

void client_response(int client_socket_fd){

    // Recebe a linha de requisição da mensagem HTTP para realizar o parsing
    request *r = read_header(client_socket_fd);
    struct response *answer = http_parser(r);

    write(client_socket_fd, answer->response_buffer, answer->message_length);
    close(client_socket_fd);

    // Exibe cada requisição e resposta HTTP correspondente
    /*printf("%s\n", r->header);
    printf("%s\n", answer->response_buffer);*/

    free(answer->response_buffer);
    free(answer);
    free(r);
}

void iterative_server(){

    // Variáveis para comunicação em rede (sockets)
    int client_socket_fd;
    struct sockaddr_in client;
    int size_socket_addr = sizeof(struct sockaddr_in);

    // Somente uma requisição por vez
    int socket_fd = socket_creation(1);

    // LOOP INFINITO: Servidor recebendo requisições indefinidamente
    while (true) {
        client_socket_fd = accept(socket_fd, (struct sockaddr *) &client, (socklen_t *) &size_socket_addr);

        if (client_socket_fd < 0)
            error("ERROR: Accept!");

        client_response(client_socket_fd);
    }

    close(socket_fd);
}

void fork_server() {

    //int statloc;
    int client_socket_fd;
    struct sockaddr_in client;
    int size_socket_addr = sizeof(struct sockaddr_in);
    int socket_fd = socket_creation(1);
    pid_t child;

    // LOOP INFINITO: Servidor recebendo requisições indefinidamente
    while (true) {
        client_socket_fd = accept(socket_fd, (struct sockaddr *) &client, (socklen_t *) &size_socket_addr);

        if (client_socket_fd < 0)
            error("ERROR: Accept!\n");

        if ((child = fork()) < 0)
            error("ERROR: Fork!\n");

        // Processo filho processa a resposta da requisição HTTP do cliente
        if (child == 0) {
            client_response(client_socket_fd);
            sleep(5);
            kill(child, SIGKILL);
        }
        /*else {
            if ((waitpid(child, &statloc, 0)) == -1)
                error("ERROR: Waitpid!\n");
        }*/

        close(client_socket_fd);
    }

    // TODO: fork control (Memory leak -> zombie processes)
    close(socket_fd);
}

void consumer() {

    while (true) {

        for (int i = 0; i < MAX_CONNECTIONS; i++) {

            // Caso em que exista requisições na fila para serem respondidas
            if (!pthread_mutex_trylock(&queue_control[i])) {

                if (requests_queue[i] != -1) {
                    client_response((int) requests_queue[i]);

                    // Marcação da requisição como já atendida
                    requests_queue[i] = -1;
                }
                pthread_mutex_unlock(&queue_control[i]);
            } else if (!pthread_mutex_trylock(&queue_control[BUFFER_SIZE - 1 - i])) {

                if (requests_queue[BUFFER_SIZE - 1 - i] != -1){
                    client_response((int) requests_queue[BUFFER_SIZE - 1 - i]);
                    requests_queue[BUFFER_SIZE - 1 - i] = -1;
                }
                pthread_mutex_unlock(&queue_control[BUFFER_SIZE - 1 - i]);
            }
        }
        // Caso em que não há requisições para atender -> inativo por 1s
        sleep(1);
    }
}

void thread_server() {

    int client_socket_fd;
    struct sockaddr_in client;
    int size_socket_addr = sizeof(struct sockaddr_in);

    // Várias conexões pararelas
    int socket_fd = socket_creation(MAX_CONNECTIONS);

    // Fila de requisições
    for (int i = 0; i < BUFFER_SIZE; i++)
        requests_queue[i] = -1;

    // Criação das threads para consumir
    pthread_mutex_init(&n_control, NULL);
    for (int i = 0; i < MAX_CONNECTIONS; i++) {

        pthread_mutex_init(&queue_control[i], NULL);
        if ((int) pthread_create(&threads[i], NULL, (void *) &consumer, NULL) < 0)
            error("ERROR: Thread!\n");
    }

    // LOOP INFINITO: Servidor recebendo requisições indefinidamente
    while (true) {

        client_socket_fd = accept(socket_fd, (struct sockaddr *) &client, (socklen_t *) &size_socket_addr);

        if (client_socket_fd < 0)
            error("ERROR: Accept!\n");

        for (int i = 0; i < BUFFER_SIZE; i++) {

            // Caso em que a requisição atual já foi atendida
            if (requests_queue[i] == -1) {

                if (!pthread_mutex_trylock(&queue_control[i])) {
                    requests_queue[i] = client_socket_fd;
                    pthread_mutex_unlock(&queue_control[i]);
                    break;
                }
            } else if (requests_queue[BUFFER_SIZE - 1 - i] == -1) {

                if (!pthread_mutex_trylock(&queue_control[BUFFER_SIZE - 1 - i])) {
                    requests_queue[BUFFER_SIZE - 1 - i] = client_socket_fd;
                    pthread_mutex_unlock(&queue_control[BUFFER_SIZE - 1 - i]);
                    break;
                }
            }
        }
    }

    // TODO: Unreachable?
    close(socket_fd);
    pthread_mutex_destroy(&n_control);
    for (int i = 0; i < MAX_CONNECTIONS; i++)
        pthread_mutex_destroy(&queue_control[i]);
}

void concurrent_server() {

    int client_socket_fd, max_socket, activity;
    int size_socket_addr = sizeof(struct sockaddr_in);
    int socket_fd = socket_creation(MAX_CONNECTIONS);
    struct sockaddr_in client;

    fd_set master, reader;
    FD_ZERO(&master); FD_ZERO(&reader);

    FD_SET(socket_fd, &master);
    max_socket = socket_fd;

    // LOOP INFINITO: Servidor recebendo requisições indefinidamente
    while (true) {

        // Espera por novas requisições
        reader = master;
        activity = select(max_socket + 1, &reader, NULL, NULL, NULL);

        if (activity < 0)
            error("ERROR: Select!\n");

        for (int i = 0; i <= max_socket; i++) {

            // Caso em que alguma mensagem foi escrita no socket i
            if (FD_ISSET(i, &reader)) {

                // Caso em que i é o socket do servidor para receber novas requisições
                if (i == socket_fd) {

                    client_socket_fd = accept(socket_fd, (struct sockaddr *) &client, (socklen_t *) &size_socket_addr);

                    if (client_socket_fd < 0)
                        error("ERROR: Accept!\n");

                    FD_SET(client_socket_fd, &master);
                    if (client_socket_fd > max_socket)
                        max_socket = client_socket_fd;
                } else {
                    client_response(i);
                    FD_CLR(i, &master);
                }
            }
        }
    }
}

void test_parser() {

    // Teste do parser isolado
    request *r = (request *) malloc(sizeof(request));
    response *a = (response *) malloc(sizeof(response));

    r->bytes_read = 0;
    strcpy(r->header, TEST3);

    a = http_parser(r);
    printf("%s\n", a->response_buffer);
}
