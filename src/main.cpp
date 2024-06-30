#include <iostream>
#include <fstream>
#include <cstring>  // For std::memcpy, std::memset
#include <cstdint>  // For fixed-width integer types
#include <string>
#include <vector>
#include <sstream>

#include "fat12.hpp"
using fat12::fat12_fs;

// prototypes
void test();
void makefilesystem(int argc, char* argv[]);
void filesystemoper(int argc, char* argv[]);


// fileSystemOper fileSystem.data operation parameters
int main(int argc, char* argv[]) {
    bool operate = false;

    #ifdef MAKEFILESYSTEM
        makefilesystem(argc, argv);
    #else
        #ifdef FILESYSTEMOPER
            filesystemoper(argc, argv);
        #else
            //test();
        #endif
    #endif

    return 0;
}

void test() {
    fat12_fs* fs = new fat12_fs("test_fs");
    fs->create_fs(1);

    fs->read_fs();
    fs->operate("mkdir", "/usr");
    fs->operate("mkdir", "/usr/ysa");
    fs->operate("mkdir", "/bin/ysa");
    fs->operate("write", "/usr/ysa/file1 test_file.data");
    //fs->operate("write", "/usr/file2 test_file.data");
    //fs->operate("write", "/file3 test_file.data");
    fs->traverse_all();

    //fs->operate("dir", "/");
    //fs->operate("dir", "/usr");
    //fs->operate("dir", "/usr/ysa");
    //fs->operate("write", "/usr/ysa/file1 test_file.txt");
    //fs->operate("read", "/usr/ysa/file1 read_file.txt");

    //fs->operate("chmod", "/usr/ysa/file1 -rw");
    //fs->operate("read", "/usr/ysa/file1 read_file.txt"); // fails due to permissions
    //fs->operate("chmod", "/usr/ysa/file1 +rw");
    //fs->operate("read", "/usr/ysa/file1 read_file.txt"); // succeeds
    //fs->operate("dumpe2fs", "");
    //fs->traverse_all();
    
    delete fs;
    
}

void makefilesystem(int argc, char* argv[]) {
    // Check if the number of arguments is correct
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <integer> <string>" << std::endl;
        return;
    }

    // Check if the first argument is an integer
    char* end;
    long size = std::strtol(argv[1], &end, 10);
    if (*end != '\0') {
        std::cerr << "The first argument must be an integer." << std::endl;
        return;
    }

    // Get the second argument as a string
    std::string fs_name = argv[2];

    fat12_fs fs(fs_name);
    // use args
    fs.create_fs(size);
}


void filesystemoper(int argc, char* argv[]) {
    bool operate = false;
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " fileSystem.data operation [parameters]" << std::endl;
    }
    else operate = true;

    std::string file_system_path = argv[1];
    fat12_fs fs(file_system_path);

    fs.read_fs();
    if (operate) {
        std::string operation = argv[2];
        fs.operate(operation, argv[3]);
    }
    fs.dump_fs();
}