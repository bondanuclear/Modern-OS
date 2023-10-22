#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

static const unsigned int s_recv_len = 2000;
static const unsigned int s_send_len = 2000;
static const unsigned int packet_size_byte = 80; // 80

ssize_t send_packet(int sock, const char *packet, size_t packet_size_byte)
{
    ssize_t bytes_sent = 0;
    ssize_t total_bytes_sent = 0;

    while (total_bytes_sent < packet_size_byte)
    {
        bytes_sent = send(sock, packet + total_bytes_sent, packet_size_byte - total_bytes_sent, 0);

        if (bytes_sent == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Buffer is full, wait a bit and try again
                sleep(0.001); // wait for 1 millisecond
                continue;
            }
            else
            {
                perror("Client: Error on send() call");
                return -1;
            }
        }

        total_bytes_sent += bytes_sent;
    }

    return total_bytes_sent;
}
long long current_timestamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec; // Return time in nanoseconds
}

int set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main()
{
    int sock = 0;
    struct sockaddr_in remote;
    char recv_msg[s_recv_len];
    char send_msg[s_send_len];

    memset(recv_msg, 0, s_recv_len * sizeof(char));
    memset(send_msg, 0, s_send_len * sizeof(char));

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Client: Error on socket() call");
        return 1;
    }

    if (set_nonblocking(sock) == -1)
    {
        perror("Client: Error setting socket non-blocking");
        close(sock);
        return 1;
    }

    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr(SERVER_IP);
    remote.sin_port = htons(SERVER_PORT);

    long long connectingTimeStarted = current_timestamp();
    printf("Client: Trying to connect... \n");

    int result = connect(sock, (struct sockaddr *)&remote, sizeof(remote));
    if (result == -1 && errno != EINPROGRESS)
    {
        perror("Client: Error on connect call");
        close(sock);
        return 1;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);
    struct timeval timeout = {5, 0}; // 5 seconds timeout
    result = select(sock + 1, NULL, &writefds, NULL, &timeout);

    if (result == 0)
    {
        printf("Client: Connection timed out\n");
        close(sock);
        return 1;
    }

    if (result == -1)
    {
        perror("Client: Error on select");
        close(sock);
        return 1;
    }

    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
    if (so_error != 0)
    {
        perror("Client: Error on non-blocking connect");
        close(sock);
        return 1;
    }

    printf("Client: Connected \n");
    long long connectingTimeEnded = current_timestamp();
    double elapsed_seconds = (connectingTimeEnded - connectingTimeStarted) / 1.0e9;
    printf("Established connection in %f seconds.\n", elapsed_seconds);

    
    int num_packets;
    char packet[packet_size_byte];
    while (printf("> Enter the number of packets to send: "), scanf("%d", &num_packets) == 1)
    {
        getchar(); // Consume the newline

        if (num_packets <= 0)
        {
            if (send(sock, "0", 1, 0) == -1)
            {
                perror("Client: Error on send() call with 0");
            }
            printf("Client: Server socket closed \n");
            close(sock);
            break;
        }

        memset(packet, 'A', packet_size_byte); // Fill the packet with the character 'A'

        long long start_time = current_timestamp();
        for (int i = 0; i < num_packets; i++)
        {
            if (send_packet(sock, packet, packet_size_byte) == -1)
            {
                perror("Client: Error on send() call");
                close(sock);
                return 1;
            }
        }

        long long end_time = current_timestamp();
        elapsed_seconds = (end_time - start_time) / 1.0e9;
        printf("Sent %d packets to server in %f seconds.\n", num_packets, elapsed_seconds);
    }
    printf("Client: bye! \n");
    close(sock);
    return 0;
}
