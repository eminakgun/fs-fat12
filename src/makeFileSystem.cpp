#ifndef MAKEFS_CPP
#define MAKEFS_CPP


#include "fat12.hpp"
using fat12::fat12_fs;

int main(int argc, char const *argv[])
{
    // Check if the number of arguments is correct
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <integer> <string>" << std::endl;
        return 1;
    }

    // Check if the first argument is an integer
    char* end;
    long size = std::strtol(argv[1], &end, 10);
    if (*end != '\0') {
        std::cerr << "The first argument must be an integer." << std::endl;
        return 1;
    }

    // Get the second argument as a string
    std::string fs_name = argv[2];

    fat12_fs fs(fs_name);

    // use args
    fs.create_fs(size);
    //fat12::create_fs(4, "myfat12");
}


#endif