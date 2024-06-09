#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 9013
#define LOG_FILE "./serverlog.txt"

void daemonize() {
    pid_t pid;

    // Fork off the parent process
    pid = fork();

    // If we got a good PID, then we can exit the parent process.
    if (pid < 0) {
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Create a new SID for the child process
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
}

void printlog(const char *format, ...) {
    FILE *logfile;
    va_list args;
    time_t now;
    char timestamp[20];

    // open the file
    logfile = fopen(LOG_FILE, "a");
    if (!logfile) {
        perror("Log: Open failed");
        return;
    }

    // normalise the log message and write in
    time(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(logfile, "%s: ", timestamp);
    va_start(args, format);
    vfprintf(logfile, format, args);
    va_end(args);
    fprintf(logfile, "\n");
    fclose(logfile);
}

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    char buffer[1024] = {0};
    int valread;

    printlog("Socket %d: Connected", client_socket);

    while ((valread = read(client_socket, buffer, 1024)) > 0) {
        // Convert received message to uppercase
        for (int i = 0; buffer[i]; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        // Send the modified message back to the client
        send(client_socket, buffer, strlen(buffer), 0);

        // Log the received message
        printlog("Socket %d: Receive message \"%s\"", client_socket, buffer);

        // Clear the buffer for the next read
        memset(buffer, 0, sizeof(buffer));
    }

    if (valread < 0) {
        perror("read");
    }

    printlog("Socket %d: Disconnected", client_socket);
    close(client_socket);
    free(arg);

    return NULL;
}


int main() {

    printlog("Server: Start");
    daemonize();
    printlog("Server: Daemonized");

    int server_fd;
    int client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1; // to ensure that upon server termination and restart, previously used addr can be immediately reused
    pthread_t thread_id;

    // Define addr
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("create socket failed");
        printlog("Server: Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        printlog("Server: Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        printlog("Server: Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections (allowing a maximum of 3 sockets at the same time)
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        printlog("Server: Listen failed");
        exit(EXIT_FAILURE);
    }

    // Main server loop
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            printlog("Accept failed");
            continue;
        }

        if (pthread_create(&thread_id, NULL, handle_client, (void*)&client_socket) != 0) {
            perror("pthread_create");
            printlog("Pthread_create failed");
        } else {
            pthread_detach(thread_id);  // detach the thread to handle its own resources
        }
    }

    // Close the server socket
    close(server_fd);
    printlog("Server: End");
    return 0;
}
