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
            unsigned short flags;
            unsigned short nlinks;
            unsigned int uid;
            unsigned int gid;
            // assuming this is earlier bytes
            unsigned int size1;
            // assuming this is later bytes
            unsigned int size2;
            std::array<unsigned int, 9> addr;
            unsigned int actime;
            unsigned int modtime;
        };
        Data _data;

    public:
        friend std::ostream &operator<<(std::ostream &ostream, const INode& in);
        ~INode();
        unsigned short flags() const;
        void flags(unsigned short flags);
        unsigned short nlinks() const;
        void nlinks(unsigned short nlinks);
        unsigned int uid() const;
        void uid(unsigned int uid);
        unsigned int gid() const;
        void gid(unsigned int gid);
        unsigned long int size() const;
        void size(unsigned long long size);
        std::array<unsigned int, 9> addr() const;
        void addr(std::array<unsigned int, 9> addr);
        unsigned int actime() const;
        void actime(unsigned int actime);
        unsigned int modtime() const;
        void modtime(unsigned int modtime);
    };
}