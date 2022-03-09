FILES = $(shell echo src/*.c)
TEST_FILES = $(shell echo tests/*.c) $(filter-out src/main.c, $(FILES))
HEADERS = $(shell echo src/*.h)
TEST_HEADERS = $(shell echo src/*.h) $(shell echo tests/*.h)
OBJECTS = $(FILES:.c=.o)
TEST_OBJECTS = $(TEST_FILES:.c=.o) 
LDFLAGS = -lSDL2 -lSDL2_ttf -lm -lc
OPTFLAGS = -O3 #-fsanitize=address
ASAN = #-lasan
EXECUTABLE = ./bin/a.out
TEST_EXECUTABLE = ./bin/test

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	clang $(ASAN) $(LDFLAGS) $(OBJECTS) -o $@ -flto

# NOTE: This rebuilds the entire project when header is changed!
%.o: %.c $(HEADERS) $(TEST_HEADERS)
	cc $(OPTFLAGS) -c $< -o $@

test: $(TEST_SOURCES) $(TEST_EXECUTABLE)
	./bin/test

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	clang $(ASAN) $(LDFLAGS) $(TEST_OBJECTS) -o $@

run: all
	./bin/a.out

clean:
	rm $(OBJECTS)