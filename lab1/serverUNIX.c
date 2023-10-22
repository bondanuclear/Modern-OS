#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <time.h>
// #define PACKET_SIZE 1024
// #define NUM_PACKETS 10000
#define RECEIVE_BUFFER_SIZE 1024
#define RECEIVE_BIG_BUFFER_SIZE 65536

#define SEND_BUFFER_SIZE 100
static const char *socket_path = "/tmp/creationOfSocketTest1";
static const unsigned int nIncomingConnections = 5; // The number of pending connections the server can have in its queue.

int main()
{
    // create server side
    int s = 0; // File descriptor for the server socket.
    int s2 = 0; //  File descriptor for the accepted client connection.
    struct sockaddr_un local, remote; // A UNIX domain socket address structure for the server and client.
    int len = 0;  // To store the length of the address structure.

    /// create server socket
    s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == s)
    {
        printf("Error on socket() call \n");
        return 1;
    }

    //// Setting up the local socket address.
    local.sun_family = AF_UNIX;
    // 
    strcpy(local.sun_path, socket_path);
    // Ensures that any previously existing socket at that path is removed.
    unlink(local.sun_path);
    // Computes the length of the address structure.
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    // Binding the server socket s to the local address. If there's an error, the program exits.
    if (bind(s, (struct sockaddr *)&local, len) != 0)
    {
        printf("Error on binding socket \n");
        return 1;
    }

    if (listen(s, nIncomingConnections) != 0)
    {
        printf("Error on listen call \n");
    }

    bool bWaiting = true;

    while (bWaiting)
    {
        unsigned int sock_len = 0;
        printf("Waiting for connection.... \n");
        if ((s2 = accept(s, (struct sockaddr *)&remote, &sock_len)) == -1)
        {
            printf("Error on accept() call \n");
            return 1;
        }

        printf("Server connected \n");

        int data_recv = 0;
        char recv_buf[RECEIVE_BUFFER_SIZE];
        char send_buf[SEND_BUFFER_SIZE];
        do
        {
            // memset - pointer to destination, value, number of bytes I want to set
            memset(recv_buf, 0, RECEIVE_BUFFER_SIZE * sizeof(char));
            memset(send_buf, 0, SEND_BUFFER_SIZE * sizeof(char));
            data_recv = recv(s2, recv_buf, RECEIVE_BUFFER_SIZE, 0);
            if (data_recv > 0)
            {
                // printf("Data received: %d \n", data_recv);
                // strcpy(send_buf, "Got message: ");
                // char tempStr[10];
                // sprintf(tempStr, "%d", data_recv);

                // strcat(send_buf, tempStr);
                
                if (strstr(recv_buf, "0") != 0)
                {
                    printf("Exit command received -> quitting \n");
                    bWaiting = false;
                    break;
                }

                // if (send(s2, send_buf, strlen(send_buf) * sizeof(char), 0) == -1)
                // {
                //     printf("Error on send() call \n");
                // }
            }
            else
            {
                printf("Error on recv() call \n");
            }
        } while (data_recv > 0);

        close(s2);
    }
    close(s);
    return 0;
}