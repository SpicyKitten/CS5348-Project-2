#include "directory.hpp"

namespace ModV6FileSystem
{
    unsigned int Directory::inode()
    {
        return this->_data.inode;
    }
    void Directory::inode(unsigned int inode)
    {
        this->_data.inode = inode;
    }
    std::array<char, 28> Directory::filename()
    {
        return this->_data.filename;
    }
    void Directory::filename(std::array<char, 28> filename)
    {
        std::copy(filename.begin(), filename.end(), this->_data.filename.begin());
    }
}