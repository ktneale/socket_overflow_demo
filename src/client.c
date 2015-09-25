#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>


typedef struct {
    uint8_t *ptr;
    uint8_t *head;              //first byte of the buffer
    uint8_t *tail;              //Pointer to the last occuppied byte in the buffer
    uint8_t *end;               //Pointer to the last byte of the buffer

    unsigned int size;
    unsigned int occupied;
} circular_buffer_t;


void buffer_init(circular_buffer_t * buffer, unsigned int size)
{
    if (!buffer) {
        fprintf(stderr, "Error! Bad buffer pointer\n");
        return;
    }

    buffer->ptr = (uint8_t *) malloc(size);

    if (!buffer->ptr) {
        fprintf(stderr, "Error! Bad allocation\n");
        return;
    }

    memset(buffer->ptr, 0, size);

    buffer->head = buffer->ptr;
    buffer->tail = buffer->ptr;

    //The value of this ptr is larger than the end of the linear buffer.
    buffer->end = buffer->head + (size);

    buffer->size = size;
    buffer->occupied = 0;

    printf("Buffer->head: %p\n", buffer->head);
    printf("Buffer->tail: %p\n", buffer->tail);
    printf("Buffer->end: %p\n", buffer->end);
    printf("Buffer->size: %d\n", buffer->size);
    printf("Buffer->occupied: %d\n", buffer->occupied);

    return;
}

void buffer_reset(circular_buffer_t * buffer)
{
    if (!buffer || !buffer->ptr) {
        fprintf(stderr, "Error! Bad buffer pointer\n");
        return;
    }

    free(buffer->ptr);

    buffer->ptr = NULL;
    buffer->head = NULL;
    buffer->tail = NULL;
    buffer->end = NULL;

    buffer->size = 0;
    buffer->occupied = 0;

    return;
}

//Write data to the buffer
uint8_t buffer_write(circular_buffer_t * buffer,
                     const uint8_t * data, const unsigned int length)
{
    if (!buffer || !data) {
        fprintf(stderr, "Error! Bad parameter(s)\n");
        return;
    }

    int i = 0;

    //Copy data into the buffer, handling the overlap event of the buffer
    for (i = 0; i < length; i++) {

        if (buffer->occupied == buffer->size)   //i.e no space.
        {
            fprintf(stderr, "Error! Buffer full\n");
            return i;
        }

        if ((buffer->head) >= (buffer->end)) {
            buffer->head = buffer->ptr; //Reset the pointer;
        }

        memcpy(buffer->head, &data[i], 1);
        buffer->head++;
        buffer->occupied++;
    }

    return;
}

//Read data from the buffer
uint8_t buffer_read(circular_buffer_t * buffer,
                    uint8_t * const data, const uint8_t length)
{
    if (!buffer || !data) {
        fprintf(stderr, "Error! Bad parameter(s)\n");
        return 0;
    }

    int i = 0, cpy_length = 0;

    cpy_length = length;

    if (length > buffer->occupied) {
        //We can only copy upto the max bytes of data available in the buffer
        cpy_length = buffer->occupied;
    }

    uint8_t *ptr = buffer->tail;

    //Copy data into the suppied buffer, handling the overlap event of the buffer
    for (i = 0; i < cpy_length; i++) {
        if ((ptr) >= (buffer->end)) {
            ptr = buffer->ptr;  //Reset the pointer;
        }

        data[i] = *ptr;
        ptr++;
        //buffer->occupied--;
    }

    return i;
}



//Consume data from the circular buffer
uint8_t buffer_consume(circular_buffer_t * buffer, const uint8_t length)
{
    if (!buffer) {
        fprintf(stderr, "Error! Bad parameter(s)\n");
        return 0;
    }

    int i = 0, cpy_length = 0;

    cpy_length = length;

    if (length > buffer->occupied) {
        //We can only copy upto the max bytes of data available in the buffer
        cpy_length = buffer->occupied;
    }
    //Copy data into the suppied buffer, handling the overlap event of the buffer
    for (i = 0; i < cpy_length; i++) {
        if ((buffer->tail) >= (buffer->end)) {
            buffer->tail = buffer->ptr; //Reset the pointer;
        }

        buffer->tail++;
        buffer->occupied--;
    }
    return i;
}


