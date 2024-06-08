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
        
    public:
    int entry_cnt_in_block;

    
        fat12_fs(string name):name(name){};
        ~fat12_fs(){ dump_fs(); delete[] fs_buffer;};

        void traverse_all();
        void dump_fs();
        void create_fs(int size_kb);
        void read_fs();
        // void traverse();
        void operate(const string& operation, const string& param);

        // commands
        void mkdir(const string& path);

        // Function to read a FAT entry
        FatEntry read_fat_entry(uint16_t cluster);
        void write_fat_entry(uint16_t cluster, FatEntry value);

        friend class Fat12Iterator;
        class Fat12Iterator {
        private:
            fat12_fs* fs;
            DirectoryEntry* root_entry;
            DirectoryEntry* current_cluster;
            uint16_t current_cluster_num;
            int current_idx;
            int fat_idx;

            void load_cluster(uint16_t cluster) {
                auto cluster_start = cluster * fs->block_size_byte;
                current_cluster = reinterpret_cast<DirectoryEntry*>(&(fs->data_area[cluster_start]));
            }

            void advance_cluster() {
                current_cluster_num = fs->FAT[fat_idx];
                current_idx = 0;
                load_cluster(current_cluster_num);
            }

        public:
            Fat12Iterator(DirectoryEntry* entry, fat12_fs* fs) 
                : root_entry(entry), fs(fs), current_cluster_num(entry->starting_cluster), current_idx(0) {
                if (is_directory(*root_entry)) {
                    auto fat_idx = entry->starting_cluster;
                    auto cluster_num = fat_idx;
                    check_fat_idx(fat_idx);
                    load_cluster(current_cluster_num);
                }
            }

            bool has_next() {
                return (current_idx < (fs->entry_cnt_in_block))
                            || !is_last_cluster(fs->FAT[fat_idx]);
            }

            DirectoryEntry* next() {
                if (!is_directory(*root_entry)) {
                    return nullptr;
                }

                if (current_idx >= fs->entry_cnt_in_block) {
                    advance_cluster();
                }

                return &current_cluster[current_idx++];
            }
        };

        Fat12Iterator* iterator(DirectoryEntry* entry){
            return new Fat12Iterator(entry, this);
        }
    };

}//namespace

#endif