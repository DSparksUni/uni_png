IDIR = include
SDIR = src
ODIR = obj
TDIR = test

vpath %.c $(SDIR)

CC = gcc
CFLAGS = -I $(IDIR) -Wall -Wextra -Wno-missing-braces \
		 -Wconversion -pedantic -std=c11 -ggdb
TARGET = uni_png.exe
TEST = penguin.png

__OBJ = main.o chunks.o
OBJ = $(patsubst %.o,$(ODIR)/%.o,$(__OBJ))

run: all
	clear
	./$(TARGET) $(TDIR)/$(TEST)

all: $(TARGET)
	echo Build Complete!

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

$(ODIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean
clean:
	rm $(TARGET) $(OBJ)
