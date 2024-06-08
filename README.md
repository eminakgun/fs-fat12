
# fs-fat12

simple fat-12 implementation

## FAT

The File Allocation Table can be considered to be the "table of contents" of a disk.
The FAT12 is the file system on a floppy disk which typically had sizes ranging from 360 KB to 1.44 MB..
The number “12” is derived from the fact that the FAT consists of 12-bit entries.

The storage space on a floppy disk is divided into units called sectors.
In larger storage devices, a bunch of sectors form a cluster.

However, for the floppy disk, the number of sectors in a cluster is one.
Also, the size of a sector (and hence a cluster) is 512 bytes for a floppy disk.

## Definitions

Sector: A unit of data that can be accessed independently of other units on the media

Cluster: A unit of allocation comprising a set of logically contiguous sectors.
Each cluster within the volume is referred to by a cluster number “N”.
All allocation for a file must be an integral multiple of a cluster.

Partition: An extent of sectors within a volume.

Volume: A logically contiguous sector address space as specified in the relevant standard for
recording.

## Volume Structure

A FAT file system volume is composed of four basic regions, which are laid out in this order on
the volume:
0 – Reserved Region
1 – FAT Region
2 – Root Directory Region (doesn’t exist on FAT32 volumes)
3 – File and Directory Data Region

- On disk data structures for the FAT format are all “little endian.”

## Boot Sector and BPB

The BPB (BIOS Parameter Block) is located in the first sector of the volume in the Reserved
Region. This sector is sometimes called the “boot sector” or the “0th sector”. The important fact to
note is that this sector is simply the first sector of the volume.

### Root entry count calculation

Root directory is made up of 14 sectors,
32 bytes per directory
(14*512) / 32 = 224 entries

# FAT12 Project Report

## Directory Table and Directory Entries

### Directory Entries

To define our directory entries, we use the following structure:

```c
struct DirectoryEntry {
    char filename[8];              // 8 bytes: Filename
    char extension[3];             // 3 bytes: File extension
    char password[6];              // 6 bytes: Password for file protection (custom addition)
    uint8_t attributes;            // 1 byte: File attributes
    uint8_t reserved[2];           // 2 bytes: Reserved for future use
    Timestamp creation;            // 4 bytes: Timestamp of file creation
    Timestamp last_modification;   // 4 bytes: Timestamp of last modification
    uint16_t starting_cluster;     // 2 bytes: Starting cluster number
    uint32_t file_size;            // 4 bytes: Size of the file in bytes
};
```

### Fields Explanation

1. **Filename (8 bytes)**: This field stores the name of the file, padded with spaces if the name is shorter than 8 characters.
2. **Extension (3 bytes)**: This field holds the file extension, which identifies the file type (e.g., 'TXT' for text files).
3. **Password (6 bytes)**: A custom addition to enhance file protection, this field stores a 6-byte password. This is not standard in FAT12 and represents an enhancement for security purposes.
4. **Attributes (1 byte)**: This field indicates the file attributes, such as read-only, hidden, system, volume label, directory, archive, and password protection.

- Bit 0 (0x01): Read permission (ATTR_READABLE)
- Bit 1 (0x02): Write permission (ATTR_WRITABLE)
- Bit 2 (0x04): System (ATTR_SYSTEM)
- Bit 3 (0x08): Volume Label (ATTR_VOLUME_ID)
- Bit 4 (0x10): Subdirectory (ATTR_DIRECTORY)
- Bit 5 (0x20): Archive (ATTR_ARCHIVE)
- Bit 6 (0x40): Password Protected (ATTR_PASSWORD_PROTECTED)

5. **Reserved (2 bytes)**: Reserved space for future use, ensuring compatibility with potential extensions or additional attributes.
6. **Creation Timestamp (4 bytes)**: The timestamp of when the file was created. This typically includes the date and time.
7. **Last Modification Timestamp (4 bytes)**: The timestamp of the last modification to the file, providing a record of when the file was last edited.
8. **Starting Cluster (2 bytes)**: The starting cluster number where the file's data is stored on the disk. This links the directory entry to the actual file content.
9. **File Size (4 bytes)**: The size of the file in bytes, which helps in managing the file's storage and retrieval.

### Timestamp Structure

The `Timestamp` structure is used to store date and time information for the creation and last modification fields. A possible definition of the `Timestamp` structure might be:

```c
struct Timestamp {
    uint16_t date; // Date in the format: bits 0-4 day (1-31), bits 5-8 month (1-12), bits 9-15 year (relative to 1980)
    uint16_t time; // Time in the format: bits 0-4 second (0-29, representing 0-58 in 2-second intervals), bits 5-10 minute (0-59), bits 11-15 hour (0-23)
};
```

