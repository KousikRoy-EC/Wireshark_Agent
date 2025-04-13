# Variable for compiler
CC = gcc

# Compiler flags (optional but recommended)
CFLAGS = -Wall -Wextra -g

# Output executable name
TARGET = wifi_agent

# Source files
SRCS = wifi_agent.c

# Object files (replace .c with .o)
OBJS = $(SRCS:.c=.o)

# Default rule
all: $(TARGET)

# Rule to build the target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Rule to compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove compiled files
clean:
	rm -f $(TARGET) $(OBJS)
