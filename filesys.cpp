#include "filesys.hpp"

namespace ModV6FileSystem
{
    FileSystem::FileSystem() : _fd(-1)
    {
        reset();
    }
    FileSystem::~FileSystem()
    {
        this->_inodes.clear();
        this->_blocks.clear();
        close(this->_fd);
        std::cout << "~FileSystem" << std::endl;
    }
    void FileSystem::reset()
    {
        if(this->_fd != -1)
        {
            close(this->_fd);
        }
        this->_fd = -1;
        this->setDimensions(0, 0);
        this->_inodes.clear();
        this->_blocks.clear();
        std::cout << "FileSystem::reset" << std::endl;
    }
    void FileSystem::setDimensions(uint32_t totalBlocks, uint32_t iNodeBlocks)
    {
        std::cout << "Setting dimensions: [totalBlocks=" << totalBlocks << ", iNodeBlocks="
            << iNodeBlocks << "]" << std::endl;
        this->TOTAL_BLOCKS = totalBlocks;
        this->BOOT_INFO_BLOCKS = 1;
        this->SUPERBLOCK_BLOCKS = 1;
        this->INODE_BLOCKS = iNodeBlocks;
        this->DATA_BLOCKS = TOTAL_BLOCKS - BOOT_INFO_BLOCKS - SUPERBLOCK_BLOCKS - INODE_BLOCKS;
        this->BOOT_INFO_BLOCK_IDX = 0;
        this->SUPERBLOCK_IDX = BOOT_INFO_BLOCK_IDX + BOOT_INFO_BLOCKS;
        this->INODE_BLOCK_IDX = SUPERBLOCK_IDX + SUPERBLOCK_BLOCKS;
        this->DATA_BLOCK_IDX = INODE_BLOCK_IDX + INODE_BLOCKS;
        this->OUT_OF_BOUNDS = DATA_BLOCK_IDX + DATA_BLOCKS;
    }
    std::shared_ptr<Block> FileSystem::getBlock(uint32_t blockIdx)
    {
        if(this->_fd == -1)
        {
            std::cout << "Failed to retrieve block " << blockIdx 
                << " because openfs has not been called successfully" << std::endl;
            return std::shared_ptr<Block>{nullptr};
        }
        if(blockIdx >= this->TOTAL_BLOCKS && blockIdx > 1)
        {
            std::cout << "Failed to retrieve block " << blockIdx << " which is out of bounds" << std::endl;
            return std::shared_ptr<Block>{nullptr};
        }
        for(auto& block_ptr : this->_blocks)
        {
            if(block_ptr->index() == blockIdx)
            {
                return block_ptr;
            }
        }
        std::shared_ptr<Block> block_ptr{new Block(this->_fd, blockIdx)};
        return block_ptr;
    }
    void FileSystem::quit()
    {
        std::cout << "mod-v6 file system shutting down gracefully" << std::endl;
    }
    void FileSystem::initfs(uint32_t totalBlocks, uint32_t iNodeBlocks)
    {
        std::cout << "Executing initfs " << totalBlocks << " " << iNodeBlocks << std::endl;
        // delete the old i-nodes
        this->_inodes.clear();
        // flush blocks
        this->_blocks.clear();
        // total size = totalBlocks * block size = totalBlocks * 1024 bytes
        // i-nodes = 16 i-nodes per block
        // total i-nodes = 16 * iNodeBlocks
        // block structure:
        // ______________________________________________________________________________________
        // | boot info | superblock | ... i-node blocks ... |        ... data blocks ...        |
        // |     0     |     1      | 2 to iNodeBlocks + 1  | iNodeBlocks + 2 to totalBlocks - 1|
        // ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄
        // 1024 bytes per block
        TOTAL_BLOCKS = totalBlocks;
        BOOT_INFO_BLOCKS = 1;
        SUPERBLOCK_BLOCKS = 1;
        INODE_BLOCKS = iNodeBlocks;
        DATA_BLOCKS = TOTAL_BLOCKS - BOOT_INFO_BLOCKS - SUPERBLOCK_BLOCKS - INODE_BLOCKS;
        BOOT_INFO_BLOCK_IDX = 0;
        SUPERBLOCK_IDX = BOOT_INFO_BLOCK_IDX + BOOT_INFO_BLOCKS;
        INODE_BLOCK_IDX = SUPERBLOCK_IDX + SUPERBLOCK_BLOCKS;
        DATA_BLOCK_IDX = INODE_BLOCK_IDX + INODE_BLOCKS;
        OUT_OF_BOUNDS = DATA_BLOCK_IDX + DATA_BLOCKS;
        std::cout << "Total blocks: " << TOTAL_BLOCKS << std::endl;
        std::cout << "End of blocks: " << OUT_OF_BOUNDS << std::endl;
        std::cout << "Data blocks: " << DATA_BLOCKS << std::endl;
        if(BOOT_INFO_BLOCKS + SUPERBLOCK_BLOCKS + INODE_BLOCKS >= TOTAL_BLOCKS ||
            TOTAL_BLOCKS != OUT_OF_BOUNDS)
        {
            throw std::invalid_argument("Invalid overflow given " + std::to_string(TOTAL_BLOCKS)
                + " blocks total and " + std::to_string(INODE_BLOCKS) + " i-node blocks");
        }
        if(this->_fd == -1)
        {
            std::cout << "openfs has not been called successfully, aborting initfs" << std::endl;
            return;
        }
        ftruncate(this->_fd, TOTAL_BLOCKS * 1024);
        this->setDimensions(totalBlocks, iNodeBlocks);
        std::shared_ptr<Block> block_ptr = this->getBlock(1);
        SuperBlock superblock{block_ptr};
        superblock.isize(INODE_BLOCKS);
        superblock.fsize(TOTAL_BLOCKS);
        superblock.nfree(0);
        superblock.flock('\0');
        superblock.ilock('\0');
        superblock.fmod('\0');
        superblock.time(0);
        this->_blocks.push_back(block_ptr);
        std::cout << "Superblock isize: " << superblock.isize() << std::endl;
        this->_inodes.push_back(std::unique_ptr<INode>(new INode()));
    }
    void FileSystem::openfs(const std::string& filename)
    {
        std::cout << "Executing openfs " << filename << std::endl;
        auto existed_before = access(filename.c_str(), F_OK) != -1;
        auto fd = open(filename.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        auto accessible = fd != -1;
        auto exists = access(filename.c_str(), F_OK) != -1;
        if(!exists)
        {
            std::cout << "Did not open FileSystem at " << filename 
                << " since file could not be created" << std::endl;
        }
        else if(!accessible)
        {
            std::cout << "Did not open FileSystem at " << filename 
                << " since file could not be accessed" << std::endl;
        }
        else if(!existed_before)
        {
            reset();
            this->_fd = fd;
        }
        else if(existed_before)
        {
            reset();
            this->_fd = fd;
            std::shared_ptr<Block> block_ptr = this->getBlock(1);
            SuperBlock superblock{block_ptr};
            this->setDimensions(superblock.fsize(), superblock.isize());
        }
        else
        {
            std::cout << "This should be impossible!" << std::endl;
        }
        // open the file otherwise
        
        // this will overwrite the first filename.size() characters in the file
        // write(fd, filename.c_str(), filename.size());
        
        std::cout << "Obtained file descriptor: " << fd << std::endl;
    }
}