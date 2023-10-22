#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_IP "127.0.0.1" 
#define SERVER_PORT 8080      

static const unsigned int s_send_len = 2000;
static const unsigned int packet_size_byte = 1472;
static const unsigned int big_packet_size_byte = 1472;

long long current_timestamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec; // Return time in nanoseconds
}

int main()
{
    int sock;
    struct sockaddr_in remote;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Client: Error on socket() call");
        return 1;
    }

    remote.sin_family = AF_INET;
    remote.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &remote.sin_addr);

    char packet[packet_size_byte];

    int num_packets_to_send;
    while (printf("> Enter the number of packets to send: "), scanf("%d", &num_packets_to_send) == 1)
    {
        getchar(); // Consume the newline

        if (num_packets_to_send <= 0)
        {
            sendto(sock, "0", 1, 0, (struct sockaddr *)&remote, sizeof(remote));
            printf("Client: Server socket closed \n");
            close(sock);
            break;
        }

        memset(packet, 'A', packet_size_byte); // Fill the packet with the character 'A'

        long long start_time = current_timestamp();

        for (int i = 0; i < num_packets_to_send; i++)
        {
            if (sendto(sock, packet, packet_size_byte, 0, (struct sockaddr *)&remote, sizeof(remote)) == -1)
            {
                perror("Client: Error on send() call");
                close(sock);
                return 1;
            }
        }

        long long end_time = current_timestamp();
        double elapsed_seconds = (end_time - start_time) / 1.0e9;

        printf("Sent %d packets to server in %f seconds.\n", num_packets_to_send, elapsed_seconds);
    }

    printf("Client: bye! \n");
    close(sock);
    return 0;
}
