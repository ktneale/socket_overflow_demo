/*
    C socket server example
*/
#include <stdlib.h>
#include<stdio.h>
#include<string.h>              //strlen
#include<sys/socket.h>
#include<arpa/inet.h>           //inet_addr
#include<unistd.h>              //write
#include <fcntl.h>
#include <memory.h>

int main(int argc, char *argv[])
{
    int socket_desc, client_sock, c, read_size;
    struct sockaddr_in server, client;
    char client_message[2000] = { 0 };

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    int yes = 1;
    if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
    }
    //Bind
    if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    client_sock =
        accept(socket_desc, (struct sockaddr *) &client,
               (socklen_t *) & c);
    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    int i = 0;

    //Receive a message from client
    while (1) {
        char buffer[255] = { 0 };

        memset(buffer, 0, 255);
        snprintf(buffer, 255,
                 "Test message, would be an alert from some process: %d\n",
                 i);

        int ret = write(client_sock, buffer, strlen(buffer));
        printf("ret: %d\n", ret);

        //sleep(1);
        i++;
    }

    if (read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    close(socket_desc);

    return 0;
}
