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
        this->_blocks.push_back(block_ptr);
        return block_ptr;
    }
    std::tuple<std::shared_ptr<Block>, INode&> FileSystem::getINode(uint32_t iNodeIdx)
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
        std::array<INode, 16>& iNodeArray = block_ptr->asINodes();
        return std::make_tuple(block_ptr, std::ref(iNodeArray[offset]));
    }
    void FileSystem::freeDataBlock(SuperBlock& superblock, uint32_t blockIdx)
    {
        if(blockIdx < this->DATA_BLOCK_IDX || blockIdx >= this->DATA_BLOCK_IDX + this->DATA_BLOCKS)
        {
            throw std::invalid_argument("Cannot free block " + std::to_string(blockIdx)
                + " as it is not a data block");
        }
        std::cout << "Free block " << blockIdx << std::endl;
        auto freeArray = superblock.free();
        auto nfree = superblock.nfree();
        superblock.nfree(++nfree);
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
            superblock.nfree(0);
        }
        superblock.free(freeArray);
    }
    /**
     * Updates the superblock with the allocated data block.
     * It will be necessary to call FileSystem::getBlock(uint32_t blockIdx) to get the actual block.
     */
    uint32_t FileSystem::allocateDataBlock(SuperBlock& superblock)
    {
        auto freeArray = superblock.free();
        auto nfree = superblock.nfree();
        if(nfree != 0)
        {
            auto blockIdx = freeArray[nfree];
            freeArray[nfree] = 0;
            superblock.free(freeArray);
            superblock.nfree(--nfree);
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
                superblock.nfree(251 - 1);
                superblock.free(freeArray);
                return nextDataBlockIdx;
            }
        }
    }
    uint32_t FileSystem::allocateINode()
    {
        const auto INODES_PER_BLOCK = 1024 / 64;
        const auto ALLOCATED_FLAG = 0b1000000000000000;
        for(uint32_t idx = 0; idx < this->INODE_BLOCKS * INODES_PER_BLOCK; idx++)
        {
            std::tuple<std::shared_ptr<Block>, INode&> tuple = this->getINode(idx);
            auto flags = std::get<1>(tuple).flags();
            if((flags & ALLOCATED_FLAG) == 0)
            {
                std::get<1>(tuple).flags(flags | ALLOCATED_FLAG);
                return idx;
            }
        }
        throw std::runtime_error("Out of memory: cannot allocate any more data blocks!");
    }
    void FileSystem::initializeFreeList(SuperBlock& superblock)
    {
        std::cout << "Number of data blocks: " << this->DATA_BLOCKS << std::endl;
        std::cout << "Data blocks start at: " << this->DATA_BLOCK_IDX << std::endl;
        for(int32_t i = DATA_BLOCKS - 1; i >= 0; i--)
        {
            freeDataBlock(superblock, DATA_BLOCK_IDX + i);
        }
    }
    void FileSystem::initializeINodes()
    {
        const auto INODES_PER_BLOCK = 1024 / 64;
        for(uint32_t idx = 0; idx < this->INODE_BLOCKS * INODES_PER_BLOCK; idx++)
        {
            std::tuple<std::shared_ptr<Block>, INode&> tuple = this->getINode(idx);
            std::get<1>(tuple).flags(0);
        }
    }
    void FileSystem::initializeRoot()
    {
        auto iNodeIdx = this->allocateINode();
        std::tuple<std::shared_ptr<Block>, INode&> tuple = this->getINode(iNodeIdx);
        std::cout << "Allocated INode " << iNodeIdx << " for root" << std::endl;
        std::cout << "INode " << iNodeIdx << ": " << std::get<1>(tuple) << std::endl;
        auto& inode = std::get<1>(tuple);
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
        inode.flags(0b1100000110000000);
        // the links are / . and .. 
        inode.nlinks(3);
        inode.uid(0);
        inode.gid(0);
        // 2 * file size
        // We have 1 for . and 1 for ..
        inode.size(2 * 32);
        // inode.addr();
        inode.actime(0);
        inode.modtime(0);
        std::cout << "INode " << iNodeIdx << ": " << std::get<1>(tuple) << std::endl;
        
        std::shared_ptr<Block> superblock_ptr = this->getBlock(1);
        SuperBlock superblock{superblock_ptr};
        auto blockIdx = this->allocateDataBlock(superblock);
        std::shared_ptr<Block> block_ptr = this->getBlock(blockIdx);
        std::array<File, 32>& fileArray = block_ptr->asFiles();
        File& self = fileArray[0];
        self.inode(iNodeIdx);
        self.filename(this->filenameToArray("."));
        File& parent = fileArray[1];
        parent.inode(iNodeIdx);
        parent.filename(this->filenameToArray(".."));
        std::cout << "Allocated data block: " << blockIdx << std::endl;
        for(auto& file : fileArray)
        {
            std::cout << "File for INode block: " << file << std::endl;
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
        this->initializeFreeList(superblock);
        this->initializeINodes();
        this->initializeRoot();
        this->_blocks.clear();
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