#include "superblock.hpp"

namespace ModV6FileSystem
{
    size_t SuperBlock::isize() const
    {
        return this->_data.isize;
    }
    void SuperBlock::isize(size_t isize)
    {
        this->_data.isize = isize;
    }
    size_t SuperBlock::fsize() const
    {
        return this->_data.fsize;
    }
    void SuperBlock::fsize(size_t fsize)
    {
        this->_data.fsize = fsize;
    }
    size_t SuperBlock::nfree() const
    {
        return this->_data.nfree;
    }
    void SuperBlock::nfree(size_t nfree)
    {
        this->_data.nfree = nfree;
    }
    std::array<unsigned int, 251> SuperBlock::free() const
    {
        return this->_data.free;
    }
    void SuperBlock::free(std::array<unsigned int, 251> free)
    {
        this->_data.free = free;
    }
    char SuperBlock::flock() const
    {
        return this->_data.flock;
    }
    void SuperBlock::flock(char flock)
    {
        this->_data.flock = flock;
    }
    char SuperBlock::fmod() const
    {
        return this->_data.fmod;
    }
    void SuperBlock::fmod(char fmod)
    {
        this->_data.fmod = fmod;
    }
    unsigned int SuperBlock::time() const
    {
        return this->_data.time;
    }
    void SuperBlock::time(unsigned int time)
    {
        this->_data.time = time;
    }
}