#include "filesys.hpp"

namespace ModV6FileSystem
{
    FileSystem::FileSystem() : _fd(-1)
    {
        reset();
    }
    FileSystem::~FileSystem()
    {
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
        // std::vector<std::weak_ptr<Block>> iterator
        auto iter = this->_blocks.begin();
        while(iter != this->_blocks.end())
        {
            if(iter->expired())
            {
                iter = this->_blocks.erase(iter);
            }
            else
            {
                if(auto block_ptr = iter->lock())
                {
                    if(block_ptr->index() == blockIdx)
                    {
                        return block_ptr;
                    }
                }
                else
                {
                    std::cout << "Found expired weak pointer in block tracker!" << std::endl;
                }
                // move iterator
                ++iter;
            }
        }
        std::shared_ptr<Block> block_ptr{new Block(this->_fd, blockIdx)};
        std::weak_ptr<Block> weak_block_ptr = block_ptr;
        this->_blocks.push_back(weak_block_ptr);
        return block_ptr;
    }
    std::shared_ptr<INode> FileSystem::getINode(uint32_t iNodeIdx)
    {
        const auto INODES_PER_BLOCK = 1024 / 64;
        if(iNodeIdx < 0 || iNodeIdx >= this->INODE_BLOCKS * INODES_PER_BLOCK)
        {
            throw std::invalid_argument("Failed to retrieve INode " + std::to_string(iNodeIdx)
                + " which is out of bounds");
        }
        auto blockIdx = this->INODE_BLOCK_IDX + (iNodeIdx / INODES_PER_BLOCK);
        auto offset = iNodeIdx % INODES_PER_BLOCK;
        std::cout << "Fetching INode " << iNodeIdx << " from spot " << offset 
            << " of block " << blockIdx << std::endl;
        std::shared_ptr<Block> block_ptr = this->getBlock(blockIdx);
        std::shared_ptr<INode> inode_ptr{new INode(block_ptr, offset)};
        return inode_ptr;
    }
    std::array<std::shared_ptr<File>, 32> FileSystem::getFiles(uint32_t blockIdx)
    {
        std::shared_ptr<Block> block_ptr = this->getBlock(blockIdx);
        std::vector<std::shared_ptr<File>> file_vector;
        for(auto fileIdx = 0; fileIdx < 32; ++fileIdx)
        {
            std::shared_ptr<File> file_ptr{new File(block_ptr, fileIdx)};
            file_vector.push_back(file_ptr);
        }
        std::array<std::shared_ptr<File>, 32> file_array;
        std::copy_n(file_vector.begin(), 32, file_array.begin());
        return file_array;
    }
    std::shared_ptr<SuperBlock> FileSystem::getSuperBlock()
    {
        auto block_ptr = this->getBlock(1);
        std::shared_ptr<SuperBlock> superblock_ptr{new SuperBlock(block_ptr)};
        return superblock_ptr;
    }
    void FileSystem::freeDataBlock(std::shared_ptr<SuperBlock> superblock_ptr, uint32_t blockIdx)
    {
        if(blockIdx < this->DATA_BLOCK_IDX || blockIdx >= this->DATA_BLOCK_IDX + this->DATA_BLOCKS)
        {
            throw std::invalid_argument("Cannot free block " + std::to_string(blockIdx)
                + " as it is not a data block");
        }
        std::cout << "Free block " << blockIdx << std::endl;
        auto freeArray = superblock_ptr->free();
        auto nfree = superblock_ptr->nfree();
        superblock_ptr->nfree(++nfree);
        if(nfree < 251)
        {
            freeArray[nfree] = blockIdx;
        }
        else
        {
            std::shared_ptr<Block> block_ptr = this->getBlock(blockIdx);
            std::array<uint32_t, 256>& intArray = block_ptr->asIntegers();
            for(auto i = 0; i < 251; i++)
            {
                intArray[i] = freeArray[i];
            }
            std::fill(freeArray.begin(), freeArray.end(), 0);
            freeArray[0] = blockIdx;
            superblock_ptr->nfree(0);
        }
        superblock_ptr->free(freeArray);
    }
    /**
     * Updates the superblock with the allocated data block.
     * It will be necessary to call FileSystem::getBlock(uint32_t blockIdx) to get the actual block.
     */
    uint32_t FileSystem::allocateDataBlock(std::shared_ptr<SuperBlock> superblock_ptr)
    {
        auto freeArray = superblock_ptr->free();
        auto nfree = superblock_ptr->nfree();
        if(nfree != 0)
        {
            auto blockIdx = freeArray[nfree];
            freeArray[nfree] = 0;
            superblock_ptr->free(freeArray);
            superblock_ptr->nfree(--nfree);
            return blockIdx;
        }
        else
        {
            auto nextDataBlockIdx = freeArray[nfree];
            if(nextDataBlockIdx == 0)
            {
                throw std::runtime_error("Out of memory: cannot allocate any more data blocks!");
            }
            else
            {
                std::shared_ptr<Block> block_ptr = this->getBlock(nextDataBlockIdx);
                std::array<uint32_t, 256>& intArray = block_ptr->asIntegers();
                for(auto i = 0; i < 251; i++)
                {
                    freeArray[i] = intArray[i];
                }
                std::fill(intArray.begin(), intArray.end(), 0);
                superblock_ptr->nfree(251 - 1);
                superblock_ptr->free(freeArray);
                return nextDataBlockIdx;
            }
        }
    }
    uint32_t FileSystem::allocateINode()
    {
        const auto INODES_PER_BLOCK = 1024 / 64;
        const auto ALLOCATED_FLAG = 0b1000000000000000;
        const auto TOTAL_INODES = this->INODE_BLOCKS * INODES_PER_BLOCK;
        for(uint32_t idx = 0; idx < TOTAL_INODES; idx++)
        {
            std::shared_ptr<INode> inode_ptr = this->getINode(idx);
            auto flags = inode_ptr->flags();
            if((flags & ALLOCATED_FLAG) == 0)
            {
                inode_ptr->flags(flags | ALLOCATED_FLAG);
                return idx;
            }
        }
        throw std::runtime_error("Out of memory: cannot allocate any more data blocks!");
    }
    void FileSystem::initializeFreeList(std::shared_ptr<SuperBlock> superblock_ptr)
    {
        std::cout << "Number of data blocks: " << this->DATA_BLOCKS << std::endl;
        std::cout << "Data blocks start at: " << this->DATA_BLOCK_IDX << std::endl;
        for(int32_t i = DATA_BLOCKS - 1; i >= 0; i--)
        {
            freeDataBlock(superblock_ptr, DATA_BLOCK_IDX + i);
        }
    }
    void FileSystem::initializeINodes()
    {
        const auto INODES_PER_BLOCK = 1024 / 64;
        for(uint32_t idx = 0; idx < this->INODE_BLOCKS * INODES_PER_BLOCK; idx++)
        {
            std::shared_ptr<INode> inode_ptr = this->getINode(idx);
            inode_ptr->flags(0);
        }
    }
    void FileSystem::initializeRoot()
    {
        auto iNodeIdx = this->allocateINode();
        std::shared_ptr<INode> inode_ptr = this->getINode(iNodeIdx);
        std::cout << "Allocated INode " << iNodeIdx << " for root" << std::endl;
        std::cout << "INode " << iNodeIdx << ": " << *inode_ptr << std::endl;
        // 1000000000000000 allocated flag
        // 0110000000000000 file type flag
        // 0001100000000000 file size flag
        // 0000010000000000 set uid on execution flag
        // 0000001000000000 set gid on execution flag
        // 0000000111000000 rwx owner permissions flag
        // 0000000000111000 rwx group permissions flag
        // 0000000000000111 rxw world permissions flag
        // 1                allocated
        //  10              directory
        //    00            small file
        //      0           don't set uid on execution
        //       0          don't set gid on execution
        //        110       owner can read and write, but not execute
        //           000    group has no permissions
        //              000 world has no permissions
        inode_ptr->flags(0b1100000110000000);
        // the links are / . and .. 
        inode_ptr->nlinks(3);
        inode_ptr->uid(0);
        inode_ptr->gid(0);
        // 2 * file size
        // We have 1 for . and 1 for ..
        inode_ptr->size(2 * 32);
        // inode.addr();
        inode_ptr->actime(0);
        inode_ptr->modtime(0);
        std::cout << "INode " << iNodeIdx << ": " << *inode_ptr << std::endl;
        
        std::shared_ptr<SuperBlock> superblock_ptr = this->getSuperBlock();
        auto blockIdx = this->allocateDataBlock(superblock_ptr);
        std::array<std::shared_ptr<File>, 32> file_ptrs = this->getFiles(blockIdx);
        std::shared_ptr<File> self = file_ptrs[0];
        self->inode(iNodeIdx);
        self->filename(this->filenameToArray("."));
        std::shared_ptr<File> parent = file_ptrs[1];
        parent->inode(iNodeIdx);
        parent->filename(this->filenameToArray(".."));
        std::cout << "Allocated data block: " << blockIdx << std::endl;
        for(std::shared_ptr<File> file_ptr : file_ptrs)
        {
            std::cout << "File for INode block: " << *file_ptr << std::endl;
        }
    }
    std::array<char, 28> FileSystem::filenameToArray(std::string filename)
    {
        if(filename.size() > 28 || filename.size() < 1)
        {
            throw std::invalid_argument("Filename \"" + filename + "\" is too short/long!");
        }
        std::array<char, 28> array;
        std::fill(array.begin(), array.end(), 0);
        std::copy(filename.begin(), filename.end(), array.begin());
        return array;
    }
    void FileSystem::quit()
    {
        std::cout << "mod-v6 file system shutting down gracefully" << std::endl;
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
            std::shared_ptr<SuperBlock> superblock_ptr = this->getSuperBlock();
            this->setDimensions(superblock_ptr->fsize(), superblock_ptr->isize());
        }
        else
        {
            std::cout << "This should be impossible!" << std::endl;
        }
        // this will overwrite the first filename.size() characters in the file
        // write(fd, filename.c_str(), filename.size());
        
