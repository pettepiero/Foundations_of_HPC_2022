CC = mpicc  
CFLAGS = -Wall -fstrict-aliasing -Wstrict-aliasing -DUSE_MPI -fno-omit-frame-pointer 
OPTFLAGS = -O3 -march=native -ffast-math
SRCDIR = src
BINDIR = build
INCDIR = include
IMGDIR = images
SNPDIR = $(IMGDIR)/snapshots
OBJDIR = $(BINDIR)/objs
#SRCS = $(wildcard $(SRCDIR)/*.c)
SRCS = $(filter-out $(SRCDIR)/smaller_gol.c, $(wildcard $(SRCDIR)/*.c))
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
SMALL_SRCS = $(filter-out $(SRCDIR)/gol.c, $(SRCS))
SMALL_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SMALL_SRCS))
EXEC = $(BINDIR)/gol.x
SMALLEREXEC = $(BINDIR)/smaller_gol.x
OMP_FLAGS = -fopenmp

default: OMP_FLAGS = -fopenmp
default: $(EXEC)

smaller_gol: $(SMALLEREXEC) 

$(SMALLEREXEC): $(SMALL_OBJS)
	$(CC) $(CFLAGS) $(OPTFLAGS) $(OMP_FLAGS) $(SMALL_OBJS) -o $(SMALLEREXEC)

serial: $(OBJS)
	$(CC) -pg -O0 $(CFLAGS) $(OMP_FLAGS) $(OBJS) -o $(BINDIR)/serial-gol.x
	

dirs:
	mkdir -p $(BINDIR) $(OBJDIR) $(IMGDIR) $(SNPDIR)
$(OBJDIR)/%.o: $(SRCDIR)/%.c | dirs
	$(CC) $(CFLAGS) $(OPTFLAGS) $(OMP_FLAGS) -I$(INCDIR) -c $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OPTFLAGS) $(OMP_FLAGS) $(OBJS) -o $(EXEC)

mpiP:	$(OBJS)
	$(CC) $(CFLAGS) $(OPTFLAGS) $(OMP_FLAGS) $(OBJS) -o $(EXEC) -lmpiP -liberty -lbfd -lunwind

noomp: $(OBJS)
	$(CC) $(CFLAGS) $(OPTFLAGS) $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJDIR)/*.o $(EXEC) $(SMALLEREXEC)

.PHONY: clean default mpiP noomp dirs

