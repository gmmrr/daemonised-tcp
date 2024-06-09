#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9013

int main() {

    printf("Client: Start\n");

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Define addr
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Client: Socket creation error\n");
        return -1;
    }

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Client: Invalid address/ Address not supported\n");
        return -1;
    }

    // Try connecting to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Client: Connection Failed\n");
        return -1;
    }
    printf("Client: Connect server [127.0.0.1:%d] success\n", PORT);

    // Main client loop
    while (1) {
        printf("\nPlease input your message: ");

        // Deal with the input
        fgets(buffer, 1024, stdin);
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') { // change newline notation into termination
            buffer[len - 1] = '\0';
        }

        // Send the message to the server
        send(sock, buffer, strlen(buffer), 0);

        // Clear the buffer for the next read
        memset(buffer, 0, sizeof(buffer));

        // Receive the message from the server
        int valread = read(sock, buffer, 1024);

        if (valread > 0) {
            printf("Client: Received message from [127.0.0.1:%d]: %s\n", PORT, buffer);
        } else if (valread == 0) {
            printf("Client: Server disconnected\n");
            break;
        } else {
            printf("Client: Read error\n");
            break;
        }

        // Clear the buffer for the next input
        memset(buffer, 0, sizeof(buffer));
    }

    // Close the client socket
    close(sock);
    printf("Client: End\n");
    return 0;
}
