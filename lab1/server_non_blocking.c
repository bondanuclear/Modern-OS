#define _POSIX_C_SOURCE 199309L
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
#include <fcntl.h>
#include <sys/time.h> // for select()
#include <errno.h>
#define RECEIVE_BUFFER_SIZE 80 // 80
#define SEND_BUFFER_SIZE 100

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
static const unsigned int nIncomingConnections = 5;

int setNonBlocking(int sock)
{
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1)
    {
        perror("Error getting socket flags");
        return -1;
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("Error setting socket to non-blocking");
        return -1;
    }
    return 0;
}

int main()
{
    int s = 0;
    struct sockaddr_in local, remote;

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

    // Set the socket to non-blocking mode
    if (setNonBlocking(s) == -1)
    {
        close(s);
        return 1;
    }
    
    fd_set active_fd_set, read_fd_set;
    FD_ZERO(&active_fd_set);
    FD_SET(s, &active_fd_set);

    bool bWaiting = true;
    while (bWaiting)
    {
        read_fd_set = active_fd_set;

        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            perror("Error on select()");
            close(s);
            return 1;
        }

        for (int i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET(i, &read_fd_set))
            {
                if (i == s)
                {
                    struct sockaddr_in clientname;
                    socklen_t size = sizeof(clientname);
                    int new = accept(s, (struct sockaddr *)&clientname, &size);
                    if (new < 0)
                    {
                        perror("Error on accept()");
                        close(s);
                        return 1;
                    }
                    fprintf(stderr, "Server: connect from host %s, port %hd.\n",
                            inet_ntoa(clientname.sin_addr),
                            ntohs(clientname.sin_port));
                    FD_SET(new, &active_fd_set);
                }
                else
                {
                    char buffer[RECEIVE_BUFFER_SIZE];
                    int nbytes = read(i, buffer, sizeof(buffer));
                    if (nbytes < 0)
                    {
                        perror("Error reading from socket");
                        close(i);
                        FD_CLR(i, &active_fd_set);
                    }
                    else if (nbytes == 0)
                    {
                        close(i);
                        FD_CLR(i, &active_fd_set);
                    }
                    else
                    {
                        if (strstr(buffer, "0") != NULL)
                        {
                            printf("Exit command received -> quitting \n");
                            bWaiting = false;
                            break;
                        }
                    }
                }
            }
        }
    }
    close(s);
    return 0;
}
