#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

#include "protocol.h"

typedef struct {
    int sock;
    char name[NAME_LEN];
    int is_admin;
} Client;

Client clients[MAX_CLIENTS];

void log_event(char *type, char *msg) {
    FILE *f = fopen("history.log", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        t->tm_year+1900, t->tm_mon+1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        type, msg);

    fclose(f);
}

int name_exists(char *name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock != 0 && strcmp(clients[i].name, name) == 0)
            return 1;
    }
    return 0;
}

void broadcast(char *msg, int sender_sock) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock != 0 && clients[i].sock != sender_sock) {
            send(clients[i].sock, msg, strlen(msg), 0);
        }
    }
}

int main() {
    int server_fd, new_socket, max_sd, sd;
    struct sockaddr_in address;
    fd_set readfds;

    for (int i = 0; i < MAX_CLIENTS; i++) clients[i].sock = 0;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(HOST);
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("[System] SERVER ONLINE\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].sock;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // NEW CONNECTION
        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd, NULL, NULL);

            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);
            int val = read(new_socket, buffer, BUFFER_SIZE);

            if (val <= 0) {
                close(new_socket);
                continue;
            }

            buffer[strcspn(buffer, "\n")] = 0;

            if (name_exists(buffer)) {
                send(new_socket, "[System] Name already taken\n", 30, 0);
                close(new_socket);
                continue;
            }

            int is_admin = 0;

            // ADMIN LOGIN
            if (strcmp(buffer, "The Knights") == 0) {
                is_admin = 1;

                send(new_socket, "Enter Password:\n", 16, 0);

                char pass[BUFFER_SIZE];
                read(new_socket, pass, BUFFER_SIZE);

                char fullmsg[BUFFER_SIZE];
                sprintf(fullmsg,
                    "[System] Authentication Successful. Granted Admin privileges.\n"
                    "\n=== THE KNIGHTS CONSOLE ===\n"
                    "1. Check Active Entities (Users)\n"
                    "2. Check Server Uptime\n"
                    "3. Execute Emergency Shutdown\n"
                    "4. Disconnect\n"
                    "Command >> ");

                send(new_socket, fullmsg, strlen(fullmsg), 0);
            } 
            else {
                char welcome[BUFFER_SIZE];
                sprintf(welcome, "--- Welcome to The Wired, %s ---\n", buffer);
                send(new_socket, welcome, strlen(welcome), 0);
            }

            // SIMPAN CLIENT
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].sock == 0) {
                    clients[i].sock = new_socket;
                    strcpy(clients[i].name, buffer);
                    clients[i].is_admin = is_admin;
                    break;
                }
            }

            char logmsg[BUFFER_SIZE];
            sprintf(logmsg, "User '%s' connected", buffer);
            log_event("System", logmsg);
            printf("[System] %s\n", logmsg);
        }

        // HANDLE MESSAGE
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].sock;

            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                char buffer[BUFFER_SIZE];
                int valread = read(sd, buffer, BUFFER_SIZE);

                if (valread <= 0) {
                    char msg[BUFFER_SIZE];
                    sprintf(msg, "User '%s' disconnected", clients[i].name);

                    printf("[System] %s\n", msg);
                    log_event("System", msg);

                    close(sd);
                    clients[i].sock = 0;
                } else {
                    buffer[strcspn(buffer, "\n")] = 0;

                    // ADMIN MODE
                    if (clients[i].is_admin) {
                        if (strcmp(buffer, "1") == 0) {
                            send(sd, "Active Users:\n", 14, 0);

                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (clients[j].sock != 0) {
                                    char user[BUFFER_SIZE];
                                    sprintf(user, "- %s\n", clients[j].name);
                                    send(sd, user, strlen(user), 0);
                                }
                            }
                        }
                        else if (strcmp(buffer, "2") == 0) {
                            send(sd, "Server Uptime feature not implemented yet\n", 44, 0);
                        }
                        else if (strcmp(buffer, "3") == 0) {
                            send(sd, "[System] EMERGENCY SHUTDOWN INITIATED\n", 40, 0);
                            exit(0);
                        }
                        else if (strcmp(buffer, "4") == 0) {
                            close(sd);
                            clients[i].sock = 0;
                            continue;
                        }

                        // tampilkan menu lagi
                        char menu[] =
                            "\n=== THE KNIGHTS CONSOLE ===\n"
                            "1. Check Active Entities (Users)\n"
                            "2. Check Server Uptime\n"
                            "3. Execute Emergency Shutdown\n"
                            "4. Disconnect\n"
                            "Command >> ";

                        send(sd, menu, strlen(menu), 0);
                        continue;
                    }

                    // USER BIASA
                    if (strcmp(buffer, "/exit") == 0) {
                        close(sd);
                        clients[i].sock = 0;
                        continue;
                    }

                    char msg[BUFFER_SIZE];
                    sprintf(msg, "[%s]: %s\n", clients[i].name, buffer);

                    printf("%s", msg);
                    log_event("User", msg);
                    broadcast(msg, sd);
                }
            }
        }
    }

    return 0;
}
