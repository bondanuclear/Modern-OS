#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h> // for select()
#include <errno.h>

#define SOCKET_PATH "/tmp/creationOfSocketTest1"
#define RECEIVE_BUFFER_SIZE 65536
#define SEND_BUFFER_SIZE 100
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
    struct sockaddr_un local, remote;

    s = socket(AF_UNIX, SOCK_STREAM, 0);
    if (-1 == s)
    {
        perror("Error on socket() call");
        return 1;
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCKET_PATH);
    unlink(SOCKET_PATH); // Remove any existing socket file

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
                    socklen_t size = sizeof(remote);
                    int new = accept(s, (struct sockaddr *)&remote, &size);
                    if (new < 0)
                    {
                        perror("Error on accept()");
                        close(s);
                        return 1;
                    }
                    fprintf(stderr, "Server: Connected from %s\n", remote.sun_path);
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
