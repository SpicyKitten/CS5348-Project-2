#pragma once
#include <array>

namespace ModV6FileSystem
{
    struct Directory
    {
    private:
        struct Data
        {
        public:
            unsigned int inode;
            std::array<char, 28> filename;
        };
        Data _data;

    public:
        unsigned int inode();
        void inode(unsigned int inode);
        std::array<char, 28> filename();
        void filename(std::array<char, 28> filename);
    };
}