// -- exit fn --
#include <stdlib.h>
// -- perror --
#include <stdio.h>
// ---- UDP ----
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// -- close sockets / sleep / access --
#include <unistd.h>
// -- form timeout --
#include <sys/time.h>

// -- strlen --
#include <string.h>

// --audreadinit--
#include "../include/audio.h"

#define MAX_LENGTH 100
#define TRUE 1
#define FALSE 0

// #define IP_CLIENT "127.0.0.1" // "86.253.89.102" // "148.60.173.191" // "148.60.3.86"
// #define Client_Port 1234
#define Server_Port 2000

int main(int argc, char *argv[])
{
    // -- Socket Creation --

    int socket_descriptor, bind_err;
    struct sockaddr_in addr;
    socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_descriptor < 0)
    {
        perror("Error server: socket creation !");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(Server_Port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr(IP_SERVER);

    char client_ready[20];
    // sendto client
    int send_err;
    struct sockaddr_in dest;

    // -- Socket Binding --

    bind_err = bind(socket_descriptor, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    if (bind_err < 0)
    {
        perror("Error server: socket binding !");
        exit(1);
    }

    printf("--Server-- Done with binding\n");

    // -- Socket Timeout --

    // struct timeval read_timeout;
    // read_timeout.tv_sec = 0;
    // read_timeout.tv_usec = 1000000;
    // int timeout_err = setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    // if (timeout_err < 0)
    // {
    //     perror("Error server: socket set Timeout !");
    //     exit(1);
    // }

    while (TRUE)
    {

        printf("--Server-- Listening for incoming messages...\n\n");

        // -- Music's Name Reception --

        // Data Request from client
        char music_file[MAX_LENGTH];
        socklen_t len, flen;
        struct sockaddr_in from;

        flen = sizeof(struct sockaddr_in);
        printf("--Server-- wait...\n");
        len = recvfrom(
            socket_descriptor,
            music_file,
            sizeof(music_file),
            0,
            (struct sockaddr *)&from,
            &flen);

        if (len < 0)
        {
            perror("Error server: Msg Reception !");
            exit(1);
        }

        printf("Received %d bytes from host %s port, %d: %s\n", len,
               inet_ntoa(from.sin_addr), ntohs(from.sin_port), music_file);

        // send the ack msg

        printf("--Server-- Chaine lue: *%s*\n", music_file);

        // IDEA: Add the precise location of the audio file
        // sscanf(music_file, "assets/audio/%s.wav", music_file);
        // printf("--Server-- Chaine modifiée: *%s*\n", music_file);

        // -- Get the client infos from the `from` sockaddr_in --

        dest.sin_family = from.sin_family;
        dest.sin_port = from.sin_port;
        dest.sin_addr.s_addr = from.sin_addr.s_addr;

        // dest.sin_family = AF_INET;
        // dest.sin_port = htons(Client_Port);
        // dest.sin_addr.s_addr = inet_addr(IP_CLIENT);

        printf("--Server-- Testing File Existence\n");
        // IDEA: test first if there is a .wav file (for more security)

        int file_existence;

        if (access(music_file, F_OK) != 0)
        {
            printf("--Server-- File doesn't exists\n");

            file_existence = FALSE;
        }
        else
        {
            printf("--Server-- File does exists\n");

            file_existence = TRUE;
        }

        // wait the 'ack'/'start signal' of the client
        len = recvfrom(
            socket_descriptor,
            client_ready,
            sizeof(client_ready),
            0,
            (struct sockaddr *)&from,
            &flen);

        // TODO: Add a strcmp with "Client ready"

        if (len < 0)
        {
            perror("Error server: Msg Reception !");
            exit(1);
        }

        send_err = sendto(
            socket_descriptor,
            &file_existence,
            sizeof(file_existence),
            0,
            (struct sockaddr *)&dest,
            sizeof(struct sockaddr_in));

        if (send_err < 0)
        {
            perror("Error server: Datagram sending ! - `file existence msg`");
            exit(1);
        }

        if (!file_existence)
        {
            // Try Again
            continue;
        }

        // -- Creation of the file descriptior --

        int sample_rate;
        int sample_size;
        int channels;
        int fd = aud_readinit(music_file, &sample_rate, &sample_size, &channels);

        if (fd < 0)
        {
            perror("Error aud_readinit !");
            exit(1);
        }

        printf("--Server-- fd : *%d*\n", fd);

        // -- Reply --

        // Has to send to the client:
        // - sample_rate
        // - sample_size
        // - channels
        // - fd (not needed)

        // to prevent if uninitialized case (should not happens)
        int send_msg = sample_rate; // = -1;

        for (int count = 0; count < 3; count++)
        {
            if (count == 0)
            {
                send_msg = sample_rate;
            }
            else if (count == 1)
            {
                send_msg = sample_size;
            }
            else if (count == 2)
            {
                send_msg = channels;
            }

            // wait the 'ack'/'start signal' of the client

            printf("--Server-- Wait the client\n");

            len = recvfrom(
                socket_descriptor,
                client_ready,
                sizeof(client_ready),
                0,
                (struct sockaddr *)&from,
                &flen);

            // TODO: Add a strcmp with "Client ready"

            if (len < 0)
            {
                perror("Error server: Msg Reception !");
                exit(1);
            }

            printf("Received %d bytes from host %s port, %d: %s\n", len,
                   inet_ntoa(from.sin_addr), ntohs(from.sin_port), client_ready);

            printf("--Server-- *%s*\n", client_ready);

            // send the sample_rate/size/channels

            printf("--Server-- Send Data\n");

            send_err = sendto(
                socket_descriptor,
                &send_msg,
                sizeof(send_msg),
                0,
                (struct sockaddr *)&dest,
                sizeof(struct sockaddr_in));

            if (send_err < 0)
            {
                perror("Error server: Datagram sending !");
                exit(1);
            }

            printf("--Server-- *%i* sent\n", send_msg);
            printf("--Server-- %dbytes sent\n", send_err);
        }

        // ---- Read of the music ----
        printf("--Server-- Send Music\n");

        unsigned char buffer[1024];
        // start by reading the first 1024 bytes
        // FIXME: ?can break if the song is less than 1024?
        ssize_t byte_left = 1024;

        // -- Send the Music Byte first into the buffer --

        do
        {
            byte_left = read(fd, buffer, byte_left);
            // send the buffer and the byte_left
            for (int count = 0; count < 2; count++)
            {
                // wait the 'ack'/'start signal' of the client

                printf("--Server-- Wait the client\n");

                len = recvfrom(
                    socket_descriptor,
                    client_ready,
                    sizeof(client_ready),
                    0,
                    (struct sockaddr *)&from,
                    &flen);

                // TODO: Add a strcmp with "Client ready"

                if (len < 0)
                {
                    perror("Error server: Msg Reception !");
                    exit(1);
                }

                printf("Received %d bytes from host %s port, %d: %s\n", len,
                       inet_ntoa(from.sin_addr), ntohs(from.sin_port), client_ready);

                printf("--Server-- *%s*\n", client_ready);

                // wait 1 second for the client opening
                // sleep(1);

                if (count == 0)
                {
                    printf("--Server-- Send the number of byte_left: *%zd*\n", byte_left);
                    send_err = sendto(
                        socket_descriptor,
                        &byte_left,
                        sizeof(byte_left),
                        0,
                        (struct sockaddr *)&dest,
                        sizeof(struct sockaddr_in));
                }
                else if (count == 1)
                {
                    printf("--Server-- Send buffer\n");
                    send_err = sendto(
                        socket_descriptor,
                        buffer,
                        sizeof(buffer),
                        0,
                        (struct sockaddr *)&dest,
                        sizeof(struct sockaddr_in));
                }

                if (send_err < 0)
                {
                    perror("Error server: Datagram sending !");
                    exit(1);
                }

                // printf("--Server-- %s sent\n", music_data);
                printf("--Server-- %dbytes sent\n", send_err);
            }
        } while (byte_left > 0);

        if (byte_left == -1)
        {
            perror("Error read !");
            exit(1);
        }

        printf("--Server-- The music ends\n");
    }

    // When ? never suppose to do so.
    // -- Close the Socket --

    int close_err = close(socket_descriptor);

    if (close_err < 0)
    {
        perror("Error server: Socket closing !");
        exit(1);
    }
}