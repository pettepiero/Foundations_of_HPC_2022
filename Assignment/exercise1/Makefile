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
OPENMP_SRCS = $(SRCDIR)/gol.c,$(SRCDIR)/read_write_pgm_image.c,$(SRCDIR)/dynamics.c
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x
EXEC_MPI = $(BINDIR)/mpi_gol.x

.PHONY: clean debug profile openmp

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(EXEC_MPI): $(OBJS)
	$(MPICC) $(OBJS) -o $(EXEC_MPI)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

profile: CFLAGS += $(PROFILEFLAGS)
profile: $(EXEC)

openmp: clean
openmp: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS) $(MPIFLAG)
openmp: OPENMP_OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(OPENMP_SRCS))
openmp: $(OPENMP_OBJS)
	$(CC) $(CFLAGS) $(OPENMP_OBJS) -o $(EXEC)

mpi: clean
mpi: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS)
mpi: $(EXEC_MPI)


clean:
	rm -f $(OBJDIR)/*.o $(EXEC)
	rm -f $(BINDIR)/*.o $(EXEC)

