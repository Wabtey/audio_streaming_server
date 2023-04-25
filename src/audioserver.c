// -- exit fn --
#include <stdlib.h>
// -- perror --
#include <stdio.h>
// -- Display time in Logs --
#include <time.h>
// ---- UDP ----
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// -- close sockets / sleep / access --
#include <unistd.h>

// -- strlen --
#include <string.h>

// --audreadinit--
#include "../include/audio.h"

#define MAX_LENGTH 200
#define TRUE 1
#define FALSE 0

// #define IP_CLIENT "127.0.0.1" // "86.253.89.102" // "148.60.173.191" // "148.60.3.86"
// #define Client_Port 1234
#define Server_Port 2000

int main(int argc, char *argv[])
{
    // ---- Log Init ----
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    // -- End Log Init --

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

    t = time(NULL);
    tm = *localtime(&t);
    printf("--Server-- %02d:%02d:%02d - Done with binding\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

    while (TRUE)
    {

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Server-- %02d:%02d:%02d - Listening for incoming messages...\n\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

        // -- Music's Name Reception --

        // Data Request from client
        char music_file[MAX_LENGTH];
        socklen_t reception_len, flen;
        struct sockaddr_in from;

        flen = sizeof(struct sockaddr_in);
        t = time(NULL);
        tm = *localtime(&t);
        printf("--Server-- %02d:%02d:%02d - wait...\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

        // No timeout here

        reception_len = recvfrom(
            socket_descriptor,
            music_file,
            sizeof(music_file),
            0,
            (struct sockaddr *)&from,
            &flen);

        if (reception_len < 0)
        {
            perror("Error server: Msg Reception !");
            exit(1);
        }

        printf("Received %d bytes from host %s port, %d: %s\n", reception_len,
               inet_ntoa(from.sin_addr), ntohs(from.sin_port), music_file);

        // send the ack msg?

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Server-- %02d:%02d:%02d - Chaine lue: *%s*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, music_file);

        // IDEA: Add the precise location of the audio file
        char assets_path[MAX_LENGTH] = "assets/audio/%s.wav";
        char music_path[MAX_LENGTH];
        sprintf(music_path, assets_path, music_file);
        // sscanf(music_file, , music_file);
        t = time(NULL);
        tm = *localtime(&t); //
        printf("--Server-- %02d:%02d:%02d - Chaine modifiée: *%s*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, music_path);

        // -- Get the client infos from the `from` sockaddr_in --

        dest.sin_family = from.sin_family;
        dest.sin_port = from.sin_port;
        dest.sin_addr.s_addr = from.sin_addr.s_addr;

        // dest.sin_family = AF_INET;
        // dest.sin_port = htons(Client_Port);
        // dest.sin_addr.s_addr = inet_addr(IP_CLIENT);

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Server-- %02d:%02d:%02d - Testing File Existence\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
        // IDEA: test first if there is a .wav file (for more security)

        int file_existence;

        if (access(music_path, F_OK) != 0)
        {
            t = time(NULL);
            tm = *localtime(&t);
            printf("--Server-- %02d:%02d:%02d - File doesn't exists\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

            file_existence = FALSE;
        }
        else
        {
            t = time(NULL);
            tm = *localtime(&t);
            printf("--Server-- %02d:%02d:%02d - File does exists\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

            file_existence = TRUE;
        }
        // wait the 'ack'/'start signal' of the client

        reception_len = recvfrom(
            socket_descriptor,
            client_ready,
            sizeof(client_ready),
            0,
            (struct sockaddr *)&from,
            &flen);

        // TODO: ? - Add a strcmp with "Client ready"

        if (reception_len < 0)
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
            // DEBUG: break;
            continue;
        }

        // -- Creation of the file descriptior --

        int sample_rate;
        int sample_size;
        int channels;
        int fd = aud_readinit(music_path, &sample_rate, &sample_size, &channels);

        if (fd < 0)
        {
            perror("Error aud_readinit !");
            exit(1);
        }

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Server-- %02d:%02d:%02d - fd : *%d*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, fd);

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

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Server-- %02d:%02d:%02d - Wait the client\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
            reception_len = recvfrom(
                socket_descriptor,
                client_ready,
                sizeof(client_ready),
                0,
                (struct sockaddr *)&from,
                &flen);

            if (reception_len < 0)
            {
                perror("Error server: Msg Reception !");
                exit(1);
            }

            printf("Received %d bytes from host %s port, %d: %s\n", reception_len,
                   inet_ntoa(from.sin_addr), ntohs(from.sin_port), client_ready);

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Server-- %02d:%02d:%02d - *%s*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, client_ready);

            // send the sample_rate/size/channels

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Server-- %02d:%02d:%02d - Send Data\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

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

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Server-- %02d:%02d:%02d - *%i* sent\n", tm.tm_hour, tm.tm_min, tm.tm_sec, send_msg);
            t = time(NULL);
            tm = *localtime(&t);
            printf("--Server-- %02d:%02d:%02d - %dbytes sent\n", tm.tm_hour, tm.tm_min, tm.tm_sec, send_err);
        }

        // ---- Read of the music ----
        t = time(NULL);
        tm = *localtime(&t);
        printf("--Server-- %02d:%02d:%02d - Send Music\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

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

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Server-- %02d:%02d:%02d - Wait the client\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                reception_len = recvfrom(
                    socket_descriptor,
                    client_ready,
                    sizeof(client_ready),
                    0,
                    (struct sockaddr *)&from,
                    &flen);

                if (reception_len < 0)
                {
                    perror("Error server: Msg Reception !");
                    exit(1);
                }

                printf("Received %d bytes from host %s port, %d: %s\n", reception_len,
                       inet_ntoa(from.sin_addr), ntohs(from.sin_port), client_ready);

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Server-- %02d:%02d:%02d - *%s*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, client_ready);

                // wait 1 second for the client opening
                // sleep(1);

                if (count == 0)
                {
                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Server-- %02d:%02d:%02d - Send the number of byte_left: *%zd*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, byte_left);
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
                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Server-- %02d:%02d:%02d - Send buffer\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
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

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Server-- %02d:%02d:%02d - %dbytes sent\n", tm.tm_hour, tm.tm_min, tm.tm_sec, send_err);
            }
        } while (byte_left > 0);

        if (byte_left == -1)
        {
            perror("Error read !");
            exit(1);
        }

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Server-- %02d:%02d:%02d - The music ends\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
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