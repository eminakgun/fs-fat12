CC     = g++
INCDIR = include
CFLAGS = -std=c++11

SRCDIR = src
TESTDIR = test

BINDIR = bin
//SRCS = $(wildcard $(SRCDIR)/*.cpp)
SRCS = $(filter-out $(SRCDIR)/main.cpp, $(wildcard $(SRCDIR)/*.cpp))

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

$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "building $@, $<..."
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

makefs: clean $(MAKEFS_OBJS)
	@echo "building $@..."
	$(CC) $(CFLAGS) -I$(INCDIR) -DMAKEFILESYSTEM -c $(SRCDIR)/main.cpp -o $(BINDIR)/main.o
	$(CC) $(CFLAGS) $(OBJS) $(BINDIR)/main.o -o makeFileSystem

operfs: clean $(OPERFS_OBJS)
	@echo "building $@..."
	$(CC) $(CFLAGS) -I$(INCDIR) -DFILESYSTEMOPER -c $(SRCDIR)/main.cpp -o $(BINDIR)/main.o
	$(CC) $(CFLAGS) $(OBJS) $(BINDIR)/main.o -o fileSystemOper

test:
	$(CC) $(CFLAGS) -I$(INCDIR) $(SRCDIR)/main.cpp -o test

main: clean $(OBJS) makefs operfs test
	@echo "Build completed."

doc:
	pandoc README.md -o doc/converted.docx

all: main

clean_fs:
	rm -rf $(MAKEFS_O) makeFileSystem 1kb-fs

clean:
	rm -rf bin/*
