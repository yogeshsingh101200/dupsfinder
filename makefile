CC = gcc
CFLAGS = -Wall -O2
LIBS = -lcrypto
TARGET = dupsfinder
SRCS = main.c finder.c hashes.c xxhash.c stack.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm *.o