FILES = ./src/buffer.c ./src/main.c ./src/font.c ./src/docview.c
LDFLAGS = -lSDL2 -lSDL2_ttf -lm
OPTFLAGS = -g

all:
	cc $(FILES) $(LDFLAGS) $(OPTFLAGS)

run: all
	./a.out

