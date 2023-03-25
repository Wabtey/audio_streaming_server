---
authors: "Dehaye Gabriel", "Epain Florian"
---

# Documentation Projet de Systèmes: streaming audio

## Commandes utilisateur

Dans un premier temps, il faut se situer dans le dossier TP_Projet et exécuter la commande `make server`.
Ensuite, pour exécuter audioserver dans un terminal, il suffit de taper `make executeserver`.
Pour exécuter audioclient dans un terminal, il suffit de taper `make executeclient`.

Après, le client attend qu'on lui rentre l'emplacement du fichier à lire, pour qu'il puisse le demander au serveur.
Ici, il faut donc rentrer `assets/audio/test.wav` .

`fgets(music_file, MAX_LENGTH, stdin);`

## Communication entre le serveur et le client

Le serveur receptionne ensuite le message du client et lui envoie dans cet ordre:

- sample_rate
- sample_size
- channels

```c
send_err = sendto(
    socket_descriptor,
    &send_msg,
    sizeof(send_msg),
    0,
    (struct sockaddr *)&dest,
    sizeof(struct sockaddr_in));
```

Le client crée le descripteur audio puis attend du serveur le nombre de byte à lire `byte_left` et le `buffer` contenant une partie à lire du fichier audio. La lecture continue ainsi jusqu'à ce que le serveur n'envoie plus rien au client. Cela permet de lire la musique.

```c
// -- Write the audio from the data, and the chunk of music the server sends us --

unsigned char buffer[1024];
ssize_t byte_left;

do
{
    // wait for buffer and the music_bytes
    for (int count = 0; count < 2; count++)
    {
        printf("--Client-- Send State: Ready\n");
        
        // ! envoi du message indicant que le client est prêt à recevoir !

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
```
