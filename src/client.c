#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <poll.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    struct pollfd fds[1];

    int sockfd;
    int len, port;
    struct sockaddr_in address;
    int result = 0;
    char ch;
    port = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);
    len = sizeof(address);

    
    int flags = 0;

    fcntl(sockfd, F_SETFL, O_NONBLOCK);
     

    if (connect(sockfd, (struct sockaddr *)&address, len) == -1) {
            perror("oops: client failed to connect");
            //return 1;
        }

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    while(1)
    {
        printf("Enter Character To Send The Server OR Q/q To End The  Connection:");
        fflush(stdin);
        scanf("%c", &ch);

        if (ch == 'Q' || ch == 'q'){
            printf("Connection Closed");
            break;
        }

        write(sockfd, &ch, 1);
        
        poll(fds,1,10);

        if(fds[0].revents & POLLIN)
        {
       
            result = read(sockfd, &ch, 1);
        }

        printf("Incremented character from server = %c\n", ch);
        printf("Incremented character from server = %d\n", ch);
    }

    close(sockfd);
    return 0;
}
