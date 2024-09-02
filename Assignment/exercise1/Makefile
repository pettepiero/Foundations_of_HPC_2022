CC = gcc
MPICC = mpicc
CFLAGS = -Wall -fstrict-aliasing -Wstrict-aliasing -g
PROFILEFLAGS = -pg -DPROFILING
OPENMPFLAGS = -fopenmp
DEBUGFLAGS = -DDEBUG
MPIFLAGS = -DUSE_MPI
SRCDIR = src
BINDIR = build
INCDIR = include
OBJDIR = $(BINDIR)/objs

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
# Define a separate set of object files for mpi
MPI_OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/mpi/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x

# Default target
default: $(EXEC)

# Default rule to compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(PROFILEFLAGS) -I$(INCDIR) -c $< -o $@

# Rule to link the executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

# Profile target
profile: CFLAGS += $(PROFILEFLAGS)
profile: $(EXEC)

# OpenMP target
openmp: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS)
openmp: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

# MPI target
# Define the pattern rule for mpi separately
$(OBJDIR)/mpi/%.o: $(SRCDIR)/%.c
	$(MPICC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

mpi: $(MPI_OBJS)
	$(MPICC) $(CFLAGS) $(MPIFLAGS) $(MPI_OBJS) -g -O0 -o $(EXEC)

# Clean target
clean:
	rm -f $(OBJDIR)/*.o $(EXEC) $(OBJDIR)/mpi/*.o

.PHONY: clean default profile openmp mpi
