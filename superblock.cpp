#include "superblock.hpp"

namespace ModV6FileSystem
{
    std::ostream &operator<<(std::ostream &ostream, const SuperBlock& in)
    {
        std::string freeStr = "[";
        std::string prefix = "";
        for(auto num : in.free())
        {
            freeStr = freeStr + prefix + std::to_string(num);
            prefix = ", ";
        }
        freeStr += "]";
        return ostream << "SuperBlock[isize=" << in.isize() << ", fsize=" << in.fsize()
            << ", nfree=" << in.nfree() << ", free=" << freeStr << ", flock"
            << in.flock() << ", ilock=" << in.ilock() << ", fmod=" << in.fmod()
            << ", time=" << in.time() << "]";
    }
    SuperBlock::SuperBlock(std::shared_ptr<Block> block) : 
        _data(*reinterpret_cast<Data*>(block->asBytes().data())), _block(block)
    {
    }
    SuperBlock::~SuperBlock()
    {
        std::cout << "~SuperBlock" << std::endl;
    }
    uint32_t SuperBlock::isize() const
    {
        return this->_data.isize;
    }
    void SuperBlock::isize(uint32_t isize)
    {
        this->_data.isize = isize;
    }
    uint32_t SuperBlock::fsize() const
    {
        return this->_data.fsize;
    }
    void SuperBlock::fsize(uint32_t fsize)
    {
        this->_data.fsize = fsize;
    }
    uint32_t SuperBlock::nfree() const
    {
        return this->_data.nfree;
    }
    void SuperBlock::nfree(uint32_t nfree)
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
    char SuperBlock::ilock() const
    {
        return this->_data.ilock;
    }
    void SuperBlock::ilock(char ilock)
    {
        this->_data.ilock = ilock;
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