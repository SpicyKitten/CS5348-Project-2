#pragma once
#include <array>

namespace ModV6FileSystem
{
    struct SuperBlock
    {
    private:
        size_t _isize;
        size_t _fsize;
        size_t _nfree;
        std::array<unsigned int, 251> _free;
        char _flock;
        char _fmod;
        unsigned int _time;

    public:
        size_t isize();
        void isize(size_t isize);
        size_t fsize();
        void fsize(size_t fsize);
        size_t nfree();
        void nfree(size_t nfree);
        std::array<unsigned int, 251> free();
        void free(std::array<unsigned int, 251> free);
        char flock();
        void flock(char flock);
        char fmod();
        void fmod(char fmod);
        unsigned int time();
        void time(unsigned int time);
    };
}