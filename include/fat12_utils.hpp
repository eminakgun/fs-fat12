
#ifndef FAT12_UTILS_HPP
#define FAT12_UTILS_HPP

#include "fat12_data_types.hpp"
#include <ctime>

namespace fat12 {
    bool is_directory_free(const DirectoryEntry& entry) {
        // Check if the first byte of the filename matches either DIR_NAME_FREE value
        return entry.filename[0] == DIR_NAME_FREE[0] || entry.filename[0] == DIR_NAME_FREE[1];
    }

    bool is_directory(const DirectoryEntry& entry) {
        return entry.attributes & ATTR_DIRECTORY;
    }

    bool is_file(const DirectoryEntry& entry) {
        return !(entry.attributes & ATTR_DIRECTORY);
    }

    bool is_reserved_cluster(uint16_t cluster) {
        return (cluster >= FAT_ENTRY_RESERVED_CLUSTER_START && cluster <= FAT_ENTRY_RESERVED_CLUSTER_END);
    }

    bool is_last_cluster(uint16_t cluster) {
        return (cluster >= FAT_ENTRY_LAST_CLUSTER_START && cluster <= FAT_ENTRY_LAST_CLUSTER_END);
    }

    void check_fat_idx(uint16_t idx) {
        if (idx == 0 || idx == 1)
        {
            throw std::invalid_argument("FAT indexes 1 and 2 are reserved!");
        }
    }
    void set_time_date(DirectoryEntry* entry) {
        // Get current time
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);

        // Set time field
        entry->time = ((now->tm_hour & 0x1F) << 11) |
            ((now->tm_min & 0x3F) << 5) |
            ((now->tm_sec / 2) & 0x1F);

        // Set date field
        entry->date = (((now->tm_year - 80) & 0x7F) << 9) |
            (((now->tm_mon + 1) & 0xF) << 5) |
            (now->tm_mday & 0x1F);
    }

}//namespace

#endif