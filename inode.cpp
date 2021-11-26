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
        // std::cout << "~INode" << std::endl;
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
    bool INode::setsUID() const
    {
        auto flags = this->flags();
        const auto UID_FLAG = 0b0000010000000000;
        return (flags & UID_FLAG) == UID_FLAG;
    }
    void INode::setsUID(bool set)
    {
        auto flags = this->flags();
        const auto UID_FLAG = 0b0000010000000000;
        if(set)
        {
            this->flags(flags | UID_FLAG);
        }
        else
        {
            this->flags(flags & ~UID_FLAG);
        }
    }
    bool INode::setsGID() const
    {
        auto flags = this->flags();
        const auto GID_FLAG = 0b0000001000000000;
        return (flags & GID_FLAG) == GID_FLAG;
    }
    void INode::setsGID(bool set)
    {
        auto flags = this->flags();
        const auto GID_FLAG = 0b0000001000000000;
        if(set)
        {
            this->flags(flags | GID_FLAG);
        }
        else
        {
            this->flags(flags & ~GID_FLAG);
        }
    }
    bool INode::ownerR() const
    {
        auto flags = this->flags();
        const auto OWNER_R_FLAG = 0b0000000100000000;
        return (flags & OWNER_R_FLAG) == OWNER_R_FLAG;
    }
    void INode::ownerR(bool set)
    {
        auto flags = this->flags();
        const auto OWNER_R_FLAG = 0b0000000100000000;
        if(set)
        {
            this->flags(flags | OWNER_R_FLAG);
        }
        else
        {
            this->flags(flags & ~OWNER_R_FLAG);
        }
    }
    bool INode::ownerW() const
    {
        auto flags = this->flags();
        const auto OWNER_W_FLAG = 0b0000000010000000;
        return (flags & OWNER_W_FLAG) == OWNER_W_FLAG;
    }
    void INode::ownerW(bool set)
    {
        auto flags = this->flags();
        const auto OWNER_W_FLAG = 0b0000000010000000;
        if(set)
        {
            this->flags(flags | OWNER_W_FLAG);
        }
        else
        {
            this->flags(flags & ~OWNER_W_FLAG);
        }
    }
    bool INode::ownerX() const
    {
        auto flags = this->flags();
        const auto OWNER_X_FLAG = 0b0000000001000000;
        return (flags & OWNER_X_FLAG) == OWNER_X_FLAG;
    }
    void INode::ownerX(bool set)
    {
        auto flags = this->flags();
        const auto OWNER_X_FLAG = 0b0000000001000000;
        if(set)
        {
            this->flags(flags | OWNER_X_FLAG);
        }
        else
        {
            this->flags(flags & ~OWNER_X_FLAG);
        }
    }
    bool INode::groupR() const
    {
        auto flags = this->flags();
        const auto GROUP_R_FLAG = 0b0000000000100000;
        return (flags & GROUP_R_FLAG) == GROUP_R_FLAG;
    }
    void INode::groupR(bool set)
    {
        auto flags = this->flags();
        const auto GROUP_R_FLAG = 0b0000000000100000;
        if(set)
        {
            this->flags(flags | GROUP_R_FLAG);
        }
        else
        {
            this->flags(flags & ~GROUP_R_FLAG);
        }
    }
    bool INode::groupW() const
    {
        auto flags = this->flags();
        const auto GROUP_W_FLAG = 0b0000000000010000;
        return (flags & GROUP_W_FLAG) == GROUP_W_FLAG;
    }
    void INode::groupW(bool set)
    {
        auto flags = this->flags();
        const auto GROUP_W_FLAG = 0b0000000000010000;
        if(set)
        {
            this->flags(flags | GROUP_W_FLAG);
        }
        else
        {
            this->flags(flags & ~GROUP_W_FLAG);
        }
    }
    bool INode::groupX() const
    {
        auto flags = this->flags();
        const auto GROUP_X_FLAG = 0b0000000000001000;
        return (flags & GROUP_X_FLAG) == GROUP_X_FLAG;
    }
    void INode::groupX(bool set)
    {
        auto flags = this->flags();
        const auto GROUP_X_FLAG = 0b0000000000001000;
        if(set)
        {
            this->flags(flags | GROUP_X_FLAG);
        }
        else
        {
            this->flags(flags & ~GROUP_X_FLAG);
        }
    }
    bool INode::worldR() const
    {
        auto flags = this->flags();
        const auto WORLD_R_FLAG = 0b0000000000000100;
        return (flags & WORLD_R_FLAG) == WORLD_R_FLAG;
    }
    void INode::worldR(bool set)
    {
        auto flags = this->flags();
        const auto WORLD_R_FLAG = 0b0000000000000100;
        if(set)
        {
            this->flags(flags | WORLD_R_FLAG);
        }
        else
        {
            this->flags(flags & ~WORLD_R_FLAG);
        }
    }
    bool INode::worldW() const
    {
        auto flags = this->flags();
        const auto WORLD_W_FLAG = 0b0000000000000010;
        return (flags & WORLD_W_FLAG) == WORLD_W_FLAG;
    }
    void INode::worldW(bool set)
    {
        auto flags = this->flags();
        const auto WORLD_W_FLAG = 0b0000000000000010;
        if(set)
        {
            this->flags(flags | WORLD_W_FLAG);
        }
        else
        {
            this->flags(flags & ~WORLD_W_FLAG);
        }
    }
    bool INode::worldX() const
    {
        auto flags = this->flags();
        const auto WORLD_X_FLAG = 0b0000000000000001;
        return (flags & WORLD_X_FLAG) == WORLD_X_FLAG;
    }
    void INode::worldX(bool set)
    {
        auto flags = this->flags();
        const auto WORLD_X_FLAG = 0b0000000000000001;
        if(set)
        {
            this->flags(flags | WORLD_X_FLAG);
        }
        else
        {
            this->flags(flags & ~WORLD_X_FLAG);
        }
    }
}