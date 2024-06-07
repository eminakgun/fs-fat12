
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

## Attributes

ATTR_READ_ONLY (0x01)
ATTR_HIDDEN (0x02) 
ATTR_SYSTEM (0x04) 
ATTR_VOLUME_ID (0x08)
ATTR_DIRECTORY (0x10)
ATTR_ARCHIVE (0x20)


32 sectors : 16KB
64 sectors: 32 KB


(32 + 257) / 258 = 