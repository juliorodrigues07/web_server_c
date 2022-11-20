#include "server.h"

int main(int argc, char **argv) {

    int option = atoi(argv[1]);

    if (argc < 2)
        error("USAGE: ./main <option>\n");

    switch (option) {
        case 1:
            printf("eu\n");
            iterative_server();
            break;
        case 2:
            printf("tu\n");
            //fork_server();
            break;
        case 3:
            printf("ele\n");
            //thread_server();
            break;
        case 4:
            printf("nós\n");
            //select_server();
            break;
        default:
            printf("Invalid Option!\n");
            break;
    }

    // Teste
    request *r = (request *) malloc(sizeof(request));
    response *a = (response *) malloc(sizeof(response));

    r->bytes_read = 0;
    strcpy(r->header, "GET ../files/m.png HTTP/1.1");

    a = http_parser(r);
    printf("%s\n", a->response_buffer);
    return 0;
}
