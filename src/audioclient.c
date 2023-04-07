// -- exit fn / atoi fn --
#include <stdlib.h>
// -- perror --
#include <stdio.h>
// -- strtod (convert string to float) / toupper --
#include <ctype.h>
// -- strlen/strcpy --
#include <string.h>
// -- UDP --
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// -- close sockets --
#include <unistd.h>

// --aud_writeinit--
#include "../include/audio.h"

#define MAX_LENGTH 50
#define TRUE 1
#define FALSE 0

#define IP_SERVER "127.0.0.1" // "148.60.173.191" // "148.60.3.86"
#define Client_Port 1234
#define Server_Port 2000

void upper_string(char s[])
{
    int c = 0;

    while (s[c] != '\0')
    {
        s[c] = toupper(s[c]);
        c++;
    }
}

int main(int argc, char *argv[])
{

    int socket_descriptor, bind_err, send_err;

    char client_ready[20] = "Client ready";

    struct sockaddr_in dest;
    struct sockaddr_in addr;

    // Receive's Stuff
    socklen_t len, flen;
    struct sockaddr_in from;
    flen = sizeof(struct sockaddr_in);

    // -- Socket Creation --

    socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_descriptor < 0)
    {
        perror("Error client: socket creation !");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(Client_Port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // IP_CLIENT

    dest.sin_family = AF_INET;
    dest.sin_port = htons(Server_Port);
    dest.sin_addr.s_addr = inet_addr(IP_SERVER); // htonl(INADDR_ANY); // inet_addr(IP_SERVER);

    // -- Socket Binding --

    bind_err = bind(socket_descriptor, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    if (bind_err < 0)
    {
        perror("Error client: socket binding !");
        exit(1);
    }

    printf("--Client-- Done with binding\n");

    while (TRUE)
    {

        // -- Music's Name Emission --

        printf("--Client-- Please type here, the name of the music.\n");

        char music_file[MAX_LENGTH];
        fgets(music_file, MAX_LENGTH, stdin);
        // Remove the return character "\n" from the user input
        music_file[strlen(music_file) - 1] = '\0';

        printf("--Client-- String read: *%s*\n", music_file);

        send_err = sendto(
            socket_descriptor,
            music_file,
            sizeof(music_file),
            0,
            (struct sockaddr *)&dest,
            sizeof(struct sockaddr_in));

        if (send_err < 0)
        {
            perror("Error client: Datagram sending !");
            exit(1);
        }

        printf("--Client-- %dbytes sent\n", send_err);

        int file_existence;

        printf("--Client-- Send State: Ready\n");
        send_err = sendto(
            socket_descriptor,
            client_ready,
            sizeof(client_ready),
            0,
            (struct sockaddr *)&dest,
            sizeof(struct sockaddr_in));

        if (send_err < 0)
        {
            perror("Error client: Ready Datagram sending !");
            exit(1);
        }

        len = recvfrom(
            socket_descriptor,
            &file_existence,
            sizeof(file_existence),
            0,
            (struct sockaddr *)&from,
            &flen);

        if (len < 0)
        {
            perror("Error client: file_existence Reception !");
            exit(1);
        }

        if (!file_existence)
        {
            printf("--Client-- The file does not exist. Try Again\n");
            continue;
        }

        // ----- Filters -----

        char answer_effects[MAX_LENGTH];
        int effects = FALSE;

        int correct_answer = FALSE;
        while (!correct_answer)
        {
            printf("--Client-- Do you want to apply any effects ? (Y/N)\n");

            if (fgets(answer_effects, MAX_LENGTH, stdin) == NULL)
            {
                /* Unexpected error */
                perror("Error client: effects fgets !");
                exit(1);
            }
            // Remove the return character "\n" from the user input
            answer_effects[strlen(answer_effects) - 1] = '\0';

            // Convert to upper case
            int j = 0;
            // while answer_effects[j]
            while (answer_effects[j] != '\0')
            {
                answer_effects[j] = toupper(answer_effects[j]);
                j++;
            }

            printf("--Client-- *%s*\n", answer_effects);

            // or just compare the first letter
            if (strcmp(answer_effects, "YES") == 0 || strcmp(answer_effects, "Y") == 0 || strcmp(answer_effects, "OUI") == 0 || strcmp(answer_effects, "O") == 0)
            {
                printf("--Client-- YES\n");
                correct_answer = TRUE;
                effects = TRUE;
            }
            else if (strcmp(answer_effects, "NO") == 0 || strcmp(answer_effects, "NON") == 0 || strcmp(answer_effects, "N") == 0)
            {
                printf("--Client-- NO\n");
                correct_answer = TRUE;
                effects = FALSE;
            }
            else
            {
                printf("--Client-- Wrong answer. Please type (Y/N/Yes/No/Oui/Non/O)\n");
            }
        }

        double speed = 1;
        if (effects)
        {
            char *endptr;

            int correct_speed = FALSE;
            char speed_wanted[MAX_LENGTH];
            while (!correct_speed)
            {
                printf("--Client-- Please type here, the wanted speed.\n");
                printf("--Client-- examples, 1: for normal, 2: for twice faster, 0.5: for twice slower\n");

                if (fgets(speed_wanted, sizeof(speed_wanted), stdin) == NULL)
                {
                    /* Unexpected error */
                    perror("Error client: speed fgets !");
                    exit(1);
                }

                // parse the string into float
                speed = strtod(speed_wanted, &endptr);
                if (speed == 0)
                {
                    printf("--Client-- You can't play a music at 0 or less speed.\n");
                }
                else if ((*endptr == '\0') || (isspace(*endptr) != 0))
                    correct_speed = TRUE;
            }
        }

        // -- data reply from the server --

        printf("--Client-- Listening for incoming messages...\n\n");

        // default value
        int server_message; // = -1;

        // -- DATA --

        int sample_rate;
        int sample_size;
        int channels;

        // -- END DATA --

        for (int count = 0; count < 3; count++)
        {
            printf("--Client-- Send State: Ready\n");
            send_err = sendto(
                socket_descriptor,
                client_ready,
                sizeof(client_ready),
                0,
                (struct sockaddr *)&dest,
                sizeof(struct sockaddr_in));

            if (send_err < 0)
            {
                perror("Error client: Ready Datagram sending !");
                exit(1);
            }

            printf("--Client-- Wait Audio Data\n");

            len = recvfrom(
                socket_descriptor,
                &server_message,
                sizeof(server_message),
                0,
                (struct sockaddr *)&from,
                &flen);

            if (len < 0)
            {
                perror("Error client: Msg Reception !");
                exit(1);
            }

            if (count == 0)
            {
                sample_rate = server_message;
                printf("--Client-- sample_rate:\n");
            }
            else if (count == 1)
            {
                sample_size = server_message;
                printf("--Client-- sample_size:\n");
            }
            else if (count == 2)
            {
                channels = server_message;
                printf("--Client-- channels:\n");
            }

            printf("Received %d bytes from host %s port, %d: %i\n", len,
                   inet_ntoa(from.sin_addr), ntohs(from.sin_port), server_message);
        }

        // -- Creation of the audio descriptior --

        int audio_descriptor = aud_writeinit(sample_rate * speed, sample_size, channels);
        if (audio_descriptor < 0)
        {
            perror("Error aud_writeinit !");
            exit(1);
        }

        // -- Write the audio from the data, and the chunk of music the server sends us --

        unsigned char buffer[1024];
        ssize_t byte_left;

        do
        {
            // wait for buffer and the music_bytes
            for (int count = 0; count < 2; count++)
            {
                printf("--Client-- Send State: Ready\n");
                send_err = sendto(
                    socket_descriptor,
                    client_ready,
                    sizeof(client_ready),
                    0,
                    (struct sockaddr *)&dest,
                    sizeof(struct sockaddr_in));

                if (send_err < 0)
                {
                    perror("Error client: Ready Datagram sending !");
                    exit(1);
                }

                printf("--Client-- Wait Audio Data\n");

                if (count == 0)
                {
                    len = recvfrom(
                        socket_descriptor,
                        &byte_left,
                        sizeof(byte_left),
                        0,
                        (struct sockaddr *)&from,
                        &flen);

                    if (len < 0)
                    {
                        perror("Error client: Music Byte Reception !");
                        exit(1);
                    }

                    printf("--Client-- music's bytes: %zd\n", byte_left);
                }
                else if (count == 1)
                {

                    len = recvfrom(
                        socket_descriptor,
                        buffer,
                        sizeof(buffer),
                        0,
                        (struct sockaddr *)&from,
                        &flen);

                    if (len < 0)
                    {
                        perror("Error client: Buffer Reception !");
                        exit(1);
                    }

                    printf("--Client-- buffer:\n");
                }

                printf("Received %d bytes from host %s port, %d\n", len,
                       inet_ntoa(from.sin_addr), ntohs(from.sin_port));
            }
            if (byte_left <= 0)
            {
                printf("--Client-- The music ends\n");
                // the break is mandatory to avoid copypaste code
                // and to ensure that we don't ask the computer to read 0 bytes
                break;
            }
            write(audio_descriptor, buffer, byte_left);
        } while (byte_left > 0);

        if (byte_left == -1)
        {
            perror("Error read !");
            exit(1);
        }
    }

    // TODO: Close it after a user timeout
    // -- Close the Socket --

    int close_err = close(socket_descriptor);

    if (close_err < 0)
    {
        perror("Error client: Socket closing !");
        exit(1);
    }
}