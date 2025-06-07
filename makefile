# compiler variables
CC = gcc
CFLAGS = -Wall -Wextra

#executable
EXE1 = main

#find source
SRC1 = main.c flooding.c

all: $(EXE1)

$(EXE1): $(SRC1)
		$(CC) $(CFLAGS) -o $(EXE1) $(SRC1)

clean: 
		rm -f $(EXE1)