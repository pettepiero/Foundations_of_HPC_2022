CC = gcc
CFLAGS = -pg -Wall  -fstrict-aliasing -Wstrict-aliasing
SRCDIR = src
BINDIR = bin
INCDIR = include

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(BINDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x

.PHONY: clean debug

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(BINDIR)/*.o $(EXEC)

debug: $(EXEC)
	gdb $(EXEC)
