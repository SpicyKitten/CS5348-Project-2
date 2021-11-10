#include "inode.hpp"

namespace ModV6FileSystem
{
    std::ostream &operator<<(std::ostream &ostream, const INode& in)
    {
        return ostream << "INode[flags=" << std::bitset<16>(in.flags())
        << ", size=" << in.size() << "]";
    }
    INode::~INode()
    {
        std::cout << "~INode" << std::endl;
    }
    unsigned short INode::flags() const
    {
        return this->_data.flags;
    }
    void INode::flags(unsigned short flags)
    {
        this->_data.flags = flags;
    }
    unsigned short INode::nlinks() const
    {
        return this->_data.nlinks;
    }
    void INode::nlinks(unsigned short nlinks)
    {
        this->_data.nlinks = nlinks;
    }
    unsigned int INode::uid() const
    {
        return this->_data.uid;
    }
    void INode::uid(unsigned int uid)
    {
        this->_data.uid = uid;
    }
    unsigned int INode::gid() const
    {
        return this->_data.gid;
    }
    void INode::gid(unsigned int gid)
    {
        this->_data.gid = gid;
    }
    unsigned long int INode::size() const
    {
        return (0L | this->_data.size1) << 32 | this->_data.size2;
    }
    void INode::size(unsigned long long size)
    {
        this->_data.size1 = (unsigned int)((size & 0xFFFFFFFF00000000L) >> 32);
        this->_data.size2 = (unsigned int)(size & 0x00000000FFFFFFFFL);
    }
    std::array<unsigned int, 9> INode::addr() const
    {
        return this->_data.addr;
    }
    void INode::addr(std::array<unsigned int, 9> addr)
    {
        std::copy(addr.begin(), addr.end(), this->_data.addr.begin());
    }
    unsigned int INode::actime() const
    {
        return this->_data.actime;
    }
    void INode::actime(unsigned int actime)
    {
        this->_data.actime = actime;
    }
    unsigned int INode::modtime() const
    {
        return this->_data.modtime;
    }
    void INode::modtime(unsigned int modtime)
    {
        this->_data.modtime = modtime;
    }
}