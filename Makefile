CC     = g++
INCDIR = include
CFLAGS = -std=c++11

SRCDIR = src
TESTDIR = test

BINDIR = bin
SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(BINDIR)/%.o, $(SRCS))

MAKEFS_SRC = $(SRCDIR)/makeFileSystem.cpp
OPERFS_SRC = $(SRCDIR)/fileSystemOper.cpp

MAKEFS_O = $(BINDIR)/makeFileSystem.o
OPERFS_O = $(BINDIR)/fileSystemOper.o

MAKEFS_OBJS = $(BINDIR)/fat12.o $(MAKEFS_O)
OPERFS_OBJS = $(BINDIR)/fat12.o $(OPERFS_O)

print:
	@echo $(SRCS)
	@echo $(OBJS)
	@echo $(TEST_SRCS)
	@echo $(TEST_OBJS)

makefs: clean $(MAKEFS_OBJS)
	@echo "building $@..."
	$(CC) $(CFLAGS) $(MAKEFS_OBJS) -o makeFileSystem
#./makeFileSystem 1 1kb-fs

operfs: clean $(OPERFS_OBJS)
	@echo "building $@..."
	$(CC) $(CFLAGS) $(OPERFS_OBJS) -o fileSystemOper
#./fileSystemOper 1kb-fs mkdir "/first_dir"
#./fileSystemOper 1kb-fs mkdir /some/new_file1
#./fileSystemOper 1kb-fs mkdir /some/new_dir/new_file2

clean_fs:
	rm -rf $(MAKEFS_O) makeFileSystem 1kb-fs

clean:
	rm -rf bin/*

oper:
	@echo "building $@..."

all: makefs operfs

$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "building $@, $<, $@..."
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@
