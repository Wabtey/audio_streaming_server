#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
// #include <strings.h>
#include <string.h>

#include "../include/audio.h"

#define MAX_LENGTH 50
#define TRUE 1
#define FALSE 0

int main(int argc, char *argv[])
{
    char music_file[MAX_LENGTH];
    fgets(music_file, MAX_LENGTH, stdin);
    // Remove the escape character from the user input
    music_file[strlen(music_file) - 1] = '\0';

    printf("Chaine lue: *%s*\n", music_file);

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

    // -- Creation of the audio descriptior --

    int audio_descriptor = aud_writeinit(sample_rate, sample_size, channels);
    if (audio_descriptor < 0)
    {
        perror("Error aud_writeinit !");
        exit(1);
    }

    // -- Read and write the wave file using the 2 previous descriptors  --

    unsigned char buffer[1024];

    ssize_t byte_left = read(fd, buffer, 1024);
    while (byte_left > 0)
    {
        // play in the audio output
        write(audio_descriptor, buffer, byte_left);
        byte_left = read(fd, buffer, byte_left);
    }

    if (byte_left == -1)
    {
        perror("Error read !");
        exit(1);
    }
}