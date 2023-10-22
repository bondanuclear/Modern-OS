#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define RECEIVE_BUFFER_SIZE 1472 // 80
#define SEND_BUFFER_SIZE 100
#define RECEIVE_BIG_BUFFER_SIZE 1472

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main()
{
    int s = 0;                        // File descriptor for the server socket.
    struct sockaddr_in local, remote; // A sockaddr_in structure for the server and client (IPv4).

    // create server socket
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == s)
    {
        perror("Error on socket() call");
        return 1;
    }

    // Setting up the local socket address.
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = inet_addr(SERVER_IP);
    local.sin_port = htons(SERVER_PORT);

    if (bind(s, (struct sockaddr *)&local, sizeof(local)) != 0)
    {
        perror("Error on binding socket");
        close(s);
        return 1;
    }

    bool bWaiting = true;

    while (bWaiting)
    {
        char recv_buf[RECEIVE_BUFFER_SIZE];
        socklen_t remote_len = sizeof(remote);

        int data_recv = recvfrom(s, recv_buf, RECEIVE_BUFFER_SIZE, 0, (struct sockaddr *)&remote, &remote_len);
        if (data_recv > 0)
        {
            recv_buf[data_recv] = '\0'; // Null terminate the received data for safety
            if (strstr(recv_buf, "0") != 0)
            {
                printf("Exit command received -> quitting \n");
                bWaiting = false;
                break;
            }
            // If you want to send a reply, use sendto()
            // e.g., sendto(s, send_buf, strlen(send_buf), 0, (struct sockaddr*)&remote, remote_len);
        }
        else
        {
            perror("Error on recvfrom() call");
        }
    }

    close(s);
    return 0;
}
