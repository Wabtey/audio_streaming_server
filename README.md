# TP Projet System

10/02/2023 -> ???

Gabriel DEHAYE et
Florian Emmanuelle EPAIN
Grp 1.2

## Execution

`$ make all` into `$ make execute`, and type the music's locations from the repertory root.

```text
$ padsp ./src/lecteur

assets/audio/test.wav
```

## TP4/5

On doit lire une chaîne de caratcères rentrée par l'utilisateur.ice.

```c
char music_file[MAX_LENGTH];
fgets(music_file, MAX_LENGTH, stdin);
// Remove the escape character from the user input
music_file[strlen(music_file) - 1] = '\0';

printf("Chaine lue: *%s*\n", music_file);
```

Créer un file descriptor qui va remplir les champs:
sample_rate, sample_size, channels

```c
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
```

Créer un audio descriptor avec les champs précédemment remplis.

```c
// -- Creation of the audio descriptior --

int audio_descriptor = aud_writeinit(sample_rate, sample_size, channels);
if (audio_descriptor < 0)
{
    perror("Error aud_writeinit !");
    exit(1);
}
```

Créer un buffer, lire une première fois le file descriptor dans le buffer, tant qu'il reste des bytes à lire on joue dans la sortie audio, en utilisant l'audio descriptor, et on lit les restes.

```c
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
```

Le programme lit bien le fichier audio rentré, sans problème.

### Questions

1. Plus la fréquence est grande, plus la vitesse de lecture est rapide. Avec une fréquence deux fois plus importante, on obtient une vitesse deux fois plus élevée.

    ```c
    // -- Creation of the audio descriptior --

    // double the sample rate
    int audio_descriptor = aud_writeinit(sample_rate*2, sample_size, channels);
    if (audio_descriptor < 0)
    {
        perror("Error aud_writeinit !");
        exit(1);
    }
    ```

2. Un fichier wave est orginisé en une suite d'échantillons. Un fichier stéréo aura le samble gauche puis sample droit, et lit deux par deux les samples. Un fichier mono a juste ses samples à la suite, et lit 1 par 1. Lire un fichier stéréo comme mono va produire un son ralenti par deux, sortant dans la gauche et droite.

    ```c
    // -- Creation of the audio descriptior --

    // Force to mono
    int audio_descriptor = aud_writeinit(sample_rate, sample_size, 1);
    if (audio_descriptor < 0)
    {
        perror("Error aud_writeinit !");
        exit(1);
    }
    ```

3. 8bits instead of 16bits. or 32b instead of 16 = do not try.
   This will create a distorded song and a big noisy song on top of the music.

    ```c
    // -- Creation of the audio descriptior --

    // Welcome to Hell
    int audio_descriptor = aud_writeinit(sample_rate, sample_size/2, channels);
    if (audio_descriptor < 0)
    {
        perror("Error aud_writeinit !");
        exit(1);
    }
    ```

### Note

Nous avons été aidé par Pierre Guillaume, pour le makefile (pb de compilation du fichier audio.c) et pour la réalisation de la lecture audio.

## TP6+ - AudioServer/Client

### Strategie

Lae client-e envoie le nom d'une musique au serveur.
Le serveur envoie le .wav petit a petit, avec qq infos (channels, freq, bitrate, etc)
