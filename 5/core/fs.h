#ifndef _FS_H
#define _FS_H

#include <string>
#include <bitset>
#include <vector>
#include <stack>
#include <iostream>

#include "disk.h"

using namespace std;

enum class Permissions : uint16_t {
    NONE = 0,
    OTH_E = 01, // Others may execute
    OTH_W = 02, // Others may write
    OTH_R = 04, // Others may read
    OTH_WE = 03,
    OTH_RE = 05,
    OTH_RW = 06,
    OTH_A = 07,
    GRP_E = 010, // Group members may execute
    GRP_W = 020, // Group members may write
    GRP_R = 040, // Group members may read
    GRP_WE = 030,
    GRP_RE = 050,
    GRP_RW = 060,
    GRP_A = 070,
    OWN_E = 0100, // Owner may execute
    OWN_W = 0200, // Owner may write
    OWN_R = 0400, // Owner may read
    OWN_WE = 0300,
    OWN_RE = 0500,
    OWN_RW = 0600,
    OWN_A = 0700,
    DIR = 01000, // Directory
    ALL = 0777,
    ALL_DIR = 01777,
};

using PermissionsT = underlying_type<Permissions>::type;

inline Permissions operator|(Permissions lhs, Permissions rhs) {
    return static_cast<Permissions>(static_cast<PermissionsT>(lhs) | static_cast<PermissionsT>(rhs));
}

inline Permissions operator&(Permissions lhs, Permissions rhs) {
    return static_cast<Permissions>(static_cast<PermissionsT>(lhs) & static_cast<PermissionsT>(rhs));
}

inline Permissions operator^(Permissions lhs, Permissions rhs) {
    return static_cast<Permissions>(static_cast<PermissionsT>(lhs) ^ static_cast<PermissionsT>(rhs));
}

inline Permissions operator~(Permissions rhs) {
    return static_cast<Permissions>(~static_cast<PermissionsT>(rhs));
}

inline Permissions &operator|=(Permissions &lhs, Permissions rhs) {
    lhs = static_cast<Permissions>(static_cast<PermissionsT>(lhs) | static_cast<PermissionsT>(rhs));
    return lhs;
}

inline Permissions &operator&=(Permissions &lhs, Permissions rhs) {
    lhs = static_cast<Permissions>(static_cast<PermissionsT>(lhs) & static_cast<PermissionsT>(rhs));
    return lhs;
}

inline Permissions &operator^=(Permissions &lhs, Permissions rhs) {
    lhs = static_cast<Permissions>(static_cast<PermissionsT>(lhs) ^ static_cast<PermissionsT>(rhs));
    return lhs;
}

inline ostream &operator<<(ostream &os, Permissions p) {
    auto fill = os.fill();
    auto width = os.width();
    os.fill('0');
    os.width(4);
    os << static_cast<PermissionsT>(p);
    os.fill(fill);
    os.width(width);
    return os;
}

/*
 * File System: total * 4096B, can be up to 128MB
 * [SuperBlock] [InodeBitMap] [BlockBitMap] [InodeBlock    ...    InodeBlock] [DataBlock ... DataBlock]
 *  1 * 4096B     1 * 4096B     1 * 4096B           total / 16 * 4096B              rest * 4096B
 * Inode: 64B
 * [mode] [uid] [size] [creationTime] [modificationTime] [direct ... direct] [indirect]
 *   2B    2B     4B        4B              4B                 4B * 11           4B
 */

class FileSystem {
public:
    const static uint32_t MAGIC_NUMBER = 0xdeadbeef;
    const static uint32_t DIRECTORY_ENTRY_SIZE = 32;
    const static uint32_t INODE_SIZE = 64;
    const static uint32_t INODE_COUNT_PER_BLOCK = Disk::BLOCK_SIZE / INODE_SIZE;
    const static uint32_t POINTER_COUNT_PER_BLOCK = Disk::BLOCK_SIZE / sizeof(uint32_t);
    const static uint32_t ENTRY_COUNT_PER_BLOCK = Disk::BLOCK_SIZE / DIRECTORY_ENTRY_SIZE;
    const static uint32_t DIRECT_BLOCKS_PER_INODE = INODE_SIZE / 4 - 5;
    const static uint32_t INDIRECT_BLOCKS_PER_INODE = POINTER_COUNT_PER_BLOCK;

    struct SuperBlock {
        uint32_t magicNumber; // Magic number to identify filesystem
        uint32_t dataBlocks; // Number of data blocks
        uint32_t inodeBlocks; // Number of inode blocks
        uint32_t inodeOffset; // Offset of first inode block
        uint32_t blockOffset; // Offset of first data block
    };

    struct InodeBase {
        Permissions mode; // File mode
        uint16_t uid; // Owner uid
        uint32_t size; // File size
        uint32_t creationTime; // Last creation time
        uint32_t modificationTime; // Last modification time
    };

    struct Inode : InodeBase {
        uint32_t direct[DIRECT_BLOCKS_PER_INODE]; // Direct pointers
        uint32_t indirect; // Indirect pointer
    };

    struct DirectoryEntry {
        uint32_t inode;
        char filename[DIRECTORY_ENTRY_SIZE - 4];
    };

    union Block {
        SuperBlock super;
        bitset<Disk::BLOCK_SIZE * 8> inodeMap;
        bitset<Disk::BLOCK_SIZE * 8> blockMap;
        Inode inodes[INODE_COUNT_PER_BLOCK];
        uint32_t pointers[POINTER_COUNT_PER_BLOCK];
        char data[Disk::BLOCK_SIZE];
        DirectoryEntry directoryEntries[ENTRY_COUNT_PER_BLOCK];
    };
private:
    Disk &disk;
    SuperBlock superBlock;
    bitset<Disk::BLOCK_SIZE * 8> inodeMap; // 1: free, 0: used
    bitset<Disk::BLOCK_SIZE * 8> blockMap; // 1: free, 0: used
    size_t currentInodeIndex = 0; // 0 is root directory
    uint16_t currentUid = 0; // 0 is root

    static uint32_t getTime();

    pair<size_t, size_t> getInodeLocation(size_t index);

    size_t getBlockLocation(size_t index);

    size_t getBlockMapIndex(size_t location);

    void setInodeMap(size_t index, bool free);

    void setBlockMap(size_t index, bool free);

    void checkInode(size_t index, bool shouldBeUsed = false);

    void checkBlock(size_t index);

    Inode getInode(size_t index);

    void setInode(size_t index, Inode inode);

    size_t createInode(Permissions mode = Permissions::OWN_RW | Permissions::GRP_R | Permissions::OTH_R);

    void removeInode(size_t index);

    string readInode(size_t index);

    void writeInode(size_t index, const string &src);

    int writeBlocks(const string &src, uint32_t *begin, const uint32_t *end, int offset);

    void initDirectory(size_t index, size_t parent);

    size_t locateFile(const string &path);

    size_t locateParent(const string &path);

public:
    void format();

    void mount();

    void setUid(uint16_t uid);

    void createFile(const string &path);

    void copyFile(const string &from, const string &to);

    void moveFile(const string &from, const string &to);

    void removeFile(const string &path);

    InodeBase statFile(const string &path);

    vector<pair<string, InodeBase>> listDirectory(const string &path);

    void changeDirectory(const string &path);

    string readFile(const string &path);

    void writeFile(const string &path, const string &src);

    void changeOwner(const string &path, uint16_t uid);

    void changeMode(const string &path, Permissions mode);

    explicit FileSystem(Disk &disk);

    ~FileSystem();

};

#endif // _FS_H
