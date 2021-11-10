#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include "inode.hpp"
#include "block.hpp"

namespace ModV6FileSystem
{
    struct FileSystem
    {
    private:
        std::vector<std::unique_ptr<INode>> _inodes;
        std::vector<std::shared_ptr<Block>> _blocks;
        int32_t _fd;
        uint32_t TOTAL_BLOCKS;
        uint32_t BOOT_INFO_BLOCKS;
        uint32_t SUPERBLOCK_BLOCKS;
        uint32_t INODE_BLOCKS;
        uint32_t DATA_BLOCKS;
        uint32_t BOOT_INFO_BLOCK_IDX;
        uint32_t SUPERBLOCK_IDX;
        uint32_t INODE_BLOCK_IDX;
        uint32_t DATA_BLOCK_IDX;
        uint32_t OUT_OF_BOUNDS;

        void reset();
        void setDimensions(uint32_t totalBlocks, uint32_t iNodeBlocks);

    public:
        std::shared_ptr<Block> getBlock(uint32_t);
        FileSystem();
        ~FileSystem();
        void quit();
        void openfs(const std::string& filename);
        void initfs(uint32_t totalBlocks, uint32_t iNodeBlocks);
    };
}