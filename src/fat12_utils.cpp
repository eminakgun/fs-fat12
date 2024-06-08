
#include "fat12_utils.hpp"
#include <ctime>
#include <stdexcept>
#include <iostream>
#include <fstream>

namespace fat12 {
    
    bool is_directory(const DirectoryEntry& entry) {
        return entry.attributes & ATTR_DIRECTORY;
    }

    bool is_directory_free(const DirectoryEntry& entry) {
        // Check if the first byte of the filename matches either DIR_NAME_FREE value
        return is_directory(entry) && is_entry_free(entry);
    }

    bool is_entry_free(const DirectoryEntry& entry) {
        return entry.filename[0] == DIR_NAME_FREE[0] || entry.filename[0] == DIR_NAME_FREE[1];
    }

    bool is_file(const DirectoryEntry& entry) {
        return !(entry.attributes & ATTR_DIRECTORY);
    }

    bool is_writable(const DirectoryEntry& entry) {
        return entry.attributes & ATTR_WRITABLE;
    }
    
    bool is_readable(const DirectoryEntry& entry) {
        return entry.attributes & ATTR_READABLE;
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
    void set_time_date(Timestamp* ts) {
        // Get current time
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);

        // Set time field
        ts->time = ((now->tm_hour & 0x1F) << 11) |
            ((now->tm_min & 0x3F) << 5) |
            ((now->tm_sec / 2) & 0x1F);

        // Set date field
        ts->date = (((now->tm_year - 80) & 0x7F) << 9) |
            (((now->tm_mon + 1) & 0xF) << 5) |
            (now->tm_mday & 0x1F);
    }

    void get_time_date(const Timestamp* ts, std::tm* decoded_time) {
        // Extract time components from ts->time
        decoded_time->tm_hour = (ts->time >> 11) & 0x1F;
        decoded_time->tm_min = (ts->time >> 5) & 0x3F;
        decoded_time->tm_sec = (ts->time & 0x1F) * 2;

        // Extract date components from ts->date
        decoded_time->tm_year = ((ts->date >> 9) & 0x7F) + 80; // Since tm_year is years since 1900
        decoded_time->tm_mon = ((ts->date >> 5) & 0xF) - 1; // Months since January (0-11)
        decoded_time->tm_mday = ts->date & 0x1F;

        // Set other tm fields to default values
        decoded_time->tm_isdst = -1; // Daylight saving time information is not available
        decoded_time->tm_yday = 0; // Not required for basic time/date extraction
        decoded_time->tm_wday = 0; // Not required for basic time/date extraction
    }

     std::vector<string> tokenize(const string& path) {
        size_t start = 0, end;

        std::vector<string> tokens;

        /// Tokenize the path using '/'
        if (path[0] != '/')
        {
            throw std::invalid_argument("Relative paths are not supported!");
        }

        std::cout << "tokens: ";
        while ((end = path.find('/', start)) != string::npos) {
            auto str = path.substr(start, end - start);
            if (str != "")
            {
                tokens.push_back(str);
                std::cout << str << ", ";
            }
            start = end + 1;
        }
        if (path.size() != 0)
        {
            tokens.push_back(path.substr(start)); // Add the last token
        }
        
        std::cout << path.substr(start) << std::endl;
        // end tokenize
        std::cout << "end tokenize" << std::endl;
        std::cout << "token size: " << tokens.size() << std::endl;
        return tokens;
     }

    string read_linux_file(const string& file_path) {
        std::ifstream src_file(file_path);
        if (!src_file.is_open()) {
            throw std::invalid_argument("Error opening input file: " + file_path);
        }

        // read _src file into a string
        std::string content((std::istreambuf_iterator<char>(src_file)),
                                std::istreambuf_iterator<char>());
        src_file.close(); // Close the file
        return content;
    }


    uint8_t read_linux_permissions(const string& file_path) {
        uint8_t attributes = 0;
        struct stat fileStat;
        if(stat(file_path.c_str(), &fileStat) < 0) {
            
        }
        else {
            if (fileStat.st_mode & S_IRUSR) {
                attributes += ATTR_READABLE;
            }
            if (fileStat.st_mode & S_IWUSR) {
                attributes += ATTR_WRITABLE;
            }
        }

        return attributes; 
    }

}//namespace
