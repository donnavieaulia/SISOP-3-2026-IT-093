#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "protocol.h"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    char name[NAME_LEN];
    printf("Enter your name: ");
    fgets(name, NAME_LEN, stdin);
    name[strcspn(name, "\n")] = 0;

    send(sock, name, strlen(name), 0);

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        FD_SET(sock, &readfds);

        select(sock + 1, &readfds, NULL, NULL, NULL);

        // TERIMA SERVER
        if (FD_ISSET(sock, &readfds)) {
            char buffer[BUFFER_SIZE];
            int valread = read(sock, buffer, BUFFER_SIZE);

            if (valread <= 0) {
                printf("Disconnected from server\n");
                break;
            }

            write(1, buffer, valread);

            if (strstr(buffer, "Enter Password") != NULL) {
                char pass[BUFFER_SIZE];
                fgets(pass, BUFFER_SIZE, stdin);
                send(sock, pass, strlen(pass), 0);
            }
        }

        // INPUT USER
        if (FD_ISSET(0, &readfds)) {
            char msg[BUFFER_SIZE];
            fgets(msg, BUFFER_SIZE, stdin);
            send(sock, msg, strlen(msg), 0);
        }
    }

    close(sock);
    return 0;
}
