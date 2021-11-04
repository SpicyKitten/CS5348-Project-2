#pragma once
#include <array>

namespace ModV6FileSystem
{
    struct Directory
    {
    private:
        unsigned int _inode;
        std::array<char, 28> _filename;

    public:
        unsigned int inode();
        void inode(unsigned int inode);
        std::array<char, 28> filename();
        void filename(std::array<char, 28> filename);
    };
}