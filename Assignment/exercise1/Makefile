CC = mpicc 
CFLAGS = -Wall -fstrict-aliasing -Wstrict-aliasing -g -fopenmp -DUSE_MPI
SRCDIR = src
BINDIR = build
INCDIR = include
IMGDIR = images
SNPDIR = $(IMGDIR)/snapshots
OBJDIR = $(BINDIR)/objs
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
EXEC = $(BINDIR)/gol.x

default: $(EXEC)

dirs:
	mkdir -p $(BINDIR) $(OBJDIR) $(IMGDIR) $(SNPDIR)
$(OBJDIR)/%.o: $(SRCDIR)/%.c | dirs
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

clean:
	rm -f $(OBJDIR)/*.o $(EXEC)

.PHONY: clean default dirs

