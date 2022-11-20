#include "server.h"

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

    rewind(reader);
    fread(file->response_buffer, 1, file->message_length, reader);
    fclose(reader);

    file->status = 200;
    return file;
}

response *http_parser (request *r){

    bool check = true;
    int i, j, h_size;
    char method[4];
    char header[128] = {0};
    char path[PATH_SIZE - 9] = {0};

    response *file;
    response *answer = (response *) malloc(sizeof(response));

    for (i = 0; r->header[i] != ' '; i++)
        method[i] = r->header[i];
    method[i] = r->header[i];

    if (strcmp(method, "GET ") != 0)
        check = false;

    if (check) {
        i++;
        j = 0;

        while (r->header[i] != ' ') {

            if (j > 0 || r->header[i] != '/') {
                path[j] = r->header[i];
                j++;
            }
            i++;
        }

        DIR *d = opendir(path);

        if (path[0] == '/' && path[1] == 0 || path[0] == 0 || d != NULL) {
            closedir(d);
            file = load_file("../files/404.html");
        } else {
            if (strncmp("../files/", path, 9)) {
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

    if (file->status == 404)
        sprintf(header, "HTTP/1.1 404 Not Found\nContent-Lenght: %d\r\n\r\n", file->message_length);
    else if (file->status == 413)
        sprintf(header, "HTTP/1.1 413 Payload Too Large\nContent-Lenght: %d\r\n\r\n", file->message_length);
    else if (file->status == 200)
        sprintf(header, "HTTP/1.1 200 OK\nContent-Lenght: %d\r\n\r\n", file->message_length);
    else if (file->status == 400)
        sprintf(header, "HTTP/1.1 400 Bad Request\nContent-Lenght: %d\r\n\r\n", file->message_length);

    h_size = strlen(header);
    answer->response_buffer = malloc(h_size + file->message_length);

    memcpy (answer->response_buffer, header, h_size);
    memcpy (h_size + answer->response_buffer, file->response_buffer, file->message_length);
    answer->message_length = h_size + file->message_length;

    free (file->response_buffer);
    free (file);

    return answer;
}

request *read_header(int client_socket){

    request *r = (request *) malloc(sizeof(request));

    r->bytes_read = 0;
    r->bytes_read = read(client_socket, r->header, HEADER_SIZE);

    return r;
}

void client_response(int client_socket_fd){

    request *r = read_header(client_socket_fd);
    printf("%s\n", r->header);
    struct response *answer = http_parser(r);

    write(client_socket_fd, answer->response_buffer, answer->message_length);
    close(client_socket_fd);

    free(answer->response_buffer);
    free(answer);
    free(r);
}

void iterative_server(){

    int client_socket_fd;
    struct sockaddr_in client;
    int size_socket_addr = sizeof(struct sockaddr_in);
    int socket_fd = socket_creation(1);

    while (true) {
        client_socket_fd = accept(socket_fd, (struct sockaddr *) &client, (socklen_t *) &size_socket_addr);

        if (client_socket_fd < 0)
            error("ERROR: Accept!");

        client_response(client_socket_fd);
    }

    close(socket_fd);
}

void fork_server() {

    int client_socket_fd, statloc;
    struct sockaddr_in client;
    int size_socket_addr = sizeof(struct sockaddr_in);
    int socket_fd = socket_creation(1);
    pid_t child;

    while (true) {
        client_socket_fd = accept(socket_fd, (struct sockaddr *) &client, (socklen_t *) &size_socket_addr);

        if (client_socket_fd < 0)
            error("ERROR: Accept!\n");

        if ((child = fork()) < 0)
            error("ERROR: Fork!\n");

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
    close(socket_fd);
}

void thread_server() {


}
