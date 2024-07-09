CC = gcc
CFLAGS = -pg -Wall -fstrict-aliasing -Wstrict-aliasing -g
PROFILEFLAGS = -DPROFILING
OPENMPFLAGS = -fopenmp
DEBUGFLAGS = -DDEBUG
SRCDIR = src
BINDIR = build
INCDIR = include
OBJDIR = $(BINDIR)/objs

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x

.PHONY: clean debug profile openmp

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(PROFILEFLAGS) -I$(INCDIR) -c $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

profile: CFLAGS += $(PROFILEFLAGS)
profile: $(EXEC)

openmp: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS)
openmp: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJDIR)/*.o $(EXEC)

# Default target without _OPENMP
default: $(EXEC)
