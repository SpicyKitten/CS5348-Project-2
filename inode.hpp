#pragma once
#include <iostream>
#include <tuple>

struct INode
{
private:
    unsigned short _flags;
    unsigned short _nlinks;
    unsigned int _uid;
    unsigned int _gid;
    // assuming this is earlier bytes
    unsigned int _size1;
    // assuming this is later bytes
    unsigned int _size2;
    std::array<unsigned int, 9> _addr;
    unsigned int _actime;
    unsigned int _modtime;
public:
    friend std::ostream& operator<<(std::ostream &ostream, const INode &in);
    unsigned short flags();
    void flags(unsigned short flags);
    unsigned short nlinks();
    void nlinks(unsigned short nlinks);
    unsigned int uid();
    void uid(unsigned int uid);
    unsigned int gid();
    void gid(unsigned int gid);
    unsigned long int size();
    void size(unsigned long int size);
    std::array<unsigned int, 9> addr();
    void addr(std::array<unsigned int, 9> addr);
    unsigned int actime();
    void actime(unsigned int actime);
    unsigned int modtime();
    void modtime(unsigned int modtime);
};