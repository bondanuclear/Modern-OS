#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>

#define RECEIVE_BUFFER_SIZE 65536
#define RECEIVE_BIG_BUFFER_SIZE 65536
#define SEND_BUFFER_SIZE 100

#define SERVER_IP "127.0.0.1" // localhost IP for testing; change if necessary
#define SERVER_PORT 8080      // Arbitrary port; ensure it's available and not used by other applications

static const unsigned int nIncomingConnections = 5; // The number of pending connections the server can have in its queue.

int main()
{
    int s = 0;                        // File descriptor for the server socket.
    int s2 = 0;                       // File descriptor for the accepted client connection.
    struct sockaddr_in local, remote; // A sockaddr_in structure for the server and client (IPv4).

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == s)
    {
        perror("Error on socket() call");
        return 1;
    }

    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(SERVER_IP);
    local.sin_port = htons(SERVER_PORT);

    if (bind(s, (struct sockaddr *)&local, sizeof(local)) != 0)
    {
        perror("Error on binding socket");
        close(s);
        return 1;
    }

    if (listen(s, nIncomingConnections) != 0)
    {
        perror("Error on listen call");
        close(s);
        return 1;
    }

    bool bWaiting = true;
    while (bWaiting)
    {
        socklen_t remote_len = sizeof(remote);
        printf("Waiting for connection.... \n");
        s2 = accept(s, (struct sockaddr *)&remote, &remote_len);
        if (s2 == -1)
        {
            perror("Error on accept() call");
            close(s);
            return 1;
        }
        printf("Server connected \n");

        int data_recv;
        char recv_buf[RECEIVE_BUFFER_SIZE];
        char send_buf[SEND_BUFFER_SIZE];
        do
        {
            memset(recv_buf, 0, sizeof(recv_buf));
            memset(send_buf, 0, sizeof(send_buf));
            data_recv = recv(s2, recv_buf, RECEIVE_BUFFER_SIZE, 0);
            if (data_recv > 0)
            {
                if (strstr(recv_buf, "0") != NULL)
                {
                    printf("Exit command received -> quitting \n");
                    bWaiting = false;
                    break;
                }
            }
            else
            {
                perror("Error on recv() call");
            }
        } while (data_recv > 0);

        close(s2); // Close the client connection
    }
    close(s); // Close the main server socket

    return 0;
}
