#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "person.h"
#include "socket.h"

#define PORT 12345
#define MAX_CLIENTS 10
#define BUF_SIZE 1024

extern Person person_list[MAX_PERSON];
extern int person_count;

static int server_fd;

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
} client_info;

void* handle_client(void* arg) {
    client_info* client = (client_info*)arg;
    char buffer[BUF_SIZE];

    printf("Client connected: %s:%d\n",
        inet_ntoa(client->addr.sin_addr),
        ntohs(client->addr.sin_port));

    while (1) {
        int bytes = recv(client->sockfd, buffer, BUF_SIZE, 0);
        if (bytes <= 0) {
            printf("Client disconnected: %s:%d\n",
                inet_ntoa(client->addr.sin_addr),
                ntohs(client->addr.sin_port));
            break;
        }

        buffer[bytes] = '\0';
        printf("Client: %s\n", buffer);
        if(buffer[0] == 'N'){
            char msg_buf[1024];
            Person p;
            char name[10] = {0};
            memcpy(p.passkey, buffer+1, 6);

            char* existing = checkCredentials(p.passkey);
            if(existing != NULL){
                int bytes = snprintf(msg_buf, sizeof(msg_buf), "Passkey already exists\n");
                send(client->sockfd, msg_buf, bytes, 0);
                continue;
            }

            strncpy(name, buffer+7, sizeof(name));
            if(name[strlen(name)-1] == '\n'){
                name[strlen(name)-1] = '\0';
            }
            p.name = name;
            add_person(p);
            int bytes = snprintf(msg_buf, sizeof(msg_buf), "User %s added\n", name);
            send(client->sockfd, msg_buf, bytes, 0);
        }
        else if(buffer[0] == 'L'){
            char msg_buf[1024];

            for (uint i = 0; i < person_count; i++) {
                int bytes = snprintf(msg_buf, sizeof(msg_buf), "%s: %s\n", 
                        person_list[i].passkey, person_list[i].name);
                if (send(client->sockfd, msg_buf, bytes, 0) < 0) {
                    perror("send");
                    break;
                }
            }

            int bytes = snprintf(msg_buf, sizeof(msg_buf), "END\n");
            send(client->sockfd, msg_buf, bytes, 0);
        }
        else if(buffer[0] == 'H'){
            FILE *fp = fopen(UNLOCKLOGFILE, "r");
            if (!fp) {
                perror("fopen");
                const char *err = "ERROR: Cannot open file\n";
                send(client->sockfd, err, strlen(err), 0);
            } else {
                char line[1024];
                while (fgets(line, sizeof(line), fp)) {
                    send(client->sockfd, line, strlen(line), 0);
                }
                fclose(fp);

                send(client->sockfd, "END\n", 4, 0);
            }
        }
        else{
            bytes = sprintf(buffer, "Invalid command\n");
            send(client->sockfd, buffer, bytes, 0);
        }

        // Echo back
    }

    close(client->sockfd);
    free(client);
    return NULL;
}

void* listener_thread(void* arg) {
    while (1) {
        client_info* client = malloc(sizeof(client_info));
        socklen_t addr_len = sizeof(client->addr);

        client->sockfd = accept(server_fd, (struct sockaddr*)&client->addr, &addr_len);
        if (client->sockfd < 0) {
            perror("accept");
            free(client);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client);
        pthread_detach(tid);
    }
    return NULL;
}

int startSocket(){
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen"); exit(1);
    }

    printf("Server listening on port %d...\n", PORT);
    pthread_t listener_tid;
    pthread_create(&listener_tid, NULL, listener_thread, NULL);
    return server_fd;
}

