all:
	gcc -I include -L lib -o bin/main src/*.c -lmingw32 -lSDL2main  -lSDL2 -lSDL2_ttf -lSDL2_image
