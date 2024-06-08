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
    
    fat12_fs* fs = new fat12_fs("mert_fs");
    fs->create_fs(1);

    fs->read_fs();
    fs->operate("mkdir", "/newdir");
    fs->traverse_all();
    fs->operate("mkdir", "/newdir/subdir");
    fs->traverse_all();
    delete fs;
    

    //fs = new fat12_fs("mert_fs");
    //fs->read_fs();

  

    //fs.traverse_all();
    //fs.dump_fs();
    //fs.read_fs();   
    //fs.traverse();

    return 0;
}



#endif