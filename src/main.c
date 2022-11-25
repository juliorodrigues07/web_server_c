#include "server.h"

int main(int argc, char **argv) {

    if (argc < 3 || argc >= 4)
        error("USAGE: ./main <option> <port number>\n");

    int option = atoi(argv[1]);
    int port_number = atoi(argv[2]);

    if (port_number < 1024 || port_number > 65536)
        error("Port number must be an integer greater than 1023 and less than 65536!\n");

    switch (option) {
        case 1:
            iterative_server(port_number);
            break;
        case 2:
            fork_server(port_number);
            break;
        case 3:
            thread_server(port_number);
            break;
        case 4:
            concurrent_server(port_number);
            break;
        default:
            printf("ERROR: Invalid Option!\n");
            break;
    }

    //test_parser();
    return 0;
}
