CC = mpicc  
CFLAGS = -Wall -fstrict-aliasing -Wstrict-aliasing -DUSE_MPI -fno-omit-frame-pointer 
OPTFLAGS = -O3 -march=native -ffast-math
SRCDIR = src
BINDIR = build
INCDIR = include
IMGDIR = images
SNPDIR = $(IMGDIR)/snapshots
OBJDIR = $(BINDIR)/objs
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x
OMP_FLAGS = -fopenmp

default: OMP_FLAGS = -fopenmp
default: $(EXEC)

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
	rm -f $(OBJDIR)/*.o $(EXEC)

.PHONY: clean default mpiP noomp dirs

