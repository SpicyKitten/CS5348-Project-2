#pragma once
#include <string>
#include <iostream>

class FileSystem
{
public:
    void openfs(std::string filename);
    void initfs(unsigned long totalBlocks, unsigned long iNodeBlocks);
};