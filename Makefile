all:
	cc ./src/buffer.c ./src/main.c -lSDL2 -lSDL2_ttf -fsanitize=address -g

run: all
	./a.out

