FILES = $(shell echo src/*.c)
TEST_FILES = $(shell echo tests/*.c) $(filter-out src/Main.c, $(FILES))
HEADERS = $(shell echo src/*.h)
TEST_HEADERS = $(shell echo src/*.h) $(shell echo tests/*.h)
OBJECTS = $(FILES:.c=.o)
TEST_OBJECTS = $(TEST_FILES:.c=.o) 
LDFLAGS = -lSDL2 -lSDL2_ttf -lm -lc
OPTFLAGS = -g -fsanitize=address -Wall
ASAN = -lasan 
EXECUTABLE = ./bin/redactor
TEST_EXECUTABLE = ./bin/test

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	cc $(ASAN) $(OPTFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ 

# NOTE: This rebuilds the entire project when header is changed!
%.o: %.c $(HEADERS) $(TEST_HEADERS)
	cc $(OPTFLAGS) -c $< -o $@

test: $(TEST_SOURCES) $(TEST_EXECUTABLE)
	./bin/test

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	cc $(ASAN) $(OPTFLAGS) $(LDFLAGS) $(TEST_OBJECTS) -o $@

run: all
	./bin/redactor 

clean:
	rm $(OBJECTS)
