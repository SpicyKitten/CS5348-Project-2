#include "block.hpp"

namespace ModV6FileSystem
{
    Block::Block(int32_t fd, uint32_t blockIdx) : _fd(fd), _blockIdx(blockIdx)
    {
        lseek(this->_fd, 1024 * this->_blockIdx, SEEK_SET);
        read(this->_fd, this->_data.bytes.data(), 1024);
    }
    Block::~Block()
    {
        lseek(this->_fd, 1024 * this->_blockIdx, SEEK_SET);
        write(this->_fd, this->_data.bytes.data(), 1024);
        // std::cout << "~Block[" << this->_blockIdx << "]" << std::endl;
    }
    std::array<uint8_t, 1024>& Block::asBytes() const
    {
        return const_cast<std::array<uint8_t, 1024>&>(this->_data.bytes);
    }
    std::array<uint32_t, 256>& Block::asIntegers() const
    {
        const auto* data_byte_ptr = this->asBytes().data();
        const auto* data_integer_ptr = reinterpret_cast<const std::array<uint32_t, 256>*>(data_byte_ptr);
        return const_cast<std::array<uint32_t, 256>&>(*data_integer_ptr);
    }
    uint32_t Block::index() const
    {
        return this->_blockIdx;
    }
}