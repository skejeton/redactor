CC?=clang
AR?=llvm-ar
OUTPUT?=bin/test
SYS_LIBS?=-lSDL2 -lSDL2_ttf -lm -lc

SHARED_CFLAGS=-O0 -g -fsanitize=address

SRC_CFLAGS=-I. -Isrc -MD
SRC_LIBS=$(SYS_LIBS) 
SRC_UNITS=$(shell find src/ -type f -name "*.c" -not -name "Main.c") $(shell find tests/ -type f -name "*.c")
SRC_HEADERS=$(shell find src/ -type f -name "**.h" -o "**.inl") $(shell find tests/ -type f -name "**.h" -o "**.inl")
SRC_OBJECTS=$(SRC_UNITS:.c=.o)
SRC_DEPS=$(SRC_OBJECTS:.o=.d)

all: $(OUTPUT)
	$(OUTPUT) $(args)

%.o: %.c
	$(CC) $(SHARED_CFLAGS) $(SRC_CFLAGS) -c $< -o $@ -DCFLAGS="\"$(SHARED_CFLAGS)\""

$(OUTPUT): $(SRC_OBJECTS)
	$(CC) $(SHARED_CFLAGS) $(SRC_CFLAGS) $(SRC_OBJECTS) $(SRC_LIBS) -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	rm -f $(SRC_OBJECTS)
	rm -f $(SRC_DEPS)

include $(wildcard $(SRC_DEPS))
