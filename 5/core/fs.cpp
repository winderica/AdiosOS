#include "fs.h"
#include "../utils/utils.h"

FileSystem::FileSystem(Disk &disk) : superBlock(SuperBlock()), disk(disk) {
    if (disk.size() < 16) {
        throw runtime_error("Disk size too small");
    }
    superBlock.magicNumber = MAGIC_NUMBER;
    superBlock.inodeBlocks = disk.size() / 16;
    superBlock.dataBlocks = disk.size() - superBlock.inodeBlocks - 3;
    superBlock.inodeOffset = 3;
    superBlock.blockOffset = superBlock.inodeBlocks + superBlock.inodeOffset;
    inodeMap.set();
    blockMap.set();
}

void FileSystem::setInodeMap(size_t index, bool free) {
    free ? inodeMap.set(index) : inodeMap.reset(index);
    Block block{};
    block.inodeMap = inodeMap;
    disk.write(1, block.data);
}

void FileSystem::setBlockMap(size_t index, bool free) {
    free ? blockMap.set(index) : blockMap.reset(index);
    Block block{};
    block.blockMap = blockMap;
    disk.write(2, block.data);
}

void FileSystem::format() {
    if (currentUid != 0) {
        throw runtime_error("Permission denied: formatting can only performed by root(uid 0)");
    }
    { // write SuperBlock
        Block block{};
        block.super = superBlock;
        disk.write(0, block.data);
    }
    for (auto i = 1; i < 3; i++) { // write InodeBitMap and BlockBitMap as all set
        Block block{};
        block.inodeMap.set();
        disk.write(i, block.data);
    }
    inodeMap.set();
    blockMap.set();
    Block emptyBlock{};
    for (auto i = 3; i < disk.size(); i++) { // write empty data to all other blocks
        disk.write(i, emptyBlock.data);
    }
    if (!disk.mounted()) {
        disk.mount();
    }
    currentInodeIndex = 0; // go back to /
    auto rootIndex = createInode(Permissions::ALL_DIR);
    if (rootIndex != 0) { // root inode index should be 0
        throw runtime_error("Unexpected root inode index " + to_string(rootIndex));
    }
    initDirectory(rootIndex, rootIndex);
}

void FileSystem::mount() {
    Block block{};
    disk.read(0, block.data); // read SuperBlock
    if (block.super.magicNumber != MAGIC_NUMBER) {
        throw runtime_error("Unexpected magic number, you should format it first");
    }
    disk.mount();
    superBlock = block.super;
    disk.read(1, block.data);
    inodeMap = block.inodeMap;
    disk.read(2, block.data);
    blockMap = block.blockMap;
}

void FileSystem::setUid(uint16_t uid) {
    currentUid = uid;
}

