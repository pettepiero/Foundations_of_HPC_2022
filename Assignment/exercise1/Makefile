CC = gcc
CFLAGS = -pg -Wall -fopenmp -fstrict-aliasing -Wstrict-aliasing
PROFILEFLAGS = -DPROFILING
#maybe add -pg to PROFILEFLAGS
DEBUGFLAGS = -DDEBUG
SRCDIR = src
BINDIR = build
INCDIR = include
OBJDIR = $(BINDIR)/objs

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x

.PHONY: clean debug

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

profile: CFLAGS += $(PROFILEFLAGS)
profile: $(EXEC)

clean:
	rm -f $(OBJDIR)/*.o $(EXEC)
	rm -f $(BINDIR)/*.o $(EXEC)
