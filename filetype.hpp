#pragma once
#include <stdexcept>

namespace ModV6FileSystem
{
    enum class FileType : uint16_t
    {
        BLOCK_SPECIAL=0b11, CHAR_SPECIAL=0b01, DIRECTORY=0b10, REGULAR=0b00
    };
}