This format allows compact storage of date and time, with enough granularity to record precise modification and creation times for files.

### Example Directory Entry

Here is an example of how a directory entry might be populated:

```c
struct DirectoryEntry example_entry = {
    .filename = "example",
    .extension = "txt",
    .password = "123456",
    .attributes = 0x20, // Archive attribute set
    .reserved = {0, 0},
    .creation = {0x4A21, 0x7C00}, // Example creation timestamp
    .last_modification = {0x4A21, 0x7C00}, // Example modification timestamp
    .starting_cluster = 5,
    .file_size = 1024 // File size in bytes
};
```

In this example:
- The filename is "example" with a "txt" extension.
- The file has a password "123456".
- The attributes indicate it is an archive file.
- Timestamps are set to specific values for demonstration.
- The file starts at cluster 5 and has a size of 1024 bytes.

### Directory Table Structure

The directory table structure in a FAT12 file system is a key organizational component that ensures efficient management and access to files and directories. This structure is part of a larger system, comprising several essential components, each playing a critical role in the overall functionality of the file system. Below, we describe the key elements of a FAT12 file system: the Boot Sector (superblock), the File Allocation Tables (FAT x 2), the Root Directory, and the Data Area.

#### Boot Sector as Superblock

The Boot Sector, often referred to as the superblock in the context of FAT12, is the first sector of the file system. It contains vital information about the disk layout and file system parameters. The `BootSector` structure is defined as follows:

```c++
struct BootSector {
    char BS_jmpBoot[3];           // Jump instruction to boot code
    char BS_OEMName[8];           // OEM Name Identifier

    uint16_t BPB_BytsPerSec;      // Bytes per sector
    uint8_t BPB_SecPerClus;       // Sectors per allocation unit (cluster)
    uint16_t BPB_RsvdSecCnt;      // Number of reserved sectors
    uint8_t BPB_NumFATs;          // Number of FATs
    uint16_t BPB_RootEntCnt;      // Number of root directory entries
    uint16_t BPB_TotSec16;        // Total sectors (16-bit)
    uint8_t BPB_Media;            // Media descriptor
    uint16_t BPB_FATSz16;         // Sectors per FAT
    uint16_t BPB_SecPerTrk;       // Sectors per track
    uint16_t BPB_NumHeads;        // Number of heads
    uint32_t BPB_HiddSec;         // Hidden sectors
    uint32_t BPB_TotSec32;        // Total sectors (32-bit)
};
```

**Key Fields Explained**:

- **BS_jmpBoot**: A jump instruction to skip the BPB and execute the boot code.
- **BS_OEMName**: An identifier for the system that formatted the disk.
- **BPB_BytsPerSec**: Number of bytes per sector, typically 512, 1024, 2048, or 4096.
- **BPB_SecPerClus**: Number of sectors per cluster, a power of 2 (1, 2, 4, 8, etc.).
- **BPB_RsvdSecCnt**: Number of reserved sectors, usually 1 for FAT12.
- **BPB_NumFATs**: Number of FATs, typically 2 for redundancy.
- **BPB_RootEntCnt**: Number of 32-byte directory entries in the root directory.
- **BPB_TotSec16**: Total number of sectors (16-bit). If zero, use BPB_TotSec32.
- **BPB_Media**: Media descriptor indicating the type of storage media.
- **BPB_FATSz16**: Number of sectors occupied by one FAT.
- **BPB_SecPerTrk**: Sectors per track for disks with physical geometry.
- **BPB_NumHeads**: Number of heads for disks with physical geometry.
- **BPB_HiddSec**: Number of hidden sectors preceding the partition.
- **BPB_TotSec32**: Total number of sectors (32-bit).

The Boot Sector defines the physical and logical parameters of the disk, ensuring the file system can properly manage data storage and retrieval.

#### FAT x 2

The FAT12 file system maintains two copies of the File Allocation Table (FAT) to provide redundancy and facilitate error recovery. Each FAT entry corresponds to a cluster and can indicate:

- **Cluster Number**: Points to the next cluster in a chain for a file.
- **End-of-File (EOF) Marker**: Indicates the end of a file's cluster chain.
- **Bad Cluster Marker**: Marks clusters that are damaged and unusable.
- **Free Cluster Marker**: Indicates clusters that are available for allocation.

#### Root Directory

The Root Directory is a special, fixed-size area(`BPB_RootEntCnt*32`) that contains directory entries for the top-level directory of the file system. The size and location of the Root Directory are defined in the Boot Sector. Each directory entry includes metadata about a file or subdirectory, such as its name, extension, attributes, timestamps, starting cluster, and file size. The structure for a directory entry is defined as:

