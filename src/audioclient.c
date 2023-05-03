// -- exit fn / atoi fn --
#include <stdlib.h>
// -- perror --
#include <stdio.h>
// -- strtod (convert string to float) / toupper --
#include <ctype.h>
// -- strlen/strcpy --
#include <string.h>
// -- Display Time in Logs --
#include <time.h>
// -- UDP --
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// -- for timeout --
#include <sys/time.h>
// -- close sockets --
#include <unistd.h>

// --aud_writeinit--
#include "../include/audio.h"

#define MAX_LENGTH 150
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

// REFACTOR: create a info!() macro / fn to print logs (instead of )

int main(int argc, char *argv[])
{
    // ---- Log Init ----
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    // -- End Log Init --

    int socket_descriptor, bind_err, send_err;

    char client_ready[20] = "Client ready";

    struct sockaddr_in dest;
    struct sockaddr_in addr;

    // Receive's Stuff
    socklen_t reception_len, flen;
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

    printf("--Client-- %02d:%02d:%02d - Done with binding\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

    // -- Socket Timeout --

    fd_set read_set;
    struct timeval read_timeout;
    int select_num;
    FD_ZERO(&read_set);
    FD_SET(socket_descriptor, &read_set);

    // TODO: remove unused counter
    int timeout_number = 0;

    // int quit = FALSE;
    // !quit
    while (TRUE)
    {
        
        // -- Socket Timeout Reset --

        read_timeout.tv_sec = 0;
        read_timeout.tv_usec = 1000000; // 0.5s == 500000

        // -- Command line - Music's Name Emission --

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Client-- %02d:%02d:%02d - Please type here, the name of the music. (or \"quit\")\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

        char music_file[MAX_LENGTH];
        fgets(music_file, MAX_LENGTH, stdin);
        // Remove the return character "\n" from the user input
        music_file[strlen(music_file) - 1] = '\0';

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Client-- %02d:%02d:%02d - String read: *%s*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, music_file);

        // -- Handle QUIT command --

        char user_cmd[MAX_LENGTH];

        strcpy(user_cmd, music_file);

        // Convert to upper case
        int j = 0;
        while (user_cmd[j] != '\0')
        {
            user_cmd[j] = toupper(user_cmd[j]);
            j++;
        }

        if (
            strcmp(user_cmd, "YES") == 0 ||
            strcmp(user_cmd, "Y") == 0 ||
            strcmp(user_cmd, "OUI") == 0 ||
            strcmp(user_cmd, "O") == 0 ||
            strcmp(user_cmd, "QUIT") == 0 ||
            strcmp(user_cmd, "Q") == 0)
        {
            // quit = TRUE;

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Client-- %02d:%02d:%02d - Quitting...\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
            break;
        }

        //

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

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Client-- %02d:%02d:%02d - %dbytes sent\n", tm.tm_hour, tm.tm_min, tm.tm_sec, send_err);

        int file_existence = -1;

        while (file_existence == -1)
        {

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Client-- %02d:%02d:%02d - Send State: Ready\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
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

            // number of descriptor which are ready for IO, 0 for tiemout, -1 err
            select_num = select(socket_descriptor + 1, &read_set, NULL, NULL, &read_timeout);
            if (select_num < 0)
            {
                perror("Error client: select() !");
                exit(1);
            }
            if (select_num == 0)
            {
                timeout_number++;

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Timeout on file_existence\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                // only available if our music_name or client_ready wasn't received
                // that the server is still in the recvfrom()
                // FIXME: Reset timeout timer after each one
                continue;
            }
            if (FD_ISSET(socket_descriptor, &read_set))
            {
                reception_len = recvfrom(
                    socket_descriptor,
                    &file_existence,
                    sizeof(file_existence),
                    0,
                    (struct sockaddr *)&from,
                    &flen);

                if (reception_len < 0)
                {
                    perror("Error client: file_existence Reception !");
                    exit(1);
                }
            }
        }

        if (!file_existence)
        {

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Client-- %02d:%02d:%02d - The file does not exist. Try Again\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
            continue;
        }

        // ----- Filters -----

        char answer_effects[MAX_LENGTH];
        int effects = FALSE;

        int correct_answer = FALSE;
        while (!correct_answer)
        {

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Client-- %02d:%02d:%02d - Do you want to apply any effects ? (Y/N)\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

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

            t = time(NULL);
            tm = *localtime(&t);
            printf("--Client-- %02d:%02d:%02d - *%s*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, answer_effects);

            // or just compare the first letter
            if (strcmp(answer_effects, "YES") == 0 || strcmp(answer_effects, "Y") == 0 || strcmp(answer_effects, "OUI") == 0 || strcmp(answer_effects, "O") == 0)
            {
                correct_answer = TRUE;
                effects = TRUE;
            }
            else if (strcmp(answer_effects, "NO") == 0 || strcmp(answer_effects, "NON") == 0 || strcmp(answer_effects, "N") == 0)
            {
                correct_answer = TRUE;
                effects = FALSE;
            }
            else
            {

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Wrong answer. Please type (Y/N/Yes/No/Oui/Non/O)\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
            }
        }

        double speed = 1;
        double volume = 1;
        int channels = 0;

        if (effects)
        {
            char *endptr;

            int correct_speed = FALSE;
            char speed_wanted[MAX_LENGTH];
            while (!correct_speed)
            {

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Please type here, the wanted speed.\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - examples, 1: for normal, 2: for twice faster, 0.5: for twice slower\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

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

                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Client-- %02d:%02d:%02d - You can't play a music at 0 or less speed.\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
                else if ((*endptr == '\0') || (isspace(*endptr) != 0))
                    correct_speed = TRUE;
            }

            int correct_volume = FALSE;
            char volume_wanted[MAX_LENGTH];
            while (!correct_volume)
            {
                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Please type here, the wanted volume.\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
                // , 0.5: for twice more quiet
                printf("--Client-- %02d:%02d:%02d - examples, 1: for normal, 2: for twice louder (only interger are currently accepted :/)\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                if (fgets(volume_wanted, sizeof(volume_wanted), stdin) == NULL)
                {
                    /* Unexpected error */
                    perror("Error client: volume fgets !");
                    exit(1);
                }

                // parse the string into float
                volume = atoi(volume_wanted);
                // parse the string into float
                // volume = strtod(volume_wanted, &endptr);
                printf("--Client-- *%f*\n", volume);
                if (volume < 0)
                {
                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Client-- %02d:%02d:%02d - You cannot play music at a negative volume. Duh.\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
                else if ((*endptr == '\0') || (isspace(*endptr) != 0))
                    correct_volume = TRUE;
            }

            char channels_wanted[MAX_LENGTH];
            while (channels == 0)
            {
                // REFACTOR: just ask for mono or stereo

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Mono?(y/yes or n/no)\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                if (fgets(channels_wanted, MAX_LENGTH, stdin) == NULL)
                {
                    /* Unexpected error */
                    perror("Error client: channels fgets !");
                    exit(1);
                }
                channels_wanted[strlen(channels_wanted) - 1] = '\0';

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - String read: *%s*\n", tm.tm_hour, tm.tm_min, tm.tm_sec, channels_wanted);

                // Convert to upper case
                int j = 0;
                while (channels_wanted[j] != '\0')
                {
                    channels_wanted[j] = toupper(channels_wanted[j]);
                    j++;
                }

                // or just compare the first letter
                if (
                    strcmp(channels_wanted, "YES") == 0 ||
                    strcmp(channels_wanted, "Y") == 0 ||
                    strcmp(channels_wanted, "OUI") == 0 ||
                    strcmp(channels_wanted, "O") == 0 ||
                    strcmp(channels_wanted, "MONO") == 0)
                {
                    channels = 1;
                }
                else if (
                    strcmp(channels_wanted, "NO") == 0 ||
                    strcmp(channels_wanted, "NON") == 0 ||
                    strcmp(channels_wanted, "N") == 0 ||
                    strcmp(channels_wanted, "STEREO") == 0)
                {
                    channels = 2;
                }
                else
                {

                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Client-- %02d:%02d:%02d - Wrong answer. Please type (Y/N/Yes/No/Oui/Non/O/Mono/Stereo)\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
                }
            }
        }

        // -- data reply from the server --

        t = time(NULL);
        tm = *localtime(&t);
        printf("--Client-- %02d:%02d:%02d - Listening for incoming messages...\n\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

        // default value
        int server_message; // = -1;

        // -- DATA --

        int sample_rate;
        int sample_size;
        // int channels;

        // -- END DATA --

        for (int count = 0; count < 3; count++)
        {
            // reset server msg
            server_message = -1;
            while (server_message == -1)
            {

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Send State: Ready\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
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

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Wait Audio Data\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                select_num = select(socket_descriptor + 1, &read_set, NULL, NULL, &read_timeout);
                if (select_num < 0)
                {
                    perror("Error client: select() !");
                    exit(1);
                }
                if (select_num == 0)
                {
                    timeout_number++;

                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Client-- %02d:%02d:%02d - Timeout on audio data\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
                    // resend the client_ready until the server send us something
                    // FIXME: Reset timeout timer after each one
                    continue;
                }
                if (FD_ISSET(socket_descriptor, &read_set))
                {
                    reception_len = recvfrom(
                        socket_descriptor,
                        &server_message,
                        sizeof(server_message),
                        0,
                        (struct sockaddr *)&from,
                        &flen);

                    if (reception_len < 0)
                    {
                        perror("Error client: Msg Reception !");
                        exit(1);
                    }
                }
            }

            if (count == 0)
            {
                sample_rate = server_message;

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - sample_rate:\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
            }
            else if (count == 1)
            {
                sample_size = server_message;

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - sample_size:\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
            }
            else if (count == 2)
            {
                if (channels == 0)
                {
                    channels = server_message;

                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Client-- %02d:%02d:%02d - Number of channels overwrite:%d\n", tm.tm_hour, tm.tm_min, tm.tm_sec, channels);
                }
                else
                {

                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Client-- %02d:%02d:%02d - Channels Choosen: %d. Music Channels:\n", tm.tm_hour, tm.tm_min, tm.tm_sec, channels);
                }
            }

            printf("Received %d bytes from host %s port, %d: %i\n", reception_len,
                   inet_ntoa(from.sin_addr), ntohs(from.sin_port), server_message);
        }

        // -- Creation of the audio descriptor --

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
                // audio_chunk_loop:

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Send State: Ready\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
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

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - Wait Audio Chunk\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                select_num = select(socket_descriptor + 1, &read_set, NULL, NULL, &read_timeout);
                if (select_num < 0)
                {
                    perror("Error client: select() !");
                    exit(1);
                }
                if (select_num == 0)
                {
                    timeout_number++;

                    t = time(NULL);
                    tm = *localtime(&t);
                    printf("--Client-- %02d:%02d:%02d - Timeout on audio chunk\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                    count--;
                    // FIXME: Reset timeout timer after each one
                    continue;
                }
                if (FD_ISSET(socket_descriptor, &read_set))
                {
                    if (count == 0)
                    {
                        reception_len = recvfrom(
                            socket_descriptor,
                            &byte_left,
                            sizeof(byte_left),
                            0,
                            (struct sockaddr *)&from,
                            &flen);

                        if (reception_len < 0)
                        {
                            perror("Error client: Music Byte Reception !");
                            exit(1);
                        }

                        t = time(NULL);
                        tm = *localtime(&t);
                        printf("--Client-- %02d:%02d:%02d - music's bytes: %zd\n", tm.tm_hour, tm.tm_min, tm.tm_sec, byte_left);
                    }
                    else if (count == 1)
                    {

                        reception_len = recvfrom(
                            socket_descriptor,
                            buffer,
                            sizeof(buffer),
                            0,
                            (struct sockaddr *)&from,
                            &flen);

                        if (reception_len < 0)
                        {
                            perror("Error client: Buffer Reception !");
                            exit(1);
                        }

                        t = time(NULL);
                        tm = *localtime(&t);
                        printf("--Client-- %02d:%02d:%02d - buffer:\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
                    }
                }

                printf("Received %d bytes from host %s port, %d\n", reception_len,
                       inet_ntoa(from.sin_addr), ntohs(from.sin_port));
            }
            if (byte_left <= 0)
            {

                t = time(NULL);
                tm = *localtime(&t);
                printf("--Client-- %02d:%02d:%02d - The music ends\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

                // FIXME: There is some pop (sound bug) at the end/or start (after the timeout management update)

                // the break is mandatory to avoid copypaste code
                // and to ensure that we don't ask the computer to read 0 bytes
                break;
            }

            // -- Volume Control --
            for (int i = 0; i < sizeof(buffer); i++)
            {
                buffer[i] = volume * buffer[i];
                // printf("%d ", buffer[i]);
                // unsigned cast = volume * buffer[i];
                // buffer[i] = cast;
                // printf("m: %d ", buffer[i]);
            }
            // printf(" \n");
            // -- Volume Control --

            write(audio_descriptor, buffer, byte_left);
        } while (byte_left > 0);

        if (byte_left == -1)
        {
            perror("Error read !");
            exit(1);
        }
    }
    // close_client:
    // TODO: feature - Close it after a user timeout (in music name selection)
    // -- Close the Socket --

    t = time(NULL);
    tm = *localtime(&t);
    printf("--Client-- %02d:%02d:%02d - Close the Socket\n", tm.tm_hour, tm.tm_min, tm.tm_sec);

    int close_err = close(socket_descriptor);

    if (close_err < 0)
    {
        perror("Error client: Socket closing !");
        exit(1);
    }
}