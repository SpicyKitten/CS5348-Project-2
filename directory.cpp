#include "directory.hpp"

namespace ModV6FileSystem
{
    unsigned int Directory::inode()
    {
        return this->_inode;
    }
    void Directory::inode(unsigned int inode)
    {
        this->_inode = inode;
    }
    std::array<char, 28> Directory::filename()
    {
        return this->_filename;
    }
    void Directory::filename(std::array<char, 28> filename)
    {
        std::copy(filename.begin(), filename.end(), this->_filename.begin());
    }
}