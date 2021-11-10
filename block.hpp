#pragma once
#include <array>
#include <cstdint>
#include "inode.hpp"
#include "directory.hpp"

namespace ModV6FileSystem
{
    // size is 1024 bytes
    struct Block
    {
    private:
        std::array<uint8_t, 1024> _data;
    public:
        ~Block();
        std::array<Directory, 32> asDirectories() const;
        std::array<INode, 16> asINodes() const;
        std::array<uint8_t, 1024> asBytes() const;
        // as indices for free data blocks 
        // in free data block linked list
        std::array<unsigned int, 256> asIntegers() const;
    };
}