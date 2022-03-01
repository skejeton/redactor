FILES = $(shell echo src/*.c)
HEADERS = $(shell echo src/*.h)
OBJECTS = $(FILES:.c=.o)
LDFLAGS = -lSDL2 -lSDL2_ttf -lm -lc
OPTFLAGS = -g -fsanitize=address
EXECUTABLE = ./a.out

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	clang -lasan $(LDFLAGS) $(OBJECTS) -o $@

# NOTE: This rebuilds the entire project when header is changed!
%.o: %.c $(HEADERS)
	cc $(OPTFLAGS) -c $< -o $@

run: all
	./a.out

clean:
	rm $(OBJECTS)