```c++
struct DirectoryEntry {
    char filename[8];              // 8 bytes: Filename
    char extension[3];             // 3 bytes: File extension
    char password[6];              // 6 bytes: Password for file protection (custom addition)
    uint8_t attributes;            // 1 byte: File attributes
    uint8_t reserved[2];           // 2 bytes: Reserved for future use
    Timestamp creation;            // 4 bytes: Timestamp of file creation
    Timestamp last_modification;   // 4 bytes: Timestamp of last modification
    uint16_t starting_cluster;     // 2 bytes: Starting cluster number
    uint32_t file_size;            // 4 bytes: Size of the file in bytes
};
```

#### Data Area

The Data Area follows the Root Directory and is where the actual contents of files and directories are stored. It is divided into clusters, which are the smallest units of allocation in the FAT12 file system. Clusters can become fragmented as files are created, deleted, and modified, but the FAT keeps track of these fragments, ensuring the file system can retrieve the complete file content.

**Key Points:**

- **Cluster Size**: Determined by the number of sectors per cluster.
- **Data Storage**: Files and directories are stored in clusters.
- **Fragmentation Management**: The FAT helps manage fragmented cluster chains.

### Free Blocks Management

In the FAT12 file system, free blocks, also known as free clusters, are effectively managed and tracked using the File Allocation Table (FAT). The FAT serves as a map that corresponds each cluster on the disk with a specific status indicating whether it's free or allocated.

#### Overview of Free Block Management

- **Free Cluster Marker**: Free clusters are identified by a specific value, often `0x000` in FAT12, which is stored in the corresponding FAT entry for that cluster.

#### Constants for Free Block Management

```c
const int FAT_RESERVED_CNT = 2; // First 2 FAT entries are reserved
const FatEntry FAT_ENTRY_UNUSED = 0x00;
const FatEntry FAT_ENTRY_RESERVED_CLUSTER_START = 0xFF0;
const FatEntry FAT_ENTRY_RESERVED_CLUSTER_END = 0xFF6;
const FatEntry FAT_ENTRY_RESERVED_CLUSTER = 0xFF6;
const FatEntry FAT_ENTRY_BAD_CLUSTER = 0xFF7;
const FatEntry FAT_ENTRY_LAST_CLUSTER_START = 0xFF8;
const FatEntry FAT_ENTRY_LAST_CLUSTER_END = 0xFFF;
```

#### How Free Blocks are Managed

1. **Allocation**:
   - When a new file is created or an existing file is extended, the file system scans the FAT to find free clusters.
   - The first available free cluster is then allocated to the file.
   - Subsequently, the FAT entry corresponding to the allocated cluster is updated to indicate that it's now part of the cluster chain for the file.

2. **Deallocation**:
   - Upon file deletion, the file system traverses the cluster chain associated with the file being deleted.
   - Each cluster's FAT entry is then reset to the free cluster marker (`FAT_ENTRY_UNUSED`), signifying that the clusters are now available for reuse.

#### Location of Free Blocks

- **File Allocation Table (FAT)**: 
  - Each entry in the FAT represents a cluster on the disk.
  - Specific FAT entries, such as those marked by `FAT_ENTRY_UNUSED`, `FAT_ENTRY_LAST_CLUSTER_START`, and `FAT_ENTRY_LAST_CLUSTER_END`, denote different states or special cases in the allocation and deallocation process.

#### Importance of Redundancy

- **FAT Redundancy**: FAT12 often employs two copies of the FAT for redundancy.
  - Both FAT copies are updated simultaneously during allocation and deallocation operations, ensuring data consistency and reliability.

Efficient management of free blocks by the FAT12 file system minimizes fragmentation and optimizes disk space utilization, contributing to the overall efficiency and performance of the file system.

### Permission Handling

In the FAT12 file system, permission attributes play a crucial role in determining access rights for various file system operations. These attributes must be evaluated during specific operations to enforce access control and maintain data integrity.

#### Operations Requiring Permission Evaluation

1. **Create File (open/write)**: When creating a new file, the file system should check if the parent directory allows new files to be created (`ATTR_WRITABLE` attribute) and if the file system as a whole allows new files to be created (`ATTR_READABLE` attribute).

2. **Read File (read/open)**: Reading from a file requires the file to have the readable attribute (`ATTR_READABLE`). Additionally, the file must exist and be accessible within its parent directory, which also requires directory readability (`ATTR_READABLE`).

3. **Write to File (write)**: Writing to a file requires both the file and its parent directory to have the writable attribute (`ATTR_WRITABLE`).

4. **Create Directory (mkdir)**: Creating a new directory involves checking if the parent directory allows new directories to be created (`ATTR_WRITABLE` attribute) and if the file system as a whole allows new directories to be created (`ATTR_READABLE` attribute).

