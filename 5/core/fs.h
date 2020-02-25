#ifndef _FS_H
#define _FS_H

#include "disk.h"

#include <string>
#include <bitset>
#include <vector>
#include <stack>

using namespace std;

/*
 * File System: total * 4096B, can be up to 128MB
 * [SuperBlock] [InodeBitMap] [BlockBitMap] [InodeBlock    ...    InodeBlock] [DataBlock ... DataBlock]
 *  1 * 4096B     1 * 4096B     1 * 4096B           total / 16 * 4096B              rest * 4096B
 * Inode: 64B
 * [mode] [uid] [size] [create_time] [modification_time] [direct ... direct] [indirect]
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
        /*
         * Mode: file mode
         * 0x1: Others may execute
         * 0x2: Others may write
         * 0x4: Others may read
         * 0x8: Group members may execute
         * 0x10: Group members may write
         * 0x20: Group members may read
         * 0x40: Owner may execute
         * 0x80: Owner may write
         * 0x100: Owner may read
         * 0x200: Directory
         */
        uint16_t mode;
        uint16_t uid; // Owner uid
        uint32_t size; // File size
        uint32_t create_time; // Last create time
        uint32_t modification_time; // Last modification time
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
    size_t currentInodeIndex = 0;

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

    ssize_t createInode(uint16_t mode = 0644);

    void removeInode(size_t index);

    string readInode(size_t index);

    void writeInode(size_t index, const string &src);

    int writeBlocks(const string &src, uint32_t *begin, const uint32_t *end, int srcOffset);

    void initDirectory(size_t index, size_t parent);

    size_t locateFile(const string &path);

    size_t locateParent(const string &path);

public:
    void format();

    void mount();

    void createFile(const string &path);

    void copyFile(const string &from, const string &to);

    void moveFile(const string &from, const string &to);

    void removeFile(const string &path);

    InodeBase statFile(const string &path);

    vector<pair<string, FileSystem::InodeBase>> listDirectory(const string &path);

    void changeDirectory(const string &path);

    string readFile(const string &path);

    void writeFile(const string &path, const string &src);

    explicit FileSystem(Disk &disk);

    ~FileSystem();

};

#endif // _FS_H
