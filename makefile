CC = gcc
CFLAGS = -O
LIBS = -lcrypto
TARGET = main2
SRCS = main.c finder.c xxhash.c
OBJS = $(SRCS:.c=.o)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm *.o