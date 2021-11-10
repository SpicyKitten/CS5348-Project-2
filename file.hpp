#pragma once
#include <array>
#include <iostream>
#include <string>

namespace ModV6FileSystem
{
    struct File
    {
    private:
        struct Data
        {
        public:
            uint32_t inode;
            std::array<char, 28> filename;
        };
        Data _data;

    public:
        friend std::ostream &operator<<(std::ostream &ostream, const File& in);
        uint32_t inode() const;
        void inode(uint32_t inode);
        std::array<char, 28> filename() const;
        void filename(std::array<char, 28> filename);
    };
}