#include "filesys.hpp"

void FileSystem::initfs(unsigned long totalBlocks, unsigned long iNodeBlocks)
{
    std::cout << "Executing initfs " << totalBlocks << " " << iNodeBlocks << std::endl;
}
void FileSystem::openfs(std::string filename)
{
    std::cout << "Executing openfs " << filename << std::endl;
}