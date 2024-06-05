#ifndef FAT12_FS_HPP
#define FAT12_FS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

#include "fat12_data_types.hpp"
using std::string;

using fat12::BootSector;

namespace fat12 {

    class fat12_fs {
    private:
        string name;
        uint16_t block_size_byte;
        uint16_t number_of_blocks;
        int total_size_bytes;
        int total_size_kb;
        int fat_size_bytes;
        int entry_cnt_in_block;
        
        // Start addresses
        int fat1_start;
        int fat2_start;
        int root_dir_start;
        int data_area_start;

        // pointers to clusters
        char* fs_buffer;
        BootSector* boot_sector;
        DirectoryEntry* root;
        FatEntry* FAT;
        uint8_t* data_area;

        // Main file system operations
        void format(char* buffer);
        void traverse(DirectoryEntry* entry);

        // Directory operations
        void create_dir(DirectoryEntry* empty, DirectoryEntry* parent, string& dir_name);
        DirectoryEntry* find_dir(DirectoryEntry* current, string& dir_name);
        DirectoryEntry* find_empty_dir(DirectoryEntry* current);
        void initialize_new_dir(uint16_t cluster_num, DirectoryEntry* current, DirectoryEntry* parent);

        // utilities
        uint16_t reserve_cluster();
        int get_entry_cnt(DirectoryEntry* dir);
        bool is_in_root(DirectoryEntry* dir);
        void dump_fs();




    public:
        fat12_fs(string name):name(name){};
        ~fat12_fs(){ dump_fs(); delete[] fs_buffer;};
        void create_fs(int size_kb);
        void read_fs();
        // void traverse();
        void operate(const string& operation, const string& param);

        // commands
        void mkdir(const string& path);

        // Function to read a FAT entry
        FatEntry read_fat_entry(uint16_t cluster);
        void write_fat_entry(uint16_t cluster, FatEntry value);
    };

}//namespace

#endif