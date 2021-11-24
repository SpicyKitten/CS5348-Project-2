#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <unistd.h>
#include "inode.hpp"
#include "file.hpp"
#include "superblock.hpp"

namespace ModV6FileSystem
{
    struct SuperBlock;
    struct File;
    struct INode;

    // size is 1024 bytes
    struct Block
    {
    private:
        struct Data
        {
        public:
            std::array<uint8_t, 1024> bytes;
        };
        Data _data;
        int32_t _fd;
        uint32_t _blockIdx;
    public:
        Block(int32_t fd, uint32_t blockIdx);
        ~Block();
        
        std::array<uint8_t, 1024>& asBytes() const;
        // as indices for free data blocks 
        // in free data block linked list
        std::array<uint32_t, 256>& asIntegers() const;
        uint32_t index() const;
    };
}