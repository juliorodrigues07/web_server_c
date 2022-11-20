#include "server.h"

int main(int argc, char **argv) {

    if (argc < 2 || argc >= 3)
        error("USAGE: ./main <option>\n");

    int option = atoi(argv[1]);

    switch (option) {
        case 1:
            iterative_server();
            break;
        case 2:
            fork_server();
            break;
        case 3:
            thread_server();
            break;
        case 4:
            concurrent_server();
            break;
        default:
            printf("ERROR: Invalid Option!\n");
            break;
    }

    //test_parser();
    return 0;
}
