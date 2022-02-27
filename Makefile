FILES = ./src/*.c
LDFLAGS = -lSDL2 -lSDL2_ttf -lm
OPTFLAGS = -g -fsanitize=address

all:
	cc $(FILES) $(LDFLAGS) $(OPTFLAGS)

run: all
	./a.out