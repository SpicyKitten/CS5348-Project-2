#include "inode.hpp"

namespace ModV6FileSystem
{
    std::ostream &operator<<(std::ostream &ostream, const INode& in)
    {
        return ostream << "INode[flags=" << std::bitset<16>(in.flags())
        << ", nlinks=" << std::bitset<16>(in.nlinks()) << ", size=" << in.size() << "]";
    }
    INode::INode(std::shared_ptr<Block> block_ptr, uint32_t offset) : 
        _data(
            const_cast<std::array<Data, 16>&>(
                *reinterpret_cast<const std::array<Data, 16>*>(
                    block_ptr->asBytes().data()
                )
            )[offset]
        ),
        _block(block_ptr)
    {
    }
    INode::~INode()
    {
        std::cout << "~INode" << std::endl;
    }
    uint16_t INode::flags() const
    {
        return this->_data.flags;
    }
    void INode::flags(uint16_t flags)
    {
        this->_data.flags = flags;
    }
    uint16_t INode::nlinks() const
    {
        return this->_data.nlinks;
    }
    void INode::nlinks(uint16_t nlinks)
    {
        this->_data.nlinks = nlinks;
    }
    uint32_t INode::uid() const
    {
        return this->_data.uid;
    }
    void INode::uid(uint32_t uid)
    {
        this->_data.uid = uid;
    }
    uint32_t INode::gid() const
    {
        return this->_data.gid;
    }
    void INode::gid(uint32_t gid)
    {
        this->_data.gid = gid;
    }
    uint64_t INode::size() const
    {
        return (0L | this->_data.size1) << 32 | this->_data.size2;
    }
    void INode::size(uint64_t size)
    {
        this->_data.size1 = (uint32_t)((size & 0xFFFFFFFF00000000ull) >> 32);
        this->_data.size2 = (uint32_t)(size & 0x00000000FFFFFFFFull);
    }
    std::array<uint32_t, 9> INode::addr() const
    {
        return this->_data.addr;
    }
    void INode::addr(std::array<uint32_t, 9> addr)
    {
        std::copy(addr.begin(), addr.end(), this->_data.addr.begin());
    }
    uint32_t INode::actime() const
    {
        return this->_data.actime;
    }
    void INode::actime(uint32_t actime)
    {
        this->_data.actime = actime;
    }
    uint32_t INode::modtime() const
    {
        return this->_data.modtime;
    }
    void INode::modtime(uint32_t modtime)
    {
        this->_data.modtime = modtime;
    }
}