#include "inode.hpp"

namespace ModV6FileSystem
{
    std::ostream &operator<<(std::ostream &ostream, const INode& in)
    {
        auto filetype = "unknown";
        switch(in.filetype())
        {
            case FileType::BLOCK_SPECIAL:
                filetype = "block_special";
                break;
            case FileType::CHAR_SPECIAL:
                filetype = "char_special";
                break;
            case FileType::DIRECTORY:
                filetype = "directory";
                break;
            case FileType::REGULAR:
                filetype = "regular";
                break;
        }
        auto filesize = "unknown";
        switch(in.filesize())
        {
            case FileSize::SMALL:
                filesize = "small";
                break;
            case FileSize::MEDIUM:
                filesize = "medium";
                break;
            case FileSize::LARGE:
                filesize = "large";
                break;
            case FileSize::MASSIVE:
                filesize = "massive";
                break;
        }
        std::string addresses = "[";
        auto prefix = "";
        for(auto address : in.addr())
        {
            addresses += prefix + std::to_string(address);
            prefix = ", ";
        }
        addresses += "]";
        return ostream << "INode[flags=" << std::bitset<16>(in.flags())
        << ", allocated=" << in.allocated()
        << ", filetype=" << filetype
        << ", filesize=" << filesize
        << ", addr=" << addresses
        << ", size=" << in.size() << "]";
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
    bool INode::allocated() const
    {
        auto flags = this->flags();
        const auto ALLOCATED_FLAG = 0b1000000000000000;
        return (flags & ALLOCATED_FLAG) == ALLOCATED_FLAG;
    }
    void INode::allocated(bool allocated)
    {
        auto flags = this->flags();
        const auto ALLOCATED_FLAG = 0b1000000000000000;
        if(allocated)
        {
            this->flags(flags | ALLOCATED_FLAG);
        }
        else
        {
            this->flags(flags & ~ALLOCATED_FLAG);
        }
    }
    FileType INode::filetype() const
    {
        const std::array<FileType,4> filetypes = {
            FileType::BLOCK_SPECIAL, FileType::CHAR_SPECIAL, FileType::DIRECTORY, FileType::REGULAR
        };
        auto flags = this->flags();
        const auto FILETYPE_FLAG = 0b0110000000000000;
        auto type = (flags & FILETYPE_FLAG) >> 13;
        for(auto filetype : filetypes)
        {
            if(type == static_cast<uint16_t>(filetype))
            {
                return filetype;
            }
        }
        throw std::runtime_error("Unexpected file type: " + std::to_string(type));
    }
    void INode::filetype(FileType filetype)
    {
        auto flags = this->flags();
        const auto FILETYPE_FLAG = 0b0110000000000000;
        this->flags(flags & ~FILETYPE_FLAG);
        flags = this->flags();
        this->flags(flags | (static_cast<uint16_t>(filetype) << 13));
    }
    FileSize INode::filesize() const
    {
        const std::array<FileSize, 4> filesizes = {
            FileSize::SMALL, FileSize::MEDIUM, FileSize::LARGE, FileSize::MASSIVE
        };
        auto flags = this->flags();
        const auto FILESIZE_FLAG = 0b0001100000000000;
        auto size = (flags & FILESIZE_FLAG) >> 11;
        for(auto filesize : filesizes)
        {
            if(size == static_cast<uint16_t>(filesize))
            {
                return filesize;
            }
        }
        throw std::runtime_error("Unexpected file size: " + std::to_string(size));
    }
    void INode::filesize(FileSize filesize)
    {
        auto flags = this->flags();
        const auto FILESIZE_FLAG = 0b0001100000000000;
        this->flags(flags & ~FILESIZE_FLAG);
        flags = this->flags();
        this->flags(flags | (static_cast<uint16_t>(filesize) << 11));
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