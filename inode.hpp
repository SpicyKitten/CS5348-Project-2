#pragma once
#include <iostream>
#include <tuple>
#include <array>
#include <bitset>

namespace ModV6FileSystem
{
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
        Data _data;

    public:
        friend std::ostream &operator<<(std::ostream &ostream, const INode& in);
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
    };
}