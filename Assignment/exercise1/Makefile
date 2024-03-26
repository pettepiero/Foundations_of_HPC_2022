CC = gcc
CFLAGS = -g -Wall

SRCS = gol.c read_write_pgm_image.c
OBJS = $(SRCS:.c=.o)
EXEC = gol.x

.PHONY: clean debug

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

debug: $(EXEC)
	gdb $(EXEC)

