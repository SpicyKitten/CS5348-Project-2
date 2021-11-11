#include "file.hpp"

namespace ModV6FileSystem
{
    std::ostream &operator<<(std::ostream &ostream, const File& in)
    {
        // std::string filename = "";
        // for(auto& chr : in.filename())
        // {
        //     filename = filename + " [" + std::to_string(static_cast<int>(chr)) + "]";
        // }
        auto filename_data = in.filename();
        std::string filename{filename_data.begin(), filename_data.end()};
        return ostream << "File[inode=" << in.inode()
        << ", filename=" << filename << "]";
    }
    uint32_t File::inode() const
    {
        return this->_data.inode;
    }
    void File::inode(uint32_t inode)
    {
        this->_data.inode = inode;
    }
    std::array<char, 28> File::filename() const
    {
        return this->_data.filename;
    }
    void File::filename(std::array<char, 28> filename)
    {
        std::copy(filename.begin(), filename.end(), this->_data.filename.begin());
    }
}