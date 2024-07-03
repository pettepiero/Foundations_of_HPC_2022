CC = gcc
CFLAGS = -pg -Wall -fstrict-aliasing -Wstrict-aliasing
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
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

profile: CFLAGS += $(PROFILEFLAGS)
profile: $(EXEC)

openmp: clean
openmp: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS)
openmp: OPENMP_OBJS = $(filter $(OBJDIR)/gol.o $(OBJDIR)/read_write_pgm_image.o $(OBJDIR)/dynamics.o,$(OBJS))
openmp: $(OPENMP_OBJS)
	$(CC) $(CFLAGS) $(OPENMP_OBJS) -o $(EXEC)

clean:
	rm -f $(OBJDIR)/*.o $(EXEC)

# Default target without _OPENMP
default: $(EXEC)
