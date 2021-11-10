#pragma once
#include <array>

namespace ModV6FileSystem
{
    struct SuperBlock
    {
    private:
        struct Data
        {
        public:
            size_t isize;
            size_t fsize;
            size_t nfree;
            std::array<unsigned int, 251> free;
            char flock;
            char fmod;
            unsigned int time;
        };
        Data _data;

    public:
        size_t isize() const;
        void isize(size_t isize);
        size_t fsize() const;
        void fsize(size_t fsize);
        size_t nfree() const;
        void nfree(size_t nfree);
        std::array<unsigned int, 251> free() const;
        void free(std::array<unsigned int, 251> free);
        char flock() const;
        void flock(char flock);
        char fmod() const;
        void fmod(char fmod);
        unsigned int time() const;
        void time(unsigned int time);
    };
}