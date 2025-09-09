#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 12345
#define BUF_SIZE 1024

int sockfd;

void handle_list(char *mode) {
    FILE* sock_file = fdopen(sockfd, "r");
    if (!sock_file) {
        perror("fdopen");
        return;
    }
    
    send(sockfd, mode, 1, 0);
    
    char line[1024];
    
    while (fgets(line, sizeof(line), sock_file)) {
        line[strcspn(line, "\n")] = '\0';
        
        if (strcmp(line, "END") == 0) {
            break;
        }
        
        printf("%s\n", line);
    }
}

void handle_new() {
    char passkey[7], name[10], combined[200];

    char *line = NULL;
    size_t len = 0;
    printf("Enter 6-letter Passkey(0-9)(A-D): ");
    getline(&line, &len, stdin);
    if(strlen(line) != 7){
        printf("Please enter 6 digit passkey\n");
        return;
    }
    line[6] = '\0';

    for(uint i=0; i<strlen(line); i++){
        char c = line[i];
        if(isdigit(c)) continue;
        if(c>='A' && c <= 'D') continue;
        printf("Please enter (0-9)(A-D)\n");
        return;
    }
    strcpy(passkey, line);
    printf("Please enter your first name: ");
    getline(&line, &len, stdin);
    line[strlen(line)-1] = '\0';
    for(uint i=0; i<strlen(line); i++){
        if(isalnum(line[i])) continue;
        printf("Please enter (0-9)(A-Z)(a-z)\n");
        return;
    }
    strncpy(name, line, sizeof(name));

    sprintf(combined, "N%s%s\n", passkey, name);
    send(sockfd, combined, strlen(combined), 0);

    // Wait for response
    char buffer[BUF_SIZE];
    int bytes = recv(sockfd, buffer, BUF_SIZE - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("Server: %s", buffer);
    }
}

int main(int argc, char* argv[]) {
    if(argc != 2){
        printf("Usage ./client <ip>\n");
        return 1;
    }
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect"); exit(1);
    }

    printf("Connected to server.\n");

    char input[BUF_SIZE];

    while (1) {
        printf("\n> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) break;

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "LIST") == 0) {
            handle_list("L");
        } 
        else if (strcmp(input, "NEW") == 0) {
            handle_new();
        }
        else if (strcmp(input, "HISTORY") == 0) {
            handle_list("H");
        }
        else {
            printf("Invalid command. Use LIST, HISTORY or NEW.\n");
        }
    }

    close(sockfd);
    return 0;
}