5. **Delete File (unlink)**: Deleting a file requires write access to its parent directory (`ATTR_WRITABLE`).

6. **Delete Directory (rmdir)**: Deleting a directory involves checking if the directory is empty and if the parent directory allows directories to be deleted (`ATTR_WRITABLE` attribute).

7. **Rename File/Directory**: Renaming a file or directory requires write access to its parent directory (`ATTR_WRITABLE`).

8. **Change Attributes**: Modifying file or directory attributes requires write access to the file or directory (`ATTR_WRITABLE`).

During these operations, the file system should evaluate the permission attributes associated with the target file or directory and its parent directory to determine whether the requested operation can proceed. This ensures that access control policies are enforced consistently and that unauthorized access attempts are prevented.

### Password Protection Handling in Directory Entries

In the FAT12 file system, password protection can be integrated into directory entries to control access to files or directories. Here's how password attributes in the DirectoryEntry structure can be utilized in various file system operations:

#### Password Attribute Constants

```c++
const uint8_t ATTR_PASSWORD_PROTECTED = 0x40;  // Indicates that the file is password protected file
```

#### Implementation in File System Operations

1. **Create File (mkdir)**:
   - When creating a new file, the file system checks if the parent directory allows new files to be created (`ATTR_WRITABLE` attribute) and if the file system as a whole permits new files (`ATTR_READABLE` attribute).
   - Additionally, if the file is password protected, the appropriate password attributes (`ATTR_PASSWORD_PROTECTED` and `ATTR_PASSWORD_REQUIRED`) are set in the directory entry.

2. **Read File (read/open)**:
   - Reading from a file requires the file to have the readable attribute (`ATTR_READABLE`).
   - If the file is password protected, the file system prompts the user to provide the correct password before allowing access. This involves checking the password attributes (`ATTR_PASSWORD_PROTECTED` and `ATTR_PASSWORD_REQUIRED`) in the directory entry.

3. **Write to File (write)**:
   - Writing to a file requires both the file and its parent directory to have the writable attribute (`ATTR_WRITABLE`).
   - If the file is password protected, the file system prompts the user to provide the correct password before allowing write access. This involves checking the password attributes (`ATTR_PASSWORD_PROTECTED` and `ATTR_PASSWORD_REQUIRED`) in the directory entry.

4. **Open File**:
   - Opening a file involves checking if the file exists and if the user has the necessary permissions to access it.
   - If the file is password protected, the file system prompts the user to provide the correct password before allowing access. This involves checking the password attributes (`ATTR_PASSWORD_PROTECTED` and `ATTR_PASSWORD_REQUIRED`) in the directory entry.

5. **Delete File (unlink)**:
   - Deleting a file requires write access to its parent directory (`ATTR_WRITABLE`).
   - If the file is password protected, the file system prompts the user to provide the correct password before allowing deletion. This involves checking the password attributes (`ATTR_PASSWORD_PROTECTED` and `ATTR_PASSWORD_REQUIRED`) in the directory entry.

By integrating password attributes into directory entries, the FAT12 file system can enforce password protection for files, enhancing security and control over file access.

### Function Names in Implementation

Here are the corresponding function names used in implementation associated with each file system operation listed in the table:

1. **dir**:
   - Function Name: `list_directory_contents`
   - Description: Lists the contents of the directory shown by the path on the screen, similar to the `ls` command in Linux. It displays permissions, modification and creation dates, and whether the file is password protected.

2. **mkdir**:
   - Function Name: `create_directory`
   - Description: Makes a new directory under the specified path. This function works similar to the `mkdir` command in DOS shell.

3. **rmdir**:
   - Function Name: `remove_directory`
   - Description: Removes the specified directory. This function works similar to the `rmdir` command in DOS shell.

4. **dumpe2fs**:
   - Function Name: `print_file_system_info`
   - Description: Provides information about the file system, including block count, free blocks, number of files and directories, and block size. It also lists all occupied blocks and their associated file names.

5. **write**:
   - Function Name: `write_to_file`
   - Description: Creates a new file and writes data into it. This function copies the contents of the Linux file into the new file, preserving Linux file permissions.

6. **read**:
   - Function Name: `read_from_file`
   - Description: Reads data from the specified file. This function writes the data from the file in the file system to the specified Linux file, ensuring that Linux file permissions are maintained.

7. **del**:
   - Function Name: `delete_file`
   - Description: Deletes the specified file from the file system. This function works similar to the `del` command in DOS shell.

8. **chmod**:
   - Function Name: `change_file_permissions`
   - Description: Changes the owner permissions of the specified file. This function works similar to the `chmod` command in Linux.

9. **addpw**:
   - Function Name: `add_password_protection`
   - Description: Adds a protection password to the specified file. After adding a password, any operation on this file must be done with the provided password.