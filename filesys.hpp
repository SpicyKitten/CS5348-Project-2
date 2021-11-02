#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include "inode.hpp"
#include <fcntl.h>
#include <unistd.h>

class FileSystem
{
private:

    std::vector<std::unique_ptr<INode>> _inodes;
public:
    ~FileSystem();
    void openfs(std::string filename);
    void initfs(unsigned long totalBlocks, unsigned long iNodeBlocks);
};