#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <time.h>

static const char *socket_path = "/tmp/creationOfSocketTest1";
static const unsigned int s_recv_len = 2000;
static const unsigned int s_send_len = 2000;
static const unsigned int packet_size_byte = 1024;
static const unsigned int small_packet_size_byte = 256;
static const unsigned int big_packet_size_byte = 65536;
static const int num_packets = 10000;
long long current_timestamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec; // Return time in nanoseconds
}
int main()
{
    int sock = 0;
    int data_len = 0;
    struct sockaddr_un remote;
    char recv_msg[s_recv_len];
    char send_msg[s_send_len];

    memset(recv_msg, 0, s_recv_len * sizeof(char));
    memset(send_msg, 0, s_send_len * sizeof(char));

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        printf("Client: Error on socket() call \n");
        return 1;
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, socket_path);
    data_len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    long long connectingTimeStarted = current_timestamp();
    
    printf("Client: Trying to connect... \n");
    if (connect(sock, (struct sockaddr *)&remote, data_len) == -1)
    {
        printf("Client: Error on connect call \n");
        return 1;
    }

    printf("Client: Connected \n");
    long long connectingTimeEnded = current_timestamp();
    double elapsed_seconds = (connectingTimeEnded - connectingTimeStarted) / 1.0e9;
    printf("Established connecting in %f seconds.\n",  elapsed_seconds);

    int num_packets;
    char packet[packet_size_byte];
    while (printf("> Enter the number of packets to send: "), scanf("%d", &num_packets) == 1)
    {
        long long start_time = current_timestamp();
        // packet sending
        //scanf("%d", &num_packets);
        getchar(); // Consume the newline
        if(num_packets <= 0)
        {
            if (send(sock, "0", 1 * sizeof(char), 0) == -1)
            {
                printf("Client: Error on send() call \n");
            }
            printf("Client: Server socket closed \n");
            printf("Zero packets are needed \n");
            close(sock);
            break;
        }
        memset(packet, 'A', packet_size_byte); // Fill the packet with the character 'A'
        for (int i = 0; i < num_packets; i++)
        {
            if (send(sock, packet, packet_size_byte, 0) == -1)
            {
                printf("Client: Error on send() call \n");
                close(sock);
                return 1;
            }
            // memset(send_msg, 0, s_send_len * sizeof(char));
            // memset(recv_msg, 0, s_recv_len * sizeof(char));
            // if ((data_len = recv(sock, recv_msg, s_recv_len, 0)) > 0)
            // {
            //     printf("Client: Data received: %s \n", recv_msg);
            // }
            // else
            // {
            //     if (data_len < 0)
            //     {
            //         printf("Client: Error on recv() call \n");
            //     }
            //     else
            //     {
            //         printf("Client: Server socket closed \n");
            //         close(sock);
            //         break;
            //     }
            // }
        }
        printf("Sent %d packets to server.\n", num_packets);
        long long end_time = current_timestamp();
        double elapsed_seconds = (end_time - start_time) / 1.0e9;
        printf("Received %d packets in %f seconds.\n", num_packets, elapsed_seconds);
    }
    
    printf("Client: bye! \n");

    return 0;
}

// if (send(sock, send_msg, strlen(send_msg) * sizeof(char), 0) == -1)
// {
//     printf("Client: Error on send() call \n");
// }
// memset(send_msg, 0, s_send_len * sizeof(char));
// memset(recv_msg, 0, s_recv_len * sizeof(char));

// if ((data_len = recv(sock, recv_msg, s_recv_len, 0)) > 0)
// {
//     printf("Client: Data received: %s \n", recv_msg);
// }
// else
// {
//     if (data_len < 0)
//     {
//         printf("Client: Error on recv() call \n");
//     }
//     else
//     {
//         printf("Client: Server socket closed \n");
//         close(sock);
//         break;
//     }
// }