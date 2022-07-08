CC?=clang
AR?=llvm-ar
OUTPUT?=bin/redactor
SYS_LIBS?=-lSDL2 -lSDL2_ttf -lm -lc

SHARED_CFLAGS?=-O0 -fsanitize=address

SRC_CFLAGS=-I. -Isrc -MD
SRC_LIBS=$(SYS_LIBS) 
SRC_UNITS=$(shell find src/ -type f -name "*.c")
SRC_HEADERS=$(shell find src/ -type f -name "**.h" -o "src/**.inl")
SRC_OBJECTS=$(SRC_UNITS:.c=.o)
SRC_DEPS=$(SRC_OBJECTS:.o=.d)

all: $(OUTPUT)

run: all
	$(OUTPUT) tests/TestInputUnicodeBasic.txt

test:     
	@make -f Test.mk

%.o: %.c
	$(CC) $(SHARED_CFLAGS) $(SRC_CFLAGS) -c $< -o $@

$(OUTPUT): $(SRC_OBJECTS)
	$(CC) $(SHARED_CFLAGS) $(SRC_CFLAGS) $(SRC_OBJECTS) $(SRC_LIBS) -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)
	rm -f $(SRC_OBJECTS)
	rm -f $(SRC_DEPS)
	make -f Test.mk clean

include $(wildcard $(SRC_DEPS))
.PHONY: all run test clean
