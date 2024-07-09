CC = gcc
MPICC = mpicc
CFLAGS = -pg -Wall -fstrict-aliasing -Wstrict-aliasing
PROFILEFLAGS = -DPROFILING
OPENMPFLAGS = -fopenmp
DEBUGFLAGS = -DDEBUG
MPIFLAG = -DMPI
SRCDIR = src
BINDIR = build
INCDIR = include
OBJDIR = $(BINDIR)/objs

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x
EXEC_MPI = $(BINDIR)/mpi_gol.x

.PHONY: clean debug profile openmp mpi

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(EXEC): $(OBJS)
	$(MPICC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(EXEC_MPI): $(OBJS)
	$(MPICC) $(CFLAGS) $(OBJS) -o $(EXEC_MPI)

profile: CFLAGS += $(PROFILEFLAGS)
profile: $(EXEC)

openmp: clean
openmp: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS) $(MPIFLAG)
openmp: OPENMP_OBJS = $(filter $(OBJDIR)/gol.o $(OBJDIR)/read_write_pgm_image.o $(OBJDIR)/dynamics.o,$(OBJS))
openmp: $(OPENMP_OBJS)
	$(CC) $(CFLAGS) $(OPENMP_OBJS) -o $(EXEC)

mpi: clean
mpi: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS)
mpi: $(EXEC_MPI)

clean:
	rm -f $(OBJDIR)/*.o $(EXEC) $(EXEC_MPI)

# Default target without _OPENMP
default: $(EXEC)
