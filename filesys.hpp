#pragma once
#include <algorithm>
#include <string>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <tuple>
#include "inode.hpp"
#include "block.hpp"

namespace ModV6FileSystem
{
    struct FileSystem
    {
    public:
        std::vector<std::weak_ptr<Block>> _blocks;
        int32_t _fd;
        std::string _working_directory;
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
        std::shared_ptr<Block> getBlock(uint32_t blockIdx);
        std::shared_ptr<INode> getINode(uint32_t iNodeIdx);
        std::array<std::shared_ptr<File>, 32> getFiles(uint32_t blockIdx);
        std::shared_ptr<SuperBlock> getSuperBlock();
        void freeDataBlock(std::shared_ptr<SuperBlock> superblock_ptr, uint32_t blockIdx);
        uint32_t allocateDataBlock(std::shared_ptr<SuperBlock> superblock_ptr);
        uint32_t allocateINode();
        void initializeFreeList(std::shared_ptr<SuperBlock> superblock_ptr);
        void initializeINodes();
        void initializeRoot();
        std::array<char, 28> filenameToArray(std::string filename);
    public:
        FileSystem();
        ~FileSystem();
        
        void quit();
        void openfs(const std::string& filename);
        void initfs(uint32_t totalBlocks, uint32_t iNodeBlocks);
        void cpin(const std::string& outerFilename, const std::string& innerFilename);
        void cpout(const std::string& innerFilename, const std::string& outerFilename);
        void rm(const std::string& innerFilename);
        void mkdir(const std::string& innerFilename);
        void cd(const std::string& innerFilename);
    };
}