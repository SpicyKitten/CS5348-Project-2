#pragma once
#include <array>
#include <cstdint>
#include "inode.hpp"
#include "directory.hpp"

namespace ModV6FileSystem
{
    // size is 1024 bytes
    class Block
    {
    private:
        std::array<uint8_t, 1024> data;
    public:
        ~Block();
        std::array<Directory, 32> asDirectories();
        std::array<INode, 16> asINodes();
        std::array<uint8_t, 1024> asBytes();
        // as indices for free data blocks 
        // in free data block linked list
        std::array<unsigned int, 256> asIntegers();
    };
}