#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include "inode.hpp"

namespace ModV6FileSystem
{
    class FileSystem
    {
    private:
        std::vector<std::unique_ptr<INode>> _inodes;

    public:
        ~FileSystem();
        void quit();
        void openfs(std::string filename);
        void initfs(unsigned long totalBlocks, unsigned long iNodeBlocks);
    };
}