        std::cout << "Obtained file descriptor: " << fd << std::endl;
    }
    void FileSystem::initfs(uint32_t totalBlocks, uint32_t iNodeBlocks)
    {
        std::cout << "Executing initfs " << totalBlocks << " " << iNodeBlocks << std::endl;
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
        std::shared_ptr<SuperBlock> superblock_ptr = this->getSuperBlock();
        superblock_ptr->isize(INODE_BLOCKS);
        superblock_ptr->fsize(TOTAL_BLOCKS);
        superblock_ptr->nfree(0);
        superblock_ptr->flock('\0');
        superblock_ptr->ilock('\0');
        superblock_ptr->fmod('\0');
        superblock_ptr->time(0);
        this->initializeFreeList(superblock_ptr);
        superblock_ptr.reset();
        this->initializeINodes();
        this->initializeRoot();
        this->_blocks.clear();
    }
    void FileSystem::cpin(const std::string& outerFilename, const std::string& innerFilename)
    {
        std::cout << "Executing cpin " << outerFilename << " " << innerFilename << std::endl;
    }
    void FileSystem::cpout(const std::string& innerFilename, const std::string& outerFilename)
    {
        std::cout << "Executing cpout " << innerFilename << " " << outerFilename << std::endl;
    }
    void FileSystem::rm(const std::string& innerFilename)
    {
        std::cout << "Executing rm " << innerFilename << std::endl;
    }
    void FileSystem::mkdir(const std::string& innerFilename)
    {
        std::cout << "Executing mkdir " << innerFilename << std::endl;
    }
    void FileSystem::cd(const std::string& innerFilename)
    {
        std::cout << "Executing cd " << innerFilename << std::endl;
    }
}