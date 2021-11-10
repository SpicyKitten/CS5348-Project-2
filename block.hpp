#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include "inode.hpp"
#include "file.hpp"
#include "superblock.hpp"

namespace ModV6FileSystem
{
    struct SuperBlock;

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
    public:
        ~Block();

        std::array<File, 32>& asFiles() const;
        std::array<INode, 16>& asINodes() const;
        std::array<uint8_t, 1024>& asBytes() const;
        // as indices for free data blocks 
        // in free data block linked list
        std::array<unsigned int, 256>& asIntegers() const;
        SuperBlock asSuperBlock() const;
    };
}