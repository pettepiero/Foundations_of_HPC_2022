CC = mpicc
CFLAGS = -Wall -fstrict-aliasing -Wstrict-aliasing -g -fopenmp -DUSE_MPI
SRCDIR = src
BINDIR = build
INCDIR = include
OBJDIR = $(BINDIR)/objs
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x

# Default target
default: $(EXEC)

# Rule to compile object files with MPI and OpenMP support
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Rule to link the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

# Clean target
clean:
	rm -f $(OBJDIR)/*.o $(EXEC)

.PHONY: clean default