uint32_t FileSystem::getTime() {
    using namespace chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

pair<size_t, size_t> FileSystem::getInodeLocation(size_t index) {
    auto number = index / INODE_COUNT_PER_BLOCK + superBlock.inodeOffset;
    auto offset = index % INODE_COUNT_PER_BLOCK;
    return {number, offset};
}

size_t FileSystem::getBlockLocation(size_t index) {
    return index + superBlock.blockOffset;
}

size_t FileSystem::getBlockMapIndex(size_t location) {
    return location - superBlock.blockOffset;
}

void FileSystem::checkInode(size_t index, bool shouldBeUsed) {
    if (!disk.mounted()) {
        throw runtime_error("BFS is not mounted");
    }
    if (index >= superBlock.inodeBlocks) {
        throw runtime_error("Space for inodes is not enough");
    }
    if (shouldBeUsed && inodeMap[index]) {
        throw runtime_error("Invalid inode index");
    }
}

void FileSystem::checkBlock(size_t index) {
    if (!disk.mounted()) {
        throw runtime_error("BFS is not mounted");
    }
    if (index >= superBlock.dataBlocks) {
        throw runtime_error("Space for blocks is not enough");
    }
}

void FileSystem::initDirectory(size_t index, size_t parent) {
    Block entryBlock{};
    entryBlock.directoryEntries[0].inode = index;
    entryBlock.directoryEntries[0].filename[0] = '.';
    entryBlock.directoryEntries[1].inode = parent;
    entryBlock.directoryEntries[1].filename[0] = '.';
    entryBlock.directoryEntries[1].filename[1] = '.';
    auto data = string(entryBlock.data, 2 * DIRECTORY_ENTRY_SIZE);
    writeInode(index, data);
}

size_t FileSystem::createInode(Permissions mode) {
    auto index = inodeMap._Find_first(); // first free inode
    checkInode(index);
    setInodeMap(index, false); // mark as used

    Inode inode{};
    inode.mode = mode;
    inode.uid = currentUid;
    inode.size = 0;
    inode.creationTime = getTime();
    inode.modificationTime = inode.creationTime;
    setInode(index, inode);

    return index;
}

void FileSystem::removeInode(size_t index) {
    checkInode(index, true);
    Block emptyBlock{};
    auto inode = getInode(index);
    auto directFilled = true;
    for (auto i = begin(inode.direct); i != end(inode.direct); i++) { // free direct blocks
        auto location = *i;
        auto mapIndex = getBlockMapIndex(location);
        if (location == 0 || blockMap[mapIndex]) {
            directFilled = false;
            break;
        }
        setBlockMap(mapIndex, true);
        disk.write(location, emptyBlock.data);
    }
    if (directFilled && inode.indirect != 0) { // free indirect blocks
        Block pointerBlock{};
        disk.read(inode.indirect, pointerBlock.data);
        for (auto i = begin(pointerBlock.pointers); i != end(pointerBlock.pointers); i++) {
            auto location = *i;
            auto mapIndex = getBlockMapIndex(location);
            if (location == 0 || blockMap[mapIndex]) {
                break;
            }
            setBlockMap(mapIndex, true);
            disk.write(location, emptyBlock.data);
        }
        setBlockMap(getBlockMapIndex(inode.indirect), true);
        disk.write(inode.indirect, emptyBlock.data); // free indirect blocks pointer
    }
    Inode emptyInode{};
    setInode(index, emptyInode); // free inode
    setInodeMap(index, true); // update InodeBitMap
}

FileSystem::Inode FileSystem::getInode(size_t index) {
    checkInode(index, true);
    Block inodeBlock{};
    auto[inodeBlockNumber, inodeBlockOffset] = getInodeLocation(index);
    disk.read(inodeBlockNumber, inodeBlock.data);
    return inodeBlock.inodes[inodeBlockOffset];
}

void FileSystem::setInode(size_t index, FileSystem::Inode inode) {
    checkInode(index, true);
    Block inodeBlock{};
    auto[inodeBlockNumber, inodeBlockOffset] = getInodeLocation(index);
    disk.read(inodeBlockNumber, inodeBlock.data);
    inodeBlock.inodes[inodeBlockOffset] = inode;
    disk.write(inodeBlockNumber, inodeBlock.data);
}

string FileSystem::readInode(size_t index) {
    checkInode(index, true);
    auto inode = getInode(index);
    if ((inode.mode & (inode.uid == currentUid ? Permissions::OWN_R : Permissions::OTH_R)) == Permissions::NONE) {
        throw runtime_error("Permission denied");
    }
    Block dataBlock{};
    string res;
    auto directFilled = true;
    for (auto i = begin(inode.direct); i != end(inode.direct); i++) { // read direct blocks
        auto location = *i;
        auto mapIndex = getBlockMapIndex(location);
        if (location == 0 || blockMap[mapIndex]) {
            directFilled = false;
            break;
        }
        disk.read(location, dataBlock.data);
        res.append(dataBlock.data, Disk::BLOCK_SIZE);
    }
    if (directFilled && inode.indirect != 0) {
        Block pointerBlock{};
        disk.read(inode.indirect, pointerBlock.data);
        for (auto i = begin(pointerBlock.pointers); i != end(pointerBlock.pointers); i++) { // read indirect blocks
            auto location = *i;
            auto mapIndex = getBlockMapIndex(location);
            if (location == 0 || blockMap[mapIndex]) {
                break;
            }
            disk.read(location, dataBlock.data);
            res.append(dataBlock.data, Disk::BLOCK_SIZE);
        }
    }
    res.resize(inode.size);
    return res;
}

void FileSystem::writeInode(size_t index, const string &src) { // no plan to implement offset
    checkInode(index, true);
    if (src.length() >= (DIRECT_BLOCKS_PER_INODE + INDIRECT_BLOCKS_PER_INODE) * Disk::BLOCK_SIZE) {
        throw runtime_error("Source size exceeds capability of BFS");
    }
    auto inode = getInode(index);
    if ((inode.mode & (inode.uid == currentUid ? Permissions::OWN_W : Permissions::OTH_W)) == Permissions::NONE) {
        throw runtime_error("Permission denied");
    }
    auto srcOffset = writeBlocks(src, begin(inode.direct), end(inode.direct), 0); // write direct blocks
    if (srcOffset < src.length()) {
        Block pointerBlock{};
        if (inode.indirect == 0) {
            auto indirectMapIndex = blockMap._Find_first();
            checkBlock(indirectMapIndex);
            auto indirectLocation = getBlockLocation(indirectMapIndex);
            setBlockMap(indirectMapIndex, false);
            inode.indirect = indirectLocation;
        } else {
            disk.read(inode.indirect, pointerBlock.data);
        }
        writeBlocks(src, begin(pointerBlock.pointers), end(pointerBlock.pointers), srcOffset); // write indirect blocks
        disk.write(inode.indirect, pointerBlock.data);
    }
    inode.size = src.length();
    inode.modificationTime = getTime(); // update modification time
    setInode(index, inode);
}

int FileSystem::writeBlocks(const string &src, uint32_t *begin, const uint32_t *end, int offset) {
    auto BLOCK_SIZE = Disk::BLOCK_SIZE;
    for (auto i = begin; i != end && offset < src.length(); i++, offset += BLOCK_SIZE) {
        auto length = min(BLOCK_SIZE, src.length() - offset);
        Block dataBlock{};
        if (*i == 0) {
            auto mapIndex = blockMap._Find_first();
            checkBlock(mapIndex);
            *i = getBlockLocation(mapIndex);
            setBlockMap(mapIndex, false);
        } else {
            disk.read(*i, dataBlock.data);
        }
        auto newData = string(dataBlock.data, BLOCK_SIZE);
        newData.replace(0, length, src, offset, length);
        disk.write(*i, newData.data());
    }
    return offset;
}

size_t FileSystem::locateFile(const string &path) {
    auto parts = Utils::split(path, "/");
    auto currentIndex = path[0] == '/' ? 0 : currentInodeIndex;
    for (auto &part : parts) {
        auto data = readInode(currentIndex);
        auto entries = reinterpret_cast<DirectoryEntry *>(data.data());
        auto matched = false;
        for (auto i = 0; i < data.size() / DIRECTORY_ENTRY_SIZE; i++) {
            if (part == entries[i].filename) {
                currentIndex = entries[i].inode;
                matched = true;
                break;
            }
        }
        if (!matched) {
            throw runtime_error("Illegal path: " + part + " does not exist");
        }
    }
    return currentIndex;
}

size_t FileSystem::locateParent(const string &path) {
    auto parentPath = path[path.size() - 1] == '/' ? path.substr(0, path.size() - 1) : path;
    auto lastSlash = parentPath.find_last_of('/');
    if (lastSlash == string::npos) {
        return currentInodeIndex;
    }
    parentPath = parentPath.substr(0, lastSlash) + "/";
    auto index = locateFile(parentPath);
    auto inode = getInode(index);
    if ((inode.mode & Permissions::DIR) == Permissions::NONE) {
        throw runtime_error("Illegal path: " + parentPath + " is not a directory");
    }
    return index;
}

void FileSystem::createFile(const string &path) {
    if (path == "/") {
        throw runtime_error("Root directory has already been created");
    }
    auto isDirectory = path[path.size() - 1] == '/';
    auto parts = Utils::split(path, "/");
    auto filename = parts[parts.size() - 1];
    if (filename.length() <= 0 || filename.length() >= sizeof(DirectoryEntry::filename)) {
        throw runtime_error("Illegal filename");
    }
    auto index = locateParent(path);
    union {
        char *data;
        DirectoryEntry *entries;
    } temp{};
    auto data = readInode(index);
    auto inode = getInode(index);
    data.resize(inode.size + DIRECTORY_ENTRY_SIZE);
    temp.data = data.data();
    for (int i = 0; i < inode.size / DIRECTORY_ENTRY_SIZE; i++) {
        if (string(temp.entries[i].filename) == filename) {
            throw runtime_error("Illegal path: " + filename + " already exists");
        }
    }
    auto &newEntry = temp.entries[inode.size / DIRECTORY_ENTRY_SIZE];
    // std::copy is a more C++ way than str(n)cpy
    copy(filename.begin(), filename.end(), newEntry.filename);
    newEntry.inode = createInode(
        (isDirectory ? Permissions::DIR : Permissions::NONE)
        | Permissions::OWN_RW | Permissions::GRP_R | Permissions::OTH_R
    );
    if (isDirectory) {
        initDirectory(newEntry.inode, index);
    }
    writeInode(index, string(temp.data, inode.size + DIRECTORY_ENTRY_SIZE));
}

void FileSystem::removeFile(const string &path) {
    auto index = locateFile(path);
    if (index == 0) {
        throw runtime_error("Root directory cannot be removed");
    }
    auto inode = getInode(index);
    if (inode.uid != currentUid) { // only owner is checked when removing
        throw runtime_error("Permission denied: file/directory can only be removed by owner");
    }
    auto parent = locateParent(path);
    union {
        char *data;
        DirectoryEntry *entries;
    } temp{};
    auto parentData = readInode(parent);
    auto parentInode = getInode(parent);
    temp.data = parentData.data();
    for (int i = 0; i < parentInode.size / DIRECTORY_ENTRY_SIZE; i++) {
        if (temp.entries[i].inode == index) {
            copy(&temp.entries[i + 1], &temp.entries[parentInode.size / DIRECTORY_ENTRY_SIZE], &temp.entries[i]);
            break;
        }
    }
    writeInode(parent, string(temp.data, parentInode.size - DIRECTORY_ENTRY_SIZE)); // update parent entry

    stack<size_t> directories;
    vector<size_t> toRemove;
    toRemove.push_back(index);
    if ((inode.mode & Permissions::DIR) != Permissions::NONE) {
        directories.push(index);
    }
    while (!directories.empty()) { // bfs in BFS
        auto data = readInode(directories.top());
        directories.pop();
        auto entries = reinterpret_cast<DirectoryEntry *>(data.data());
        for (auto i = 0; i < data.size() / DIRECTORY_ENTRY_SIZE; i++) {
            auto entry = entries[i];
            if (string(entry.filename) == "." || string(entry.filename) == "..") {
                continue;
            }
            inode = getInode(entry.inode);
            if ((inode.mode & Permissions::DIR) != Permissions::NONE) {
                directories.push(entry.inode);
            }
            toRemove.push_back(entry.inode);
        }
    }
    for (auto i: toRemove) { // remove all files
        removeInode(i);
    }
}

FileSystem::InodeBase FileSystem::statFile(const string &path) {
    auto inode = getInode(locateFile(path));
    // cast from Inode to InodeBase directly could be more concise, though.
    return {inode.mode, inode.uid, inode.size, inode.creationTime, inode.modificationTime};
}

void FileSystem::copyFile(const string &from, const string &to) {
    if (from[from.size() - 1] == '/' || to[to.size() - 1] == '/') {
        throw runtime_error("Copying directory is not supported");
    }
    createFile(to);
    writeFile(to, readFile(from));
}

void FileSystem::moveFile(const string &from, const string &to) {
    if (from[from.size() - 1] == '/' || to[to.size() - 1] == '/') {
        throw runtime_error("Moving directory is not supported");
    }
    copyFile(from, to);
    removeFile(from);
}

void FileSystem::changeDirectory(const string &path) {
    auto index = locateFile(path);
    auto inode = getInode(index);
    if ((inode.mode & Permissions::DIR) == Permissions::NONE) {
        throw runtime_error("Illegal path: " + path + " is not a directory");
    }
    currentInodeIndex = index;
}

vector<pair<string, FileSystem::InodeBase>> FileSystem::listDirectory(const string &path) {
    vector<pair<string, FileSystem::InodeBase>> stats;
    string data;
    if (path.empty()) {
        data = readInode(currentInodeIndex);
    } else {
        auto index = locateFile(path);
        auto inode = getInode(index);
        if ((inode.mode & Permissions::DIR) == Permissions::NONE) {
            throw runtime_error("Illegal path: " + path + " is not a directory");
        }
        data = readInode(index);
    }
    auto entries = reinterpret_cast<DirectoryEntry *>(data.data());
    for (auto i = 0; i < data.size() / DIRECTORY_ENTRY_SIZE; i++) {
        auto entry = entries[i];
        stats.emplace_back(string(entry.filename), getInode(entry.inode));
    }
    return stats;
}

string FileSystem::readFile(const string &path) {
    auto index = locateFile(path);
    auto inode = getInode(index);
    if ((inode.mode & Permissions::DIR) != Permissions::NONE) {
        throw runtime_error("Reading directory is not allowed");
    }
    return readInode(index);
}

void FileSystem::writeFile(const string &path, const string &src) {
    auto index = locateFile(path);
    auto inode = getInode(index);
    if ((inode.mode & Permissions::DIR) != Permissions::NONE) {
        throw runtime_error("Writing directory is not allowed");
    }
    writeInode(index, src);
}

void FileSystem::changeOwner(const string &path, uint16_t uid) {
    auto index = locateFile(path);
    if (index == 0) {
        throw runtime_error("Permission denied: uid of root directory cannot be changed");
    }
    auto inode = getInode(index);
    inode.uid = uid;
    setInode(index, inode);
}

void FileSystem::changeMode(const string &path, Permissions mode) {
    auto index = locateFile(path);
    if (index == 0) {
        throw runtime_error("Permission denied: mode of root directory cannot be changed");
    }
    auto inode = getInode(index);
    if (inode.uid != currentUid) {
        throw runtime_error("Permission denied: mode can only be changed by owner");
    }
    mode = mode & Permissions::ALL; // mode should not be larger than 0777
    inode.mode = (inode.mode & Permissions::DIR) | mode;
    setInode(index, inode);
}

FileSystem::~FileSystem() {
    if (disk.mounted()) {
        disk.unmount();
    }
}
