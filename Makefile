FILES = $(shell echo src/*.c src/Redex/*.c)
TEST_FILES = $(shell echo tests/*/Main.c) $(shell echo tests/*.c) $(filter-out src/Main.c, $(FILES))
HEADERS = $(shell find -name "*.h")
TEST_HEADERS = $(HEADERS)
OBJECTS = $(FILES:.c=.o)
TEST_OBJECTS = $(TEST_FILES:.c=.o) 
LDFLAGS = -lSDL2 -lSDL2_ttf -lm -lc
ASANFLAGS ?= -fsanitize=address
ASAN ?= -lasan 
CCFLAGS = -I. -g -Wall $(ASANFLAGS)
#CCFLAGS = -O3 -g -flto
EXECUTABLE = ./bin/redactor
TEST_EXECUTABLE = ./bin/test

all: $(FILES) $(EXECUTABLE) $(HEADERS) Makefile

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(ASAN) $(CCFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ 

# NOTE: This rebuilds the entire project when header is changed!
%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

test: $(TEST_SOURCES) $(TEST_EXECUTABLE) $(HEADERS) $(TEST_HEADERS) Makefile
	./bin/test

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	$(CC) $(ASAN) $(CCFLAGS) $(LDFLAGS) $(TEST_OBJECTS) -o $@

run: all
	./bin/redactor 

clean:
	@touch $(OBJECTS)
	rm $(OBJECTS) 
