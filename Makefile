FILES = $(shell echo src/*.c)
OBJECTS = $(FILES:.c=.o)
LDFLAGS = -lSDL2 -lSDL2_ttf -lm -lc
OPTFLAGS = -g -fsanitize=address
EXECUTABLE = ./a.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	clang $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	cc -c $< -o $@

run: all
	./a.out

clean:
	rm $(OBJECTS)