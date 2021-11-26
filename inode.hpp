#pragma once
#include <iostream>
#include <tuple>
#include <array>
#include <bitset>
#include <memory>
#include <stdexcept>
#include <string>
#include "block.hpp"
#include "filetype.hpp"
#include "filetype.hpp"
#include "filesize.hpp"

namespace ModV6FileSystem
{
    struct Block;

    struct INode
    {
    private:
        struct Data
        {
        public:
            uint16_t flags;
            uint16_t nlinks;
            uint32_t uid;
            uint32_t gid;
            // assuming this is earlier bytes
            uint32_t size1;
            // assuming this is later bytes
            uint32_t size2;
            std::array<uint32_t, 9> addr;
            uint32_t actime;
            uint32_t modtime;
        };
        Data& _data;
        std::shared_ptr<Block> _block;

    public:
        friend std::ostream &operator<<(std::ostream &ostream, const INode& in);
        INode(std::shared_ptr<Block> block_ptr, uint32_t offset);
        ~INode();

        uint16_t flags() const;
        void flags(uint16_t flags);
        uint16_t nlinks() const;
        void nlinks(uint16_t nlinks);
        uint32_t uid() const;
        void uid(uint32_t uid);
        uint32_t gid() const;
        void gid(uint32_t gid);
        uint64_t size() const;
        void size(uint64_t size);
        std::array<uint32_t, 9> addr() const;
        void addr(std::array<uint32_t, 9> addr);
        uint32_t actime() const;
        void actime(uint32_t actime);
        uint32_t modtime() const;
        void modtime(uint32_t modtime);
        // methods for flags
        // the name "set" here indicates that most of the time we just want to set/unset these bits
        bool allocated() const;
        void allocated(bool allocated);
        FileType filetype() const;
        void filetype(FileType filetype);
        FileSize filesize() const;
        void filesize(FileSize filesize);
        bool setsUID() const;
        void setsUID(bool set);
        bool setsGID() const;
        void setsGID(bool set);
        bool ownerR() const;
        void ownerR(bool set);
        bool ownerW() const;
        void ownerW(bool set);
        bool ownerX() const;
        void ownerX(bool set);
        bool groupR() const;
        void groupR(bool set);
        bool groupW() const;
        void groupW(bool set);
        bool groupX() const;
        void groupX(bool set);
        bool worldR() const;
        void worldR(bool set);
        bool worldW() const;
        void worldW(bool set);
        bool worldX() const;
        void worldX(bool set);
    };
}