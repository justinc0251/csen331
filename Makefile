# Makefile for IEEE 802.11 Frame Exchange Assignment

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

# Source files
CLIENT_SRC = client.c
SERVER_SRC = server.c
COMMON_SRC = 

# Header files
HEADERS = frame.h

# Output executables
CLIENT = client
SERVER = server

# Default target
all: $(CLIENT) $(SERVER)

# Client target
$(CLIENT): $(CLIENT_SRC) $(COMMON_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRC) $(COMMON_SRC) $(LDFLAGS)

# Server target
$(SERVER): $(SERVER_SRC) $(COMMON_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRC) $(COMMON_SRC) $(LDFLAGS)

# Clean target
clean:
	rm -f $(CLIENT) $(SERVER) *.o

# Run targets for testing
run-server:
	./$(SERVER)

run-client:
	./$(CLIENT)

# Phony targets
.PHONY: all clean run-server run-client