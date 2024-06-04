#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>


#define PORT 9013

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connect server [127.0.0.1:%d] success\n", PORT);

    while (1) {
        printf("Please input your message: ");
        fgets(buffer, 1024, stdin);

        // Remove the newline character from fgets
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        send(sock, buffer, strlen(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
        read(sock, buffer, 1024);

        // Convert received message to uppercase
        for (int i = 0; buffer[i]; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        printf("get receive message from [127.0.0.1:%d]: %s\n", PORT, buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    return 0;
}
