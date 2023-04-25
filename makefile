#
# Link
#

CFLAGS = -Wall -g

all: ./main # src/lecteur

# -- V1 --

execute:
	padsp ./src/lecteur

main: ./lecteur.o ./audio.o
	gcc $(CFLAGS) -o ./src/lecteur ./obj/lecteur.o ./obj/audio.o

lecteur.o: ./src/lecteur.c ./include/audio.h
	mkdir -p obj 
	gcc $(CFLAGS) -c ./src/lecteur.c -o obj/lecteur.o

audio.o: ./src/audio.c ./include/audio.h
	mkdir -p obj 
	gcc $(CFLAGS) -c ./src/audio.c -o obj/audio.o

# -- V2 --

executeclient:
	padsp ./src/audioclient

executeserver:
	./src/audioserver

server: ./audioserver.o ./audioclient.o ./audio.o
	gcc $(CFLAGS) -o ./src/audioserver ./obj/audioserver.o ./obj/audio.o
	gcc $(CFLAGS) -o ./src/audioclient ./obj/audioclient.o ./obj/audio.o

audioserver.o: ./src/audioserver.c ./include/audio.h
	mkdir -p obj
	gcc $(CFLAGS) -c ./src/audioserver.c -o ./obj/audioserver.o

audioclient.o: ./src/audioclient.c ./include/audio.h
	mkdir -p obj
	gcc $(CFLAGS) -c ./src/audioclient.c -o ./obj/audioclient.o

#
# BIN
#

# bin/lecteur: obj/lecteur.o include/audio.h

# 	gcc $(CFLAGS) -o src/lecteur obj/lecteur.o 

#
# Objets
#

obj/lecteur.o: src/lecteur.c include/audio.h
# Folder obj exists 
	mkdir -p obj    			
# Compile
	gcc $(CFLAGS) -c src/lecteur.c -I../include -o obj/lecteur.o

obj/audio.o: src/audio.c include/audio.h
	mkdir -p obj
	gcc $(CFLAGS) -c src/audio.c -o obj/audio.o

obj/audioclient.o: src/audioclient.c # include/audio.h
	gcc $(CFLAGS) -c src/audioclient.c -I../include -o obj/audioclient.o

obj/audioserver.o: src/audioserver.c # include/audio.h
	gcc $(CFLAGS) -c src/audioserver.c -I../include -o obj/audioserver.o

#
# Remove files
#

clean :
	rm obj/*.o src/lecteur