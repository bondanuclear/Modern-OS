#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <time.h>

#define RECEIVE_BUFFER_SIZE 1024
#define RECEIVE_BIG_BUFFER_SIZE 65536

#define SEND_BUFFER_SIZE 100
static const char *socket_path = "/tmp/creationOfSocketTest1";

int main()
{
    int s = 0;                        // File descriptor for the server socket.
    struct sockaddr_un local, remote; // A UNIX domain socket address structure for the server and client.

    // create server socket
    s = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == s)
    {
        perror("Error on socket() call");
        return 1;
    }

    // Setting up the local socket address.
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, socket_path);
    unlink(local.sun_path); // Ensures that any previously existing socket at that path is removed.

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
        // char send_buf[SEND_BUFFER_SIZE]; // Not used for now
        socklen_t remote_len = sizeof(remote);

        int data_recv = recvfrom(s, recv_buf, RECEIVE_BUFFER_SIZE, 0, (struct sockaddr *)&remote, &remote_len);
        if (data_recv > 0)
        {
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
