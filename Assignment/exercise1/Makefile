CC = gcc
MPICC = mpicc
CFLAGS = -pg -Wall -fstrict-aliasing -Wstrict-aliasing
PROFILEFLAGS = -DPROFILING
OPENMPFLAGS = -fopenmp
#maybe add -pg to PROFILEFLAGS
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

.PHONY: clean debug

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(EXEC_MPI): $(OBJS)
	$(MPICC) $(OBJS) -o $(EXEC)


$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

profile: CFLAGS += $(PROFILEFLAGS)
profile: $(EXEC)

openmp: clean
openmp: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS) $(MPIFLAG)
openmp: $(EXEC)

mpi: clean
mpi: CFLAGS += $(OPENMPFLAGS) $(PROFILEFLAGS)
mpi: $(EXEC_MPI)


clean:
	rm -f $(OBJDIR)/*.o $(EXEC)
	rm -f $(BINDIR)/*.o $(EXEC)


mpicc -fopenmp -DTIME -o $execname $filename $libname