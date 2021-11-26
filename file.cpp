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
    File::File(std::shared_ptr<Block> block_ptr, uint32_t offset) : 
        _data(
            const_cast<std::array<Data, 32>&>(
                *reinterpret_cast<const std::array<Data, 32>*>(
                    block_ptr->asBytes().data()
                )
            )[offset]
        ),
        _block(block_ptr)
    {
    }
    File::~File()
    {
        // std::cout << "~File" << std::endl;
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
    std::string File::name() const
    {
        auto filename_chars = this->filename();
        std::string filename{filename_chars.begin(), filename_chars.end()};
        filename.erase(filename.find('\0'));
        return filename;
    }
    void File::name(std::string filename)
    {
        if(filename.size() > 28 || filename.size() < 1)
        {
            throw std::invalid_argument("Filename \"" + filename + "\" is too short/long!");
        }
        std::array<char, 28> array;
        std::fill(array.begin(), array.end(), 0);
        std::copy(filename.begin(), filename.end(), array.begin());
        this->filename(array);
    }
}