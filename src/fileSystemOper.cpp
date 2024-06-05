#ifndef FSOPER_CPP
#define FSOPER_CPP

#include <iostream>
#include <fstream>
#include <cstring>  // For std::memcpy, std::memset
#include <cstdint>  // For fixed-width integer types
#include <string>
#include <vector>
#include <sstream>

#include "fat12.hpp"
using fat12::fat12_fs;


// fileSystemOper fileSystem.data operation parameters
int main(int argc, char* argv[]) {
    bool operate = false;
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " fileSystem.data operation [parameters]" << std::endl;
    }
    else operate = true;

    std::string file_system_path = argv[1];
    fat12_fs fs(file_system_path);

    fs.read_fs();
    if (operate)
    {
        std::string operation = argv[2];
        fs.operate(operation, argv[3]);
    }
    
    //fs.traverse();

    return 0;
}



#endif