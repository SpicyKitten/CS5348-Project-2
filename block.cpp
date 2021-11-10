#include "block.hpp"

namespace ModV6FileSystem
{
    Block::~Block()
    {
        std::cout << "~Block" << std::endl;
    }
    std::array<File, 32>& Block::asFiles() const
    {
        const auto* data_byte_ptr = this->_data.bytes.data();
        const auto* data_file_ptr = reinterpret_cast<const std::array<File, 32>*>(data_byte_ptr);
        return const_cast<std::array<File, 32>&>(*data_file_ptr);
    }
    std::array<INode, 16>& Block::asINodes() const
    {
        const auto* data_byte_ptr = this->asBytes().data();
        const auto* data_inode_ptr = reinterpret_cast<const std::array<INode, 16>*>(data_byte_ptr);
        return const_cast<std::array<INode, 16>&>(*data_inode_ptr);
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
    SuperBlock Block::asSuperBlock() const
    {
        SuperBlock superblock{const_cast<Block*>(this)};
        return superblock;
    }
}