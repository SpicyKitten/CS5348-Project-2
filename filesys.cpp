#include "filesys.hpp"

FileSystem::~FileSystem()
{
    std::cout << "~FileSystem" << std::endl;
}
void FileSystem::initfs(unsigned long totalBlocks, unsigned long iNodeBlocks)
{
    std::cout << "Executing initfs " << totalBlocks << " " << iNodeBlocks << std::endl;
    // delete the old i-nodes
    this->_inodes.clear();
    // total size = totalBlocks * block size = totalBlocks * 1024 bytes
    // i-nodes = 16 i-nodes per block
    // total i-nodes = 16 * iNodeBlocks
    // block structure:
    // ______________________________________________________________________________________
    // | boot info | superblock | ... i-node blocks ... |        ... data blocks ...        |
    // |     0     |     1      | 2 to iNodeBlocks + 1  | iNodeBlocks + 2 to totalBlocks - 1|
    // ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ 
    this->_inodes.push_back(std::unique_ptr<INode>(new INode()));
}
void FileSystem::openfs(std::string filename)
{
    std::cout << "Executing openfs " << filename << std::endl;
    // create a file if it doesn't exist
    // open the file otherwise
    auto fd = open(filename.c_str(), O_CREAT | O_RDWR);
    write(fd, filename.c_str(), sizeof(filename.c_str()));
    close(fd);
    std::cout << "Obtained file descriptor: " << fd << std::endl;
}