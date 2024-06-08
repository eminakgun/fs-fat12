
#ifndef FAT12_UTILS_HPP
#define FAT12_UTILS_HPP

#include <sys/stat.h>
#include <ctime>
#include <string>
#include <vector>

#include "fat12_data_types.hpp"

using std::string;

namespace fat12 {
    
    bool is_directory(const DirectoryEntry& entry);
    bool is_directory_free(const DirectoryEntry& entry);
    bool is_entry_free(const DirectoryEntry& entry);
    bool is_file(const DirectoryEntry& entry);
    bool is_writable(const DirectoryEntry& entry);
    bool is_readable(const DirectoryEntry& entry);


    bool is_reserved_cluster(uint16_t cluster);
    bool is_last_cluster(uint16_t cluster);
    void check_fat_idx(uint16_t idx);
    void set_time_date(Timestamp* ts);
    void get_time_date(const Timestamp* ts, std::tm* decoded_time);
    std::vector<string> tokenize(const string& path);

    // linux stuff
    string read_linux_file(const string& file_path);
    uint8_t read_linux_permissions(const string& file_path);


}//namespace

#endif