#include "superblock.hpp"

namespace ModV6FileSystem
{
    size_t SuperBlock::isize()
    {
        return this->_isize;
    }
    void SuperBlock::isize(size_t isize)
    {
        this->_isize = isize;
    }
    size_t SuperBlock::fsize()
    {
        return this->_fsize;
    }
    void SuperBlock::fsize(size_t fsize)
    {
        this->_fsize = fsize;
    }
    size_t SuperBlock::nfree()
    {
        return this->_nfree;
    }
    void SuperBlock::nfree(size_t nfree)
    {
        this->_nfree = nfree;
    }
    std::array<unsigned int, 251> SuperBlock::free()
    {
        return this->_free;
    }
    void SuperBlock::free(std::array<unsigned int, 251> free)
    {
        this->_free = free;
    }
    char SuperBlock::flock()
    {
        return this->_flock;
    }
    void SuperBlock::flock(char flock)
    {
        this->_flock = flock;
    }
    char SuperBlock::fmod()
    {
        return this->_fmod;
    }
    void SuperBlock::fmod(char fmod)
    {
        this->_fmod = fmod;
    }
    unsigned int SuperBlock::time()
    {
        return this->_time;
    }
    void SuperBlock::time(unsigned int time)
    {
        this->_time = time;
    }
}