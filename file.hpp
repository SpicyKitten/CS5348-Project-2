#pragma once
#include <array>
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include "block.hpp"

namespace ModV6FileSystem
{
    struct Block;

    struct File
    {
    private:
        struct Data
        {
        public:
            uint32_t inode;
            std::array<char, 28> filename;
        };
        Data& _data;
        std::shared_ptr<Block> _block;

    public:
        friend std::ostream &operator<<(std::ostream &ostream, const File& in);
        File(std::shared_ptr<Block> block, uint32_t offset);
        ~File();
        
        uint32_t inode() const;
        void inode(uint32_t inode);
        std::array<char, 28> filename() const;
        void filename(std::array<char, 28> filename);
        std::string name() const;
        void name(std::string filename);
    };
}