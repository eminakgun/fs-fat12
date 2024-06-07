
#include "fat12.hpp"
#include "fat12_utils.hpp"

namespace fat12 {

std::ostream& operator<<(std::ostream& os, const BootSector& boot_sector) {
    os << "===================Boot Sector===============\n";
    os << "OEM Name: " << std::string(boot_sector.BS_OEMName, 8) << '\n';
    os << "Bytes per Sector: " << boot_sector.BPB_BytsPerSec << '\n';
    os << "Sectors per Cluster: " << static_cast<int>(boot_sector.BPB_SecPerClus) << '\n';
    os << "Reserved Sectors: " << boot_sector.BPB_RsvdSecCnt << '\n';
    os << "Number of FATs: " << static_cast<int>(boot_sector.BPB_NumFATs) << '\n';
    os << "Root Entry Count: " << boot_sector.BPB_RootEntCnt << '\n';
    os << "Total Sectors (16-bit): " << boot_sector.BPB_TotSec16 << '\n';
    os << "Media Type: " << static_cast<int>(boot_sector.BPB_Media) << '\n';
    os << "Sectors per FAT (16-bit): " << boot_sector.BPB_FATSz16 << '\n';
    os << "Sectors per Track: " << boot_sector.BPB_SecPerTrk << '\n';
    os << "Number of Heads: " << boot_sector.BPB_NumHeads << '\n';
    os << "Hidden Sectors: " << boot_sector.BPB_HiddSec << '\n';
    os << "Total Sectors (32-bit): " << boot_sector.BPB_TotSec32 << '\n';
    os << "=============================================";
    return os;
}

// Overload the << operator for DirectoryEntry struct
std::ostream& operator<<(std::ostream& os, const DirectoryEntry& entry) {
    os << "===================DirectoryEntry===============\n";
    os << "Filename: " << entry.filename << std::endl;
    os << "Extension: " << entry.extension << std::endl;

    os << "Attributes: ";
    if (entry.attributes & ATTR_READ_ONLY) os << "Read Only ";
    if (entry.attributes & ATTR_HIDDEN) os << "Hidden ";
    if (entry.attributes & ATTR_SYSTEM) os << "System ";
    if (entry.attributes & ATTR_VOLUME_ID) os << "Volume ID ";
    if (entry.attributes & ATTR_DIRECTORY) os << "Directory ";
    if (entry.attributes & ATTR_ARCHIVE) os << "Archive ";
    os << std::endl;

    os << "Reserved: ";
    for (int i = 0; i < 10; ++i) {
        os << static_cast<int>(entry.reserved[i]) << " ";
    }
    os << std::endl;
    os << "Time: " << entry.time << std::endl;
    os << "Date: " << entry.date << std::endl;
    os << "Starting Cluster: " << entry.starting_cluster << std::endl;
    os << "File Size: " << entry.file_size << std::endl;
    return os;
}

void fat12_fs::dump_fs() {
    std::cout << "DUMP FILESYSTEM! size: " << total_size_bytes << std::endl;
    std::ofstream ofs(name , std::ios::out | std::ios::binary);
    if (!ofs.is_open()) {
        throw std::invalid_argument("Error opening input file: " + name);
    }

    ofs.write(fs_buffer, total_size_bytes); 
    ofs.close();
}

// this one uses current OS's api to create a file with an empty fat12 FS
void fat12_fs::create_fs(int size_kb) {

    // Check if size_kb is either 0.5 or 1
    if (size_kb != 0.5 && size_kb != 1) {
        throw std::invalid_argument("The block size must be either 0.5 KB or 1 KB.");
    }
    
    // Open a file for output operation and in binary mode
    std::ofstream ofs(name , std::ios::out | std::ios::binary);
    if (!ofs.is_open()) {
        throw std::invalid_argument("Error opening input file: " + name);
    }

    // Formatting options
    this->block_size_byte = size_kb * 1024 * DEFAULT_SECPERCLUS; // cluster is synonym of block in FAT
    this->total_size_bytes = block_size_byte * (1 << 12);
    this->total_size_kb = this->total_size_bytes / 1024;
    this->number_of_blocks =  total_size_bytes / block_size_byte;

    // Create a file system buffer initialized with zeros
    fs_buffer = new char[total_size_bytes];
    format(fs_buffer);
    ofs.write(fs_buffer, total_size_bytes); 
    ofs.close();

    std::cout << "Created file system: " << name << " with a size of " << total_size_kb << "KB" << std::endl;
    std::cout << "Number of Blocks: " << number_of_blocks << std::endl;
    std::cout << "Block Size (Bytes): " << block_size_byte << std::endl;
    std::cout << "Total Size (Bytes): " << total_size_bytes << std::endl;
    std::cout << "Total Size (KB): " << total_size_kb << std::endl;
}

void fat12_fs::format(char* buffer) {
    BootSector boot_sector = {
        {0x00, 0x00, 0x00},
        {'G', 'T', 'U', 'F', 'A', 'T', '1', '2'},
        block_size_byte,        // BPB_BytsPerSec, standard in fat12
        DEFAULT_SECPERCLUS,     // BPB_SecPerClus, cluster size(aka block size) is 1 (*BytsPerSec= 512 bytes), 
        DEFAULT_RSVDSECCNT,     // BPB_RsvdSecCnt
        DEFAULT_NUMFATS,        // BPB_NumFATs
        DEFAULT_ROOTENTCNT,     // BPB_RootEntCnt
        this->number_of_blocks, // BPB_TotSec16
        MEDIA_NONREMOVABLE,        // BPB_Media
        DEFAULT_FAT12_SIZE,     // BPB_FATSz16
        // since its a floppy disk beloe are all zeros 
        0,      // BPB_SecPerTrk
        0,      // BPB_NumHeads
        0,      // BPB_HiddSec
        0       // BPB_TotSec32
    };

    // Allocate Reserved Sector
    std::memcpy(buffer, &boot_sector, sizeof(BootSector));

    // Allocate FAT tables
    // Add offset to reach next region: FAT Tables
    // Calculate the start of the FAT tables
    size_t fat1_start = DEFAULT_RSVDSECCNT * DEFAULT_BYTSPERSEC;
    size_t fat2_start = fat1_start + (DEFAULT_FAT12_SIZE * DEFAULT_BYTSPERSEC);

    // Initialize FAT[0] and FAT[1]
    // First entry (media type) and second entry (EOC)
    // are reseved
    FatEntry fat[3] = { MEDIA_NONREMOVABLE, 0x0FF, 0x0F0 };
    std::memcpy(buffer + fat1_start, fat, 3);
    std::memset(buffer + fat1_start + 4, FAT_ENTRY_UNUSED,  (boot_sector.BPB_FATSz16 * boot_sector.BPB_BytsPerSec) - 4);
    
    std::memcpy(buffer + fat2_start, fat, 3);
    std::memset(buffer + fat2_start + 4, FAT_ENTRY_UNUSED,  (boot_sector.BPB_FATSz16 * boot_sector.BPB_BytsPerSec) - 4);

    // Calculate the start of the root directory
    size_t root_dir_start = fat2_start + (DEFAULT_FAT12_SIZE * DEFAULT_BYTSPERSEC);

    // Initialize the root directory with empty entries (0x00)
    // directory entries are 32 bytes
    std::memset(buffer + root_dir_start, 0, DEFAULT_ROOTENTCNT * 32);

    // Calculate the start of the data area
    size_t data_area_start = root_dir_start + (DEFAULT_ROOTENTCNT * 32);

    std::cout << "Data Area start at byte: " << data_area_start << std::endl;
    std::cout << "Data Area Size: " << (total_size_bytes - data_area_start) / 1024 << "KB" << std::endl;

    // Initialize the data area (optional, here we zero it out)
    std::memset(buffer + data_area_start, 0, total_size_bytes - data_area_start);
}

void fat12_fs::read_fs() {
    std::ifstream fs(name, std::ios::in | std::ios::binary | std::ios::ate);
    if (!fs.is_open()) {
        throw std::invalid_argument("Error opening input file: " + name);
    }

    // Determine the file size
    std::streamsize file_size = fs.tellg();
    std::cout << "File size: " << file_size / 1024 << "KB" << std::endl;
    
    // move pointer to beginning
    fs.seekg(0, std::ios::beg);
    fs_buffer = new char[file_size];
    this->total_size_bytes = file_size;
    std::cout << "char buffer of size: " << file_size << std::endl;

    // Read the entire file system image into the buffer
    if (!fs.read(fs_buffer, file_size)) {
        std::cerr << "Failed to read file: "<< name << std::endl;
        fs.close();
        delete[] fs_buffer;
        return;
    }
    fs.close();

    boot_sector = (BootSector*)fs_buffer; // reserved sector stars with superblock
    std::cout << *boot_sector << std::endl;

    block_size_byte = boot_sector->BPB_BytsPerSec * boot_sector->BPB_SecPerClus;
    fat_size_bytes = boot_sector->BPB_FATSz16 * boot_sector->BPB_BytsPerSec;
    entry_cnt_in_block = (unsigned long)block_size_byte / sizeof(DirectoryEntry);

    std::cout << "block_size_byte: " << block_size_byte << std::endl;
    std::cout << "fat_size_bytes: " << fat_size_bytes << std::endl;
    std::cout << "entry_cnt_in_block: " << entry_cnt_in_block << std::endl;
    std::cout << "sizeof(DirectoryEntry): " << sizeof(DirectoryEntry) << std::endl;

    // Calculate the starting addresses
    fat1_start = fat_size_bytes;
    fat2_start = fat1_start + fat_size_bytes;

    root_dir_start = fat2_start + fat_size_bytes;
    data_area_start = root_dir_start + (boot_sector->BPB_RootEntCnt * 32);

    // Print the calculated addresses
    std::cout << "FAT1 Start: " << fat1_start << std::endl;
    std::cout << "FAT2 Start: " << fat2_start << std::endl;
    std::cout << "Root Directory Start: " << root_dir_start << std::endl;
    std::cout << "Data Area Start: " << data_area_start << std::endl;

    // - Parse FAT tables
    this->FAT = reinterpret_cast<FatEntry*>(&fs_buffer[fat1_start]);
    this->data_area = reinterpret_cast<uint8_t*>(&fs_buffer[data_area_start]);

    // Parse root directory entries
    this->root = reinterpret_cast<DirectoryEntry*>(&fs_buffer[root_dir_start]);
    for (int i = 0; i < boot_sector->BPB_RootEntCnt; ++i) {
        if (!is_directory_free(root[i]))
        {
            std::cout << i << "th Directory:\n" << root[i] << std::endl;
            traverse(&root[i]);
        }
    }

    // - Access data area clusters
}

void fat12_fs::operate(const string& operation, const string& param) {
    if (operation == "mkdir") {
        mkdir(param);
    }
}

FatEntry fat12_fs::read_fat_entry(uint16_t cluster) {
    // Calculate the byte offset in the FAT
    uint32_t fat_offset = cluster + (cluster / 2);

    // Read the FAT entry (2 bytes)
    FatEntry entry = (FAT[fat_offset] | (FAT[fat_offset + 1] << 8));

    // Mask the high nibble if cluster number is odd
    if (cluster & 1) {
        entry >>= 4;
    } else {
        entry &= 0xFFF;
    }

    return entry;
}

// Function to write a FAT entry
void fat12_fs::write_fat_entry(uint16_t cluster, FatEntry value) {
    // Calculate the byte offset in the FAT
    uint32_t fat_offset = cluster + (cluster / 2);

    // Mask the high nibble if cluster number is odd
    if (cluster & 1) {
        FAT[fat_offset] = (FAT[fat_offset] & 0xF0) | (value >> 8);
        FAT[fat_offset + 1] = value & 0xFF;
    } else {
        FAT[fat_offset] = value & 0xFF;
        FAT[fat_offset + 1] = (FAT[fat_offset + 1] & 0x0F) | ((value & 0xF00) >> 4);
    }
}


void fat12_fs::mkdir(const string& path) {
    std::vector<string> tokens;
    size_t start = 0, end;
    // Tokenize the path using '/'
    std::cout << path << std::endl;
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
    tokens.push_back(path.substr(start)); // Add the last token
    std::cout << path.substr(start) << std::endl;
    // end tokenize

    
    bool duplicate = false;
    DirectoryEntry* empty_dir = nullptr;
    string dir_name = dir_name;
    // Trying to mkdir in root
    if (tokens.size() == 1)
    {
        auto dir_name = tokens[0];
        std::cout << "Attempt to create directory " << dir_name << " in Root: " << root << std::endl;
        for (int i = 0; i < boot_sector->BPB_RootEntCnt; ++i) {
            //std::cout << "Loop: " << i << std::endl;
            if (!is_directory_free(root[i]) && is_directory(root[i])){
                 //std::cout << "Found existing directory!" << std::endl;
                if (root[i].filename == dir_name) {
                    std::cout << "Found duplicate directory!" << std::endl;
                    duplicate = true;
                    break;
                }
            }
            else {
                empty_dir = &root[i];
            }
        }

        if (empty_dir != nullptr) {
            create_dir(empty_dir, root, dir_name);
        }
    }
    else {
        DirectoryEntry* next_dir = root;
        for (auto& token: tokens) {
            dir_name = token;
            if (token != tokens[tokens.size() - 1])
            {
                std::cout << "Looking for: " << token << " in: " << next_dir->filename << std::endl;
                auto found_dir = find_dir(next_dir, token);
                if (found_dir == nullptr)
                    throw std::invalid_argument("Invalid folder path: " + token);
                else {
                    std::cout << "Found dir: " << found_dir->filename << std::endl;
                    next_dir = found_dir;
                }
            }
        }

        empty_dir = find_empty_dir(next_dir);
        if (empty_dir != nullptr)
            create_dir(empty_dir, next_dir, dir_name);
    }
}

bool fat12_fs::is_in_root(DirectoryEntry* dir) {
    char* ptr = reinterpret_cast<char*>(dir);
    return (ptr >= &fs_buffer[root_dir_start]) && 
                (ptr < &fs_buffer[data_area_start]);
}

int fat12_fs::get_entry_cnt(DirectoryEntry* dir) {
    return is_in_root(dir) ? boot_sector->BPB_RootEntCnt 
                           : entry_cnt_in_block;
}

DirectoryEntry* fat12_fs::find_dir(DirectoryEntry* current, string& dir_name){
    int entry_cnt = is_in_root(current) ? boot_sector->BPB_RootEntCnt 
                                        : entry_cnt_in_block;
    for (int i = 0; i < entry_cnt; ++i) {
        if (is_directory(current[i])) {
            if (!is_directory_free(current[i]) ) {
                if (current[i].filename == dir_name) {
                    return &current[i];
                }
            }
        }
    }

    std::cout << "Can't find given directory: " << dir_name << std::endl;
    return nullptr;
}

DirectoryEntry* fat12_fs::find_empty_dir(DirectoryEntry* current){
    int entry_cnt = is_in_root(current) ? boot_sector->BPB_RootEntCnt 
                                        : entry_cnt_in_block;

    if (is_in_root(current)) {
        std::cout << current->filename << " is inside root directory" << std::endl;
    }
    
    for (int i = 0; i < entry_cnt; ++i) {
        if (is_directory(current[i])) {
            std::cout << "Check directory: " << current[i].filename << std::endl;
            auto fat_idx = current[i].starting_cluster;
            auto cluster_num = fat_idx;
            check_fat_idx(fat_idx);
            
            FatEntry fat_entry;
            do {
            auto cluster_start = cluster_num * block_size_byte;
            auto cluster = reinterpret_cast<DirectoryEntry*>(&data_area[cluster_start]);

                std::cout << "Searching for cluster_num: " << cluster_num << std::endl;
                std::cout << "              cluster_start: " << cluster_start << std::endl;
                for (int i = 0; i < entry_cnt_in_block; ++i) {
                    auto cluster = &data_area[cluster_start + i];
                    auto current_dir = reinterpret_cast<DirectoryEntry*>(cluster);
                    if (is_directory(*current_dir) && is_directory_free(*current_dir))
                        return current_dir;
                }

                fat_entry = FAT[fat_idx]; 
                cluster_num = fat_entry;
            } while (!is_last_cluster(fat_entry));
            std::cout << "End search empty cluster" << std::endl; 
        }
    }
    std::cout << "There's no free directories under: " << current->filename << std::endl;
    return nullptr;
}


// TODO
// Traverse through whole file system
void fat12_fs::traverse_all(){
    std::cout << "traverse_all!!!!" << std::endl;
    this->root = reinterpret_cast<DirectoryEntry*>(&fs_buffer[root_dir_start]);
    for (int i = 0; i < boot_sector->BPB_RootEntCnt; ++i) {
        if (!is_directory_free(root[i]))
        {
            std::cout << i << "th Directory:\n" << root[i] << std::endl;
            traverse(&root[i]);
        }
    }
}


void fat12_fs::traverse(DirectoryEntry* entry){

    if (!is_directory(*entry))
        return;

    std::cout << "Check directory: " << entry->filename << std::endl;
    auto fat_idx = entry->starting_cluster;
    auto cluster_num = fat_idx;
    check_fat_idx(fat_idx);

    FatEntry fat_entry;
    do {
        auto cluster_start = cluster_num * block_size_byte;
        auto cluster = reinterpret_cast<DirectoryEntry*>(&data_area[cluster_start]);

        std::cout << "Searching for cluster_num: " << cluster_num << std::endl;
        std::cout << "              cluster_start: " << cluster_start << std::endl;
        std::cout << "              fat_idx: "       << fat_idx << std::endl;
        for (int i = 0; i < entry_cnt_in_block; ++i) {
            auto current_dir = cluster[i];
            std::cout << "Found directory:\n" << current_dir << std::endl;
            if (is_directory(current_dir)) {
                ;
            }
        }

        fat_entry = FAT[fat_idx]; 
        cluster_num = fat_entry;
    } while (!is_last_cluster(fat_entry));
}

void fat12_fs::create_dir(DirectoryEntry* empty, DirectoryEntry* parent, string& dir_name) {
    std::cout << "Attemp to create a directory: " << dir_name 
              << ", Under parent directory: " << parent->filename << std::endl;

    DirectoryEntry entry;
    std::memset(&entry, 0, sizeof(DirectoryEntry)); // Initialize with zeros

    // Copy the filename to the directory entry
    std::strncpy(entry.filename, dir_name.c_str(), sizeof(entry.filename));

    // Set directory attributes (e.g., hidden, read-only, directory)
    entry.attributes = ATTR_DIRECTORY; // Assuming directory attribute
    set_time_date(entry);
    entry.file_size = 0;

    // Find a free cluster for the new directory (using FAT)
    uint16_t new_cluster = reserve_cluster();
    std::cout << "Reserved a new cluster" << std::endl;
    entry.starting_cluster = new_cluster;
    
    // Write the directory entry to the root directory buffer (or the parent directory buffer)
    std::memcpy(empty, &entry, sizeof(DirectoryEntry));

    initialize_new_dir(new_cluster, empty, parent);
}

void fat12_fs::initialize_new_dir(uint16_t cluster_num, DirectoryEntry* current, DirectoryEntry* parent) {
    std::cout << "initialize_new_dir at cluser: " << cluster_num << std::endl;
    std::cout << "current entry: " << *current << std::endl;
    std::cout << "parent entry: " << *parent << std::endl;

    // Set all bytes in the cluster to zero
    size_t cluster_start = cluster_num * block_size_byte;
    std::cout << "cluster_start: " << cluster_start << std::endl;

    std::memset(&data_area[cluster_start], 0, block_size_byte);

    // initialize a directory entry as "." as current directory
    DirectoryEntry dot_entry;
    std::memcpy(&dot_entry, current, sizeof(DirectoryEntry));
    std::strncpy(dot_entry.filename, ".          ", 11); // Name padded to 11 characters

    // initialize a directory entry as ".." as parent directory
    DirectoryEntry dotdot_entry;
    std::memcpy(&dotdot_entry, parent, sizeof(DirectoryEntry));
    std::strncpy(dotdot_entry.filename, "..         ", 11); // Name padded to 11 characters
    if (parent == root)
    {
        dotdot_entry.starting_cluster = 0; // root
    }

    // write dot and dotdot as first 2 directories for the directory to be initialized
    std::cout << "Dot entry: " << dot_entry << std::endl;
    std::cout << "Dotdot entry: " << dotdot_entry << std::endl;
    std::memcpy(&data_area[cluster_start], &dot_entry, sizeof(DirectoryEntry));
    std::memcpy(&data_area[cluster_start + sizeof(DirectoryEntry)], &dotdot_entry, sizeof(DirectoryEntry));
}

// Function to find a free cluster in the FAT
uint16_t fat12_fs::reserve_cluster() {
    // Iterate through the FAT entries to find a free cluster
    for (int i = FAT_RESERVED_CNT; i < fat_size_bytes/sizeof(FatEntry); ++i) {
        if (FAT[i] == FAT_ENTRY_UNUSED)
        {
            FAT[i] = EOC_MARKER;
            return i;
        }
    }
    return -1;
}

} // namespace