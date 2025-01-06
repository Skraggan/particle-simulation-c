all:
	gcc -I include -L lib -o bin/main src/*.c -lmingw32 -lSDL2ttf -lSDL2