//How much space is in the buffer
uint8_t buffer_space(circular_buffer_t * buffer)
{
    if (!buffer) {
        fprintf(stderr, "Error! Bad parameter(s)\n");
        return 0;
    }

    return (buffer->size - buffer->occupied);
}

//Print the buffer
void buffer_print(circular_buffer_t * buffer)
{
    if (!buffer) {
        fprintf(stderr, "Error! Bad parameter(s)\n");
        return;
    }

    int i = 0;

    printf("\nCircular buffer data [START]\n");

    for (i = 0; i < buffer->size; i++) {
        //if(i!=0 && i % 49 == 0) printf("\n");

        //printf("%c",*(buffer->ptr+i));

        printf("%p, %c\n", buffer->ptr + i, *(buffer->ptr + i));

    }

    printf("\nCircular buffer data [END]\n");

    printf("Buffer->head: %p\n", buffer->head);
    printf("Buffer->tail: %p\n", buffer->tail);
    printf("Buffer->end: %p\n", buffer->end);
    printf("Buffer->size: %d\n", buffer->size);
    printf("Buffer->occupied: %d\n", buffer->occupied);

    return;
}


int main(int argc, char *argv[])
{
    struct pollfd fds[1];

    if (!argv[1]) {
        printf("Usage: ./client <port_number>\n");
        return -1;
    }

    int sockfd;
    int len, port;
    struct sockaddr_in address;
    int result = 0;

    char buffer[512] = { 0 };


    port = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //if ( setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &yes, sizeof(int)) == -1 )
    //{
    //perror("setsockopt");
    //}


    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);
    len = sizeof(address);

    int i = 0, j = 0, ret = 0;
    int flags = 0;

    //fcntl(sockfd, F_SETFL, O_NONBLOCK);


    uint8_t buff[255] = { 0 };

    uint8_t parse_buffer[255] = { 0 };

    circular_buffer_t c_buffer;
    buffer_init(&c_buffer, 255);

    //const char * data = "Hello there.";
    //const char * data1 = "Goodbye.";


    if (connect(sockfd, (struct sockaddr *) &address, len) == -1) {
        perror("oops: client failed to connect");
    }

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;


    while (1) {
        sleep(2);
        int num_bytes = buffer_space(&c_buffer);
        //printf("Space available within the buffer: %d\n",num_bytes);
        result = read(sockfd, buffer, num_bytes);
        //printf("Result: %d\n",result);
        buffer_write(&c_buffer, buffer, result);
        //buffer_print(&c_buffer);

        //Clear the temporary buffer
        memset(buffer, '\0', 255);
        memset(parse_buffer, '\0', 255);

        result = buffer_read(&c_buffer, parse_buffer, 255);     //Try to create a linear buffer for parsing.

        //printf("\nParse buffer [START]\n");

        //for(j=0; j < 255; j++)
        //    printf("%c",parse_buffer[j]);

        //printf("\nParse buffer [END]\n");

        //Parse it!
        uint8_t *pch = NULL;

        int index = 0, start = 0;
        int bytes_occupied = 0;

        for (index = 0; index < 255; index++) {
            if (parse_buffer[index] == '\n') {
                bytes_occupied = index - start;
                bytes_occupied++;
                //printf("bytes_occupied: %d\n",bytes_occupied);

                //Process message, simply print it in this case 
                printf("\nMessage received: ");

                for (j = start; j < index; j++)
                    printf("%c", parse_buffer[j]);

                printf("\n");

                //Message finished with, move on
                start = index + 1;
                //Remove this number of bytes (the message) from the buffer
                ret = buffer_consume(&c_buffer, bytes_occupied);
                //printf("Buffer_consume (%d): %d\n", bytes_occupied,ret); 
                //buffer_print(&c_buffer);
            }


        }
        //buffer_print(&c_buffer);
        //return 0; //Temp


    }


    buffer_reset(&c_buffer);

    close(sockfd);

    return 0;
}
