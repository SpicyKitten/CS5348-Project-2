#pragma once
#include <array>
#include <memory>
#include <string>
#include "block.hpp"

namespace ModV6FileSystem
{
    struct Block;

    struct SuperBlock
    {
    private:
        struct Data
        {
        public:
            uint32_t isize;
            uint32_t fsize;
            uint32_t nfree;
            std::array<unsigned int, 251> free;
            char flock;
            char ilock;
            char fmod;
            unsigned int time;
        };
        Data& _data;
        std::shared_ptr<Block> _block;

    public:
        friend std::ostream &operator<<(std::ostream &ostream, const SuperBlock& in);
        SuperBlock(std::shared_ptr<Block> block);
        ~SuperBlock();
        
        uint32_t isize() const;
        void isize(uint32_t isize);
        uint32_t fsize() const;
        void fsize(uint32_t fsize);
        uint32_t nfree() const;
        void nfree(uint32_t nfree);
        std::array<unsigned int, 251> free() const;
        void free(std::array<unsigned int, 251> free);
        char flock() const;
        void flock(char flock);
        char ilock() const;
        void ilock(char ilock);
        char fmod() const;
        void fmod(char fmod);
        unsigned int time() const;
        void time(unsigned int time);
    };
}