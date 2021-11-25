#pragma once

namespace ModV6FileSystem
{
    enum class FileSize : uint16_t
    {
        SMALL=0b00, MEDIUM=0b01, LARGE=0b10, MASSIVE=0b11
    };
}