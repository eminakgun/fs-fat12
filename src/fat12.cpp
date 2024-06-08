
#include "fat12.hpp"
#include "fat12_utils.hpp"
#include <ctime>

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
        os << "Password: " << entry.password << std::endl;

        os << "Attributes: ";
        if (entry.attributes & ATTR_READABLE) os << "+R ";
        if (entry.attributes & ATTR_WRITABLE) os << "+W ";
        if (entry.attributes & ATTR_SYSTEM) os << "System ";
        if (entry.attributes & ATTR_VOLUME_ID) os << "Volume ID ";
        if (entry.attributes & ATTR_DIRECTORY) os << "Directory ";
        if (entry.attributes & ATTR_ARCHIVE) os << "Archive ";
        os << std::endl;

        std::tm tm_creation;
        std::tm tm_last_modified;
        
        get_time_date(&(entry.creation), &tm_creation);
        get_time_date(&(entry.last_modification), &tm_last_modified);

        // Print using strftime to format the date and time
        char buffer[100];
        // Format creation date, time
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm_creation);
        os << "Creation Date: " << buffer << std::endl;
        std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm_creation);
        std::cout << "Creation Time: " << buffer << std::endl;

        // Format last modified date, time
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm_last_modified);
        os << "Last Modificaiton Date: " << buffer << std::endl;
        std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm_last_modified);
        std::cout << "Last Modificaiton Time: " << buffer << std::endl;


        os << "Starting Cluster: " << entry.starting_cluster << std::endl;
        os << "File Size: " << entry.file_size << std::endl;
        return os;
    }

    void fat12_fs::dump_fs() {
        std::cout << "DUMP FILESYSTEM! size: " << total_size_bytes << std::endl;

        //traverse_all();

        std::ofstream ofs(name, std::ios::out | std::ios::binary);
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
        std::ofstream ofs(name, std::ios::out | std::ios::binary);
        if (!ofs.is_open()) {
            throw std::invalid_argument("Error opening input file: " + name);
        }

        // Formatting options
        this->block_size_byte = size_kb * 1024 * DEFAULT_SECPERCLUS; // cluster is synonym of block in FAT
        this->total_size_bytes = block_size_byte * (1 << 12);
        this->total_size_kb = this->total_size_bytes / 1024;
        this->number_of_blocks = total_size_bytes / block_size_byte;

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
        std::memcpy(buffer + fat1_start, fat, sizeof(FatEntry));
        std::memset(buffer + fat1_start + 4, FAT_ENTRY_UNUSED, (boot_sector.BPB_FATSz16 * boot_sector.BPB_BytsPerSec) - 4);

        std::memcpy(buffer + fat2_start, fat, sizeof(FatEntry));
        std::memset(buffer + fat2_start + 4, FAT_ENTRY_UNUSED, (boot_sector.BPB_FATSz16 * boot_sector.BPB_BytsPerSec) - 4);

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
            std::cerr << "Failed to read file: " << name << std::endl;
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
            if (!is_entry_free(root[i]))
            {
                std::cout << i << "th Directory:\n" << root[i] << std::endl;
                traverse(&root[i]);
            }
        }

        // - Access data area clusters
    }

    void fat12_fs::operate(const string& operation, const string& param) {
        std::cout << "Operating: " << operation << " " << param << std::endl;
        try {
            if ("mkdir" == operation) {
                mkdir(param);
            } 
            else if ("dir" == operation)
            {
                dir(param);
            }
            else if ("write" == operation)
            {
                write(param);
            }
            else if ("read" == operation)
            {
                read(param);
            }
            else if ("chmod" == operation)
            {
                chmod(param);
            }
            else if ("dumpe2fs" == operation)
            {
                dumpe2fs();
            }
            
            else {
                throw std::runtime_error("Unsupported operation: " + operation);
            }
        } catch (const std::exception& e) {
            std::cout << "Exception occurred: " << e.what() << std::endl;
        }
    }

    void fat12_fs::mkdir(const string& path) {
        std::cout << "Processing mkdir " << path << std::endl;
        auto tokens = tokenize(path);
        // Initialize variables
        bool duplicate = false;
        DirectoryEntry* target_dir = root;
        DirectoryEntry* empty_dir = nullptr;
        string dir_name;

        // Trying to mkdir in root
        if (tokens.size() == 1)
        {
            dir_name = tokens[0];
            std::cout << "Attempt to create directory " << dir_name << " in Root: " << root << std::endl;
            for (int i = 0; i < boot_sector->BPB_RootEntCnt; ++i) {
                //std::cout << "Loop: " << i << std::endl;
                if (!is_entry_free(root[i])) {
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
            std::cout << "Search:" << next_dir->filename << std::endl;
            for (auto& token : tokens) {
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
            target_dir = next_dir;
            //target_dir = find_dir_recursive(tokens);
        }
        std::cout << "Target dir: " << *target_dir << std::endl;

        if (target_dir != nullptr) {
            empty_dir = find_empty_dir(target_dir);
        }

        if (empty_dir != nullptr) {
            create_dir(empty_dir, target_dir, dir_name);
        }
        else {
            std::cerr << "Couldn't find an empty directory under" << target_dir->filename << std::endl;
        }
    }

    void fat12_fs::dir(const string& path) {
        // TODO Must also handle file names

        auto tokens = tokenize(path);

        DirectoryEntry* target_dir = root;
        string dir_name;

        if (tokens.size() == 1 && tokens[0] == "")
        {
            // list root directory
            for (int i = 0; i < boot_sector->BPB_RootEntCnt; ++i)
                if (is_directory(root[i]))
                    std::cout << root[i] << std::endl;
        }
        else {
            target_dir = find_dir_recursive(tokens);
            if (target_dir != nullptr) {
                auto it = iterator(target_dir);
                while (it->has_next()) {
                    auto dir = it->next();
                    if (!is_entry_free(*dir))
                        std::cout << *dir << std::endl;
                }
            }
        }
    }

    void fat12_fs::write(const string& path) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(path);

        // process path parame
        while (std::getline(iss, token, ' ')) {
            tokens.push_back(token);
        }

        if (tokens.size() != 2)
        {
            throw std::invalid_argument("Invalid arguments");
        }
        
        std::cout << "Processing: " << tokens[0] << " , " << tokens[1] << std::endl;
        auto target_path = tokens[0];

        string content = read_linux_file(tokens[1]);

        std::cout << "File content to be copied:\n" << content << std::endl;

        auto path_tokens = tokenize(target_path);
        string& fname = path_tokens[path_tokens.size()-1]; // last token
        path_tokens.pop_back(); // remove last token, i.e file name

        std::cout << "path_tokens size: " << path_tokens.size() << std::endl;
        for (size_t i = 0; i < path_tokens.size(); i++)
        {
            std::cout << "i: " << path_tokens[i] << std::endl;
        }
        
        DirectoryEntry* target_dir = root;
        if (path_tokens.size() > 0)
        {
            target_dir = find_dir_recursive(path_tokens);
        }
        std::cout << "Target dir: " << *target_dir << std::endl;

        if (target_dir != nullptr) {
            auto empty = find_empty_dir(target_dir); 
            if (empty != nullptr) {
                std::cout << "Empty: " << *empty << std::endl;
                create_file(empty, target_dir, fname);
                std::cout << "Updated Empty: " << *empty << std::endl;
                // Copy linux permission
                empty->attributes += read_linux_permissions(tokens[1]);
                
                // TODO we must check content size, and allocate depending on size
                write_file(empty, content);
            }   
        }
    }

    void fat12_fs::read(const string& path) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(path);

        // process path parame
        while (std::getline(iss, token, ' ')) {
            tokens.push_back(token);
        }

        if (tokens.size() != 2)
        {
            throw std::invalid_argument("Invalid arguments");
        }
        
        std::cout << "Processing: " << tokens[0] << " , " << tokens[1] << std::endl;
        auto fat_path = tokens[0];
        auto linux_file_path = tokens[1];

        auto path_tokens = tokenize(fat_path);
        string& fname = path_tokens[path_tokens.size()-1]; // last token
        path_tokens.pop_back(); // remove last token, i.e file name

        auto target_dir = find_dir_recursive(path_tokens);
        std::cout << "Target dir: " << *target_dir << std::endl;
        if (target_dir != nullptr) {
            auto it = iterator(target_dir);
            while (it->has_next())
            {
                auto entry = it->next();
                if (entry->filename == fname) {
                    if (!is_readable(*entry)) {
                        throw std::runtime_error("Target file does not have read permission!");
                    }
                    
                    std::cout << "Found a file to read!" << std::endl;
                    char* file_cluster_base = reinterpret_cast<char*>(&data_area[entry->starting_cluster]);
        
                    string file_content(file_cluster_base);
                    std::cout << "Read file content: " << file_content << std::endl;

                    std::cout << "Now write to a linux file!" << std::endl;
                    std::ofstream outfile(linux_file_path);

                    // Check if the file is opened successfully
                    if (!outfile.is_open()) {
                        throw std::invalid_argument("Error opening file: " + linux_file_path);
                    }

                    // Write the string to the file
                    outfile << file_content;

                    // Close the file
                    outfile.close();
                }
            }
        }
    }

    void fat12_fs::chmod(const string& path) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(path);

        // process path parame
        while (std::getline(iss, token, ' ')) {
            tokens.push_back(token);
        }

        if (tokens.size() != 2)
        {
            throw std::invalid_argument("Invalid number of arguments: " + path);
        }
        
        std::cout << "Processing: " << tokens[0] << " , " << tokens[1] << std::endl;
        auto fat_path = tokens[0];
        string permissions = tokens[1];

        if (permissions.size() < 2) {
            throw std::invalid_argument("Invalid permission argument: " + permissions);
        }

        auto path_tokens = tokenize(fat_path);
        string& fname = path_tokens[path_tokens.size()-1]; // last token
        path_tokens.pop_back(); // remove last token, i.e file name

        auto target_dir = find_dir_recursive(path_tokens);
        if (target_dir = nullptr) {
            std::cout << "Directory search failed! " << fat_path << std::endl;
            return;
        }
        
        std::cout << "Target dir: " << *target_dir << std::endl;
        if (target_dir != nullptr) {
            auto it = iterator(target_dir);
            while (it->has_next())
            {
                auto entry = it->next();
                if (entry->filename == fname) {
                    std::cout << "Found the file to change permissions!" << std::endl;

                    std::string permission = permissions.substr(0, 1);
                    if (permission == "+") {
                        std::cout << "The first character is +" << std::endl;
                        for (size_t i = 1; i < permissions.size(); ++i) {
                            if (permissions[i] == 'r') {
                                std::cout << "Read permission granted on file " << fname << std::endl;
                                entry->attributes |= ATTR_READABLE;
                            } else if (permissions[i] == 'w') {
                                std::cout << "Write permission granted on file " << fname << std::endl;
                                entry->attributes |= ATTR_WRITABLE;
                            }
                        }
                    } else if (permission == "-") {
                        std::cout << "The first character is -" << std::endl;
                        for (size_t i = 1; i < permissions.size(); ++i) {
                            if (permissions[i] == 'r') {
                                std::cout << "Read permission revoked on file " << fname << std::endl;
                                entry->attributes &= ~ATTR_READABLE;
                            } else if (permissions[i] == 'w') {
                                std::cout << "Write permission revoked on file " << fname << std::endl;
                                entry->attributes &= ~ATTR_WRITABLE;
                            }
                        }
                    }
                }
            }
        }
    }

    void fat12_fs::dumpe2fs() {
        // Print file system information
        std::cout << *boot_sector << std::endl;
        std::cout << "Block size:" << block_size_byte << " bytes" << std::endl;
        std::cout << "FAT1 Start: " << fat1_start << std::endl;
        std::cout << "FAT2 Start: " << fat2_start << std::endl;
        std::cout << "Root Directory Start: " << root_dir_start << std::endl;
        std::cout << "Data Area Start: " << data_area_start << std::endl;
        // TODO
        // list block count, free blocks,
        // number of files and directories.
        // list all the occupied blocks and the file names for each of them.
    }

    void fat12_fs::write_file(DirectoryEntry* file, string& content) {
        auto content_char = reinterpret_cast<char*>(&content);
        
        // write content
        char* char_ptr = reinterpret_cast<char*>(&data_area[file->starting_cluster]);
        strcpy(char_ptr, content.c_str()); 
        file->file_size = content.size();
    }
    

    //

    void fat12_fs::print_cluster(uint16_t cluster) {
/*         auto it = iterator(cluster);
        while (it->has_next())
        {
            auto ch = reinterpret_cast<char>(it->next_char());
            std::cout << ch;
        }
        std::cout << std::endl; */
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

    DirectoryEntry* fat12_fs::find_dir_recursive(std::vector<std::string> tokens) {
        DirectoryEntry* target_dir = root;
        string dir_name;
        bool found = false;

        for (auto& token : tokens) {
            dir_name = token;
            
            target_dir = find_dir(target_dir, token);
            if (target_dir == nullptr) {
                std::cout << "break search!"<< std::endl;
                std::cout << target_dir << std::endl;
                break;
            }
            else
                found = true;         
        }

        if (found == true && target_dir != nullptr) {
            std::cout << "Reached target directory:," << dir_name << std::endl;
            std::cout << *target_dir << std::endl;
        }

        return target_dir;
    }

    DirectoryEntry* fat12_fs::find_dir(DirectoryEntry* current, string& dir_name) {
        int entry_cnt = is_in_root(current) ? boot_sector->BPB_RootEntCnt
                                            : entry_cnt_in_block;
        std::cout << "Searching " << dir_name << " under folder: " << current->filename << std::endl;
        std::cout << *current << std::endl;

/*         auto it = iterator(current);
        while (it->has_next()) {
            auto next = it->next();
            if (!is_entry_free(*next)) {
                if (is_directory(*next)) {
                    if (next->filename == dir_name) {
                        return next;
                    }
                }
            }
        } */
        
        for (int i = 0; i < entry_cnt; ++i) {
            if (!is_entry_free(current[i])) {
                if (is_directory(current[i])) {
                    if (current[i].filename == dir_name) {
                        return &current[i];
                    }
                }
            }
        }

        std::cout << "Can't find given directory: " << dir_name << std::endl;
        return nullptr;
    }

    DirectoryEntry* fat12_fs::find_empty_dir(DirectoryEntry* current) {
        int entry_cnt = is_in_root(current) ? boot_sector->BPB_RootEntCnt
            : entry_cnt_in_block;

        if (is_in_root(current)) {
            std::cout << current->filename << " is inside root directory" << std::endl;
            for (int i = 0; i < boot_sector->BPB_RootEntCnt; ++i)
                if (is_entry_free(root[i]))
                    return &root[i];
        }
        else {
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
                            if (is_entry_free(cluster[i])) {
                                std::cout << "Found empty directory at index " << i << std::endl;
                                return &cluster[i];
                            }
                                
                        }

                        fat_entry = FAT[fat_idx];
                        cluster_num = fat_entry;
                    } while (!is_last_cluster(fat_entry));
                    std::cout << "End search empty cluster" << std::endl;
                }
            }
        }
        
        std::cout << "There's no free directories under: " << current->filename << std::endl;
        return nullptr;
    }


    // TODO
    // Traverse through whole file system
    void fat12_fs::traverse_all() {
        std::cout << "traverse_all!!!!" << std::endl;
        this->root = reinterpret_cast<DirectoryEntry*>(&fs_buffer[root_dir_start]);
        for (int i = 0; i < boot_sector->BPB_RootEntCnt; ++i) {
            if (!is_entry_free(root[i])) {
                std::cout << i << "th Directory under root:\n" << root[i] << std::endl;
                traverse(&root[i]);
            }
        }
    }


    void fat12_fs::traverse(DirectoryEntry* entry) {

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

/*             std::cout << "Searching for cluster_num: " << cluster_num << std::endl;
            std::cout << "              cluster_start: " << cluster_start << std::endl;
            std::cout << "              fat_idx: " << fat_idx << std::endl; */
            for (int i = 0; i < entry_cnt_in_block; ++i) {
                if (!is_entry_free(cluster[i])) {
                    std::cout << "Found directory:\n" << cluster[i] << std::endl;
                    if (i >= 2 && !is_in_root(&cluster[i])) {
                        traverse(&cluster[i]);
                    }
                }
            }

            fat_entry = FAT[fat_idx];
            cluster_num = fat_entry;
        } while (!is_last_cluster(fat_entry));
    }


    void fat12_fs::create_file(DirectoryEntry* empty, DirectoryEntry* parent, string file_name) {
        std::cout << "Attemp to create a file: " << file_name
                  << ", Under parent directory: " << parent->filename << std::endl;

        
        uint16_t new_cluster = reserve_cluster();
        std::cout << "Reserved a new cluster: " << new_cluster << std::endl;
        std::strncpy(empty->filename, file_name.c_str(), file_name.size());
        empty->file_size = 0;
        empty->starting_cluster = new_cluster;
        set_time_date(&(empty->creation));
        set_time_date(&(empty->last_modification));
        set_time_date(&(parent->last_modification));
        std::cout << "Created a file: " << file_name << "\n" << empty << std::endl;
    }

    void fat12_fs::create_dir(DirectoryEntry* empty, DirectoryEntry* parent, string& dir_name) {
        std::cout << "Attemp to create a directory: " << dir_name
            << ", Under parent directory: " << parent->filename << std::endl;

        uint16_t new_cluster = reserve_cluster();
        std::cout << "Reserved a new cluster: " << new_cluster << std::endl;

        std::strncpy(empty->filename, dir_name.c_str(), dir_name.size());
        empty->attributes = ATTR_DIRECTORY;
        empty->file_size = 0;
        empty->starting_cluster = new_cluster;
        set_time_date(&(empty->creation));
        set_time_date(&(empty->last_modification));
        set_time_date(&(parent->last_modification)); // update paren'ts last modification timestamp
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
        dot_entry = *current;
        //std::memcpy(&dot_entry, current, sizeof(DirectoryEntry));
        std::strncpy(dot_entry.filename, ".          ", 11); // Name padded to 11 characters

        // initialize a directory entry as ".." as parent directory
        DirectoryEntry dotdot_entry;
        //std::memcpy(&dotdot_entry, parent, sizeof(DirectoryEntry));
        dotdot_entry = *parent;
        std::strncpy(dotdot_entry.filename, "..         ", 11); // Name padded to 11 characters
        if (parent == root)
        {
            dotdot_entry.attributes = ATTR_DIRECTORY;
            dotdot_entry.starting_cluster = 0; // root
        }

        // write dot and dotdot as first 2 directories for the directory to be initialized
        std::cout << "Dot entry: " << dot_entry << std::endl;
        std::cout << "Dotdot entry: " << dotdot_entry << std::endl;
        auto cluster = reinterpret_cast<DirectoryEntry*>(&data_area[cluster_start]);
        cluster[0] = dot_entry;
        cluster[1] = dotdot_entry;
    }

    // Function to find a free cluster in the FAT
    uint16_t fat12_fs::reserve_cluster() {
        // Iterate through the FAT entries to find a free cluster
        for (int i = FAT_RESERVED_CNT; i < fat_size_bytes / sizeof(FatEntry); ++i) {
            if (FAT[i] == FAT_ENTRY_UNUSED)
            {
                FAT[i] = EOC_MARKER;
                return i;
            }
        }
        return -1;
    }

} // namespace