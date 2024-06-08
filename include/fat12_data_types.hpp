#ifndef FAT12_ADT_HPP
#define FAT12_ADT_HPP

#include <cstdint>

namespace fat12 {


    /*
        Boot sector contains 36 bytes
    */
    #pragma pack(push, 1)
    struct BootSector {
        char BS_jmpBoot[3]; // Jump instruction to boot code
        char BS_OEMName[8]; // OEM Name Identifier

        /*
            Count of bytes per sector. This value may take on
            only the following values: 512, 1024, 2048 or 4096.
        */
        uint16_t BPB_BytsPerSec; // bytes per sector

        /* Sectors per allocation unit(cluster)
           This value must be a power of 2 that is greater than 0. The
           legal values are 1, 2, 4, 8, 16, 32, 64, and 128
        */ 

        uint8_t BPB_SecPerClus;

        /*
            Number of reserved sectors in the reserved region
            of the volume starting at the first sector of the
            volume. 
            This field is used to align the start of the
            data area to integral multiples of the cluster size
            with respect to the start of the partition/media. 
        */
        uint16_t BPB_RsvdSecCnt;

        /*
            The count of file allocation tables (FATs) on the
            volume. 
        */
        uint8_t BPB_NumFATs; // number of fats


        /*
            For FAT12 and FAT16 volumes, this field contains
            the count of 32-byte directory entries in the root
            directory. And this value should always specify 
            a count that when multiplied by 32 results in an 
            even multiple of BPB_BytsPerSec
        */
        uint16_t BPB_RootEntCnt;
        
        /*
            This field is the old 16-bit total count of sectors on
            the volume. This count includes the count of all
            sectors in all four regions of the volume.
            For FAT12 and FAT16 volumes, this field contains
            the sector count, and BPB_TotSec32 is 0 if the
            total sector count “fits” (is less than 0x10000)
        */
        uint16_t BPB_TotSec16; // total sectors;


        /*
            The legal values for this field are 0xF0, 0xF8, 0xF9,
            0xFA, 0xFB, 0xFC, 0xFD, 0xFE, and 0xFF.
            0xF8 is the standard value for “fixed” (nonremovable) media. 
            For removable media, 0xF0 is frequently used
        */
        uint8_t BPB_Media;

        /*
            This field is the FAT12/FAT16 16-bit count of
            sectors occupied by one FAT.
        */
        uint16_t BPB_FATSz16; // sectors per FAT

        /*
            Sectors per track for interrupt 0x13.
            This field is only relevant for media that have a
            geometry (volume is broken down into tracks by
            multiple heads and cylinders) and are visible on
            interrupt 0x13
        */
       uint16_t BPB_SecPerTrk;

        /*
            Number of heads for interrupt 0x13. This field is
            relevant as discussed earlier for BPB_SecPerTrk.
            This field contains the one based “count of heads”.
            For example, on a 1.44 MB 3.5-inch floppy drive
            this value is 2.
        */
       uint16_t BPB_NumHeads;

        /*
            Count of hidden sectors preceding the partition that
            contains this FAT volume. This field is generally
            only relevant for media visible on interrupt 0x13.
            This field must always be zero on media that are
            not partitioned. 

        */
       uint32_t BPB_HiddSec;

        /*
            This field is the new 32-bit total count of sectors on
            the volume. This count includes the count of all
            sectors in all four regions of the volume.

            This field can be 0; if it is 0, then BPB_TotSec16
            must be non-zero. For FAT12/FAT16 volumes, this
            field contains the sector count if BPB_TotSec16 is 0
        */
       uint32_t BPB_TotSec32;
    };
    #pragma pack(pop)

    // Default paramters of boot sector 
    const uint16_t DEFAULT_BYTSPERSEC = 512;
    const uint8_t  DEFAULT_SECPERCLUS = 1;
    const uint8_t  DEFAULT_BLOCK_SIZE = static_cast<uint8_t>(DEFAULT_BYTSPERSEC * DEFAULT_SECPERCLUS); 
    const uint16_t DEFAULT_RSVDSECCNT = 1;
    const uint16_t DEFAULT_NUMFATS    = 2;
    const uint8_t  DEFAULT_ROOTENTCNT = 224;
    const uint16_t DEFAULT_FAT12_SIZE = 9; // 9 sectors
    const uint8_t  MEDIA_REMOVABLE = 0xF0;
    const uint8_t  MEDIA_NONREMOVABLE = 0xF8;
    const uint16_t EOC_MARKER = 0xFFF;


    #pragma pack(push, 1)
    struct Timestamp {
        uint16_t time;
        uint16_t date;
    };
    #pragma pack(pop)

    // Ensure packing of the struct to match the exact layout expected in FAT12
    #pragma pack(push, 1)
    struct DirectoryEntry {
        char filename[8];              // 8 bytes: Filename
        char extension[3];             // 3 bytes: File extension
        char password[6];
        uint8_t attributes;            // 1 byte: File attributes
        uint8_t reserved[2];          // 10 bytes: Reserved
        Timestamp creation;
        Timestamp last_modification;
        uint16_t starting_cluster;     // 2 bytes: Starting cluster number
        uint32_t file_size;            // 4 bytes: Size of the file in bytes
    };
    #pragma pack(pop)

    // Define legal file attribute types as constants
    const uint8_t ATTR_READABLE = 0x01;
    const uint8_t ATTR_WRITABLE = 0x02;
    const uint8_t ATTR_SYSTEM = 0x04;
    const uint8_t ATTR_VOLUME_ID = 0x08;
    const uint8_t ATTR_DIRECTORY = 0x10;
    const uint8_t ATTR_ARCHIVE = 0x20;
    const uint8_t ATTR_PASSWORD_PROTECTED = 0x40;


    const unsigned char DIR_NAME_FREE[2] = {0xE5, 0x00}; 


    // Define FAT entry data type as uint16_t
    using FatEntry = uint16_t;
    
    const int FAT_RESERVED_CNT = 2; // First 2 fat entries are reserved
    const FatEntry FAT_ENTRY_UNUSED = 0x00;
    const FatEntry FAT_ENTRY_RESERVED_CLUSTER_START = 0xFF0;
    const FatEntry FAT_ENTRY_RESERVED_CLUSTER_END = 0xFF6;
    const FatEntry FAT_ENTRY_RESERVED_CLUSTER = 0xFF6;
    const FatEntry FAT_ENTRY_BAD_CLUSTER = 0xFF7;
    const FatEntry FAT_ENTRY_LAST_CLUSTER_START = 0xFF8;
    const FatEntry FAT_ENTRY_LAST_CLUSTER_END = 0xFFF;

}


#endif

