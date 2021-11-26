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
        this->_working_directory = "/";
        this->setDimensions(0, 0);
        this->_blocks.clear();
        std::cout << "FileSystem::reset" << std::endl;
    }
    void FileSystem::setDimensions(uint32_t totalBlocks, uint32_t inodeBlocks)
    {
        std::cout << "Setting dimensions: [totalBlocks=" << totalBlocks << ", inodeBlocks="
            << inodeBlocks << "]" << std::endl;
        this->TOTAL_BLOCKS = totalBlocks;
        this->BOOT_INFO_BLOCKS = 1;
        this->SUPERBLOCK_BLOCKS = 1;
        this->INODE_BLOCKS = inodeBlocks;
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
            throw std::runtime_error("Failed to retrieve block " + std::to_string(blockIdx) + " which is out of bounds");
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
    std::shared_ptr<INode> FileSystem::getINode(uint32_t inodeIdx)
    {
        const auto INODES_PER_BLOCK = 1024 / 64;
        if(inodeIdx < 0 || inodeIdx >= this->INODE_BLOCKS * INODES_PER_BLOCK)
        {
            throw std::invalid_argument("Failed to retrieve i-node " + std::to_string(inodeIdx)
                + " which is out of bounds");
        }
        auto blockIdx = this->INODE_BLOCK_IDX + (inodeIdx / INODES_PER_BLOCK);
        auto offset = inodeIdx % INODES_PER_BLOCK;
        // std::cout << "Fetching i-node " << inodeIdx << " from spot " << offset 
        //     << " of block " << blockIdx << std::endl;
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
        std::shared_ptr<Block> block_ptr = this->getBlock(blockIdx);
        std::array<uint32_t, 256>& intArray = block_ptr->asIntegers();
        std::fill(intArray.begin(), intArray.end(), 0);
        auto freeArray = superblock_ptr->free();
        auto nfree = superblock_ptr->nfree();
        superblock_ptr->nfree(++nfree);
        if(nfree < 251)
        {
            freeArray[nfree] = blockIdx;
        }
        else
        {
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
        throw std::runtime_error("Out of memory: cannot allocate any more i-nodes!");
    }
    void FileSystem::initializeFreeList(std::shared_ptr<SuperBlock> superblock_ptr)
    {
        std::cout << "Number of data blocks: " << this->DATA_BLOCKS << std::endl;
        std::cout << "Data blocks start at: " << this->DATA_BLOCK_IDX << std::endl;
        for(int32_t i = DATA_BLOCKS - 1; i >= 0; i--)
        {
            this->freeDataBlock(superblock_ptr, DATA_BLOCK_IDX + i);
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
        auto inodeIdx = this->allocateINode();
        std::shared_ptr<INode> inode_ptr = this->getINode(inodeIdx);
        std::cout << "Allocated i-node " << inodeIdx << " for root" << std::endl;
        std::cout << "I-node " << inodeIdx << ": " << *inode_ptr << std::endl;
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
        
        std::shared_ptr<SuperBlock> superblock_ptr = this->getSuperBlock();
        auto blockIdx = this->allocateDataBlock(superblock_ptr);
        // update allocated block for file
        auto blocks = inode_ptr->addr();
        blocks[0] = blockIdx;
        inode_ptr->addr(blocks);

        std::cout << "I-node " << inodeIdx << ": " << *inode_ptr << std::endl;

        std::array<std::shared_ptr<File>, 32> file_ptrs = this->getFiles(blockIdx);
        std::shared_ptr<File> self = file_ptrs[0];
        self->inode(inodeIdx);
        self->name(".");
        std::shared_ptr<File> parent = file_ptrs[1];
        parent->inode(inodeIdx);
        parent->name("..");
        std::cout << "Allocated data block: " << blockIdx << std::endl;
    }
    std::string FileSystem::getExtendedFilename(std::string workingDirectory, std::string filename)
    {
        std::string slash{"/"};
        std::string target = filename;
        if(target.size() > 0 && target[target.size() - 1] != slash[0])
        {
            target += slash;
        }
        if(target[0] != slash[0])
        {
            target = this->_working_directory + target;
        }
        return target;
    }
    std::vector<std::string> FileSystem::parseFilename(std::string filename)
    {
        std::vector<std::string> path;
        signed int start = 0;
		signed int end = filename.find("/");
		std::string root = filename.substr(start, end-start);
		while(end != -1)
		{
			start = end + 1;
			end = filename.find("/", start);
			path.push_back(filename.substr(start, end-start));
		}
        if(root.size() != 0)
        {
            throw std::invalid_argument("Parsed non-rooted filename: " + filename);
        }
        for(auto component : path)
        {
            if(component.size() > 28)
            {
                throw std::invalid_argument("Parsed excessively long component \"" +
                    component + "\" from filename: " + filename);
            }
        }
        return path;
    }
    std::vector<uint32_t> FileSystem::getINodesForPath(std::vector<std::string> path)
    {
        std::vector<uint32_t> result;
        uint32_t currentIdx = 0;
        std::shared_ptr<INode> inode_ptr;
        result.push_back(currentIdx);
        for(auto element : path)
        {
            if(element.size() > 0)
            {
                inode_ptr = this->getINode(currentIdx);
                if(inode_ptr->filetype() == FileType::REGULAR)
                {
                    break;
                }
                else if(inode_ptr->filetype() == FileType::DIRECTORY)
                {
                    bool found_next = false;
                    std::vector<std::shared_ptr<File>> files = this->getFilesForINode(inode_ptr);
                    for(auto file_ptr : files)
                    {
                        auto next_inode = file_ptr->inode();
                        auto filename = file_ptr->name();
                        std::cout << "Contains file: " << filename << std::endl;
                        if(filename == element)
                        {
                            found_next = true;
                            currentIdx = next_inode;
                        }
                    }
                    if(!found_next)
                    {
                        break;
                    }
                    result.push_back(currentIdx);
                    std::cout << "I-node: " << *inode_ptr << std::endl;
                }
                else
                {
                    throw std::runtime_error("Unexpectedly found non-regular non-directory file!");
                }
            }
        }
        return result;
    }
    std::vector<uint32_t> FileSystem::getBlocksForINode(std::shared_ptr<INode> inode_ptr)
    {
        std::vector<uint32_t> result;
        if(inode_ptr->filesize() == FileSize::SMALL)
        {
            for(uint32_t block_address : inode_ptr->addr())
            {
                if(block_address != 0)
                {
                    result.push_back(block_address);
                }
            }
        }
        else
        {
            throw std::runtime_error("Medium/Large/Massive files in i-nodes not supported yet!");
        }
        return result;
    }
    std::vector<std::shared_ptr<File>> FileSystem::getFilesForINode(std::shared_ptr<INode> inode_ptr)
    {
        std::vector<std::shared_ptr<File>> files;
        std::vector<uint32_t> blockIdxs = this->getBlocksForINode(inode_ptr);
        for(auto blockIdx : blockIdxs)
        {
            std::array<std::shared_ptr<File>, 32> file_ptrs = this->getFiles(blockIdx);
            for(auto file_ptr : file_ptrs)
            {
                auto inode = file_ptr->inode();
                auto filename = file_ptr->name();
                if(filename.size() > 0)
                {
                    files.push_back(file_ptr);
                }
                else if(filename.size() == 0 && inode != 0)
                {
                    throw std::runtime_error("Discovered anomalous file with i-node " + std::to_string(inode) +
                        " and filename \'" + filename + "\'!");
                }
                else if(filename.size() < 0)
                {
                    throw std::runtime_error("Anomalous filename length for i-node " + std::to_string(inode) +
                        " and filename \'" + filename + "\'!");
                }
            }
        }
        return files;
    }
    std::string FileSystem::getWorkingDirectory(uint32_t inodeIdx)
    {
        std::string pwd = "/";
        auto inode_ptr = this->getINode(inodeIdx);
        std::vector<std::shared_ptr<File>> parent_ptrs = this->getFilesForINode(inode_ptr);
        while(inodeIdx != 0)
        {
            inode_ptr = this->getINode(inodeIdx);
            std::vector<std::shared_ptr<File>> file_ptrs = parent_ptrs;
            bool found = false;
            for(auto file_ptr : file_ptrs)
            {
                if(file_ptr->name() == "..")
                {
                    found = true;
                    auto parentIdx = file_ptr->inode();
                    auto parent_inode_ptr = this->getINode(parentIdx);
                    parent_ptrs = this->getFilesForINode(parent_inode_ptr);
                    bool found_child = false;
                    for(auto parent_file_ptr : parent_ptrs)
                    {
                        auto inode = parent_file_ptr->inode();
                        auto name = parent_file_ptr->name();
                        if(name != "." && name != ".." && inode == inodeIdx)
                        {
                            pwd = "/" + name + pwd;
                            found_child = true;
                            break;
                        }
                    }
                    if(!found_child)
                    {
                        throw std::runtime_error("I-node " + std::to_string(inodeIdx) + " is not a child of its parent!");
                    }
                    inodeIdx = parentIdx;
                    break;
                }
            }
            if(!found)
            {
                throw std::runtime_error("I-node " + std::to_string(inodeIdx) + " has no parent!");
            }
        }
        return pwd;
    }
    uint32_t FileSystem::createDirectory(std::string name, uint32_t parentIdx)
    {
        auto inodeIdx = this->allocateINode();
        std::shared_ptr<INode> inode_ptr = this->getINode(inodeIdx);
        std::cout << "Allocated i-node " << inodeIdx << std::endl;
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
        inode_ptr->allocated(true);
        inode_ptr->filetype(FileType::DIRECTORY);
        inode_ptr->filesize(FileSize::SMALL);
        inode_ptr->setsUID(false);
        inode_ptr->setsGID(false);
        inode_ptr->ownerR(true);
        inode_ptr->ownerW(true);
        inode_ptr->ownerX(false);
        inode_ptr->groupR(false);
        inode_ptr->groupW(false);
        inode_ptr->groupX(false);
        inode_ptr->worldR(false);
        inode_ptr->worldW(false);
        inode_ptr->worldX(false);
        // inode_ptr->flags(0b1100000110000000);
        std::cout << "Current i-node contents: " << *inode_ptr << std::endl;
        // the links are the parent link . and ..
        inode_ptr->nlinks(3);
        inode_ptr->uid(0);
        inode_ptr->gid(0);
        // 2 * file size
        // We have 1 for . and 1 for ..
        inode_ptr->size(2 * 32);
        // inode.addr();
        inode_ptr->actime(0);
        inode_ptr->modtime(0);
        
        std::shared_ptr<SuperBlock> superblock_ptr = this->getSuperBlock();
        auto blockIdx = this->allocateDataBlock(superblock_ptr);
        // update allocated block for file
        auto blocks = inode_ptr->addr();
        blocks[0] = blockIdx;
        inode_ptr->addr(blocks);

        std::cout << "i-node " << inodeIdx << ": " << *inode_ptr << std::endl;

        std::array<std::shared_ptr<File>, 32> file_ptrs = this->getFiles(blockIdx);
        std::shared_ptr<File> self = file_ptrs[0];
        self->inode(inodeIdx);
        self->name(".");
        std::shared_ptr<File> parent = file_ptrs[1];
        parent->inode(parentIdx);
        parent->name("..");
        std::cout << "Allocated data block: " << blockIdx << std::endl;
        auto parent_ptr = this->getINode(parentIdx);
        std::shared_ptr<File> file_ptr = this->addFileToINode(parent_ptr);
        file_ptr->inode(inodeIdx);
        file_ptr->name(name);
        return inodeIdx;
    }
    std::shared_ptr<File> FileSystem::addFileToINode(std::shared_ptr<INode> inode_ptr)
    {
        uint64_t size = inode_ptr->size();
        if((size + 32) < size)
        {
            throw std::overflow_error("I-node size " + std::to_string(size) + " overflows if extended!");
        }
        auto FILES_PER_BLOCK = 1024 / 32;
        this->resizeINode(inode_ptr, size + 32);
        std::vector<uint32_t> blocks = this->getBlocksForINode(inode_ptr);
        std::array<std::shared_ptr<File>, 32> files = this->getFiles(blocks.back());
        auto offset = (inode_ptr->size() % 1024) / FILES_PER_BLOCK;
        std::cout << "Offset of file in block is: " << std::to_string(offset) << std::endl;
        return files[offset];
    }
    void FileSystem::resizeINode(std::shared_ptr<INode> inode_ptr, uint64_t size)
    {
        if(size > 154618822656ull)
        {
            throw std::runtime_error("I-node data not accessible if extended to "+ std::to_string(size) + " bytes!");
        }
        std::cout << "Setting size of i-node to " << size << std::endl;
        auto old_size = inode_ptr->size();
        inode_ptr->size(size);
        auto old_blocks = old_size / 1024 + (old_size != 0);
        auto blocks = size / 1024 + (size != 0);
        std::cout << "old_blocks=" << old_blocks << ", blocks=" << blocks << std::endl;
        std::vector<std::vector<uint32_t>> levels = this->getBlocks(inode_ptr);
        auto level = 0;
        for(auto vec : levels)
        {
            std::cout << "Level" << ++level << ": size " << vec.size() << std::endl;
            for(auto member : vec)
            {
                std::cout << member << ", ";
            }
            std::cout << std::endl;
        }
        std::shared_ptr<SuperBlock> superblock_ptr = this->getSuperBlock();
        if(blocks == old_blocks)
        {
            std::cout << "I-node contains the same number of blocks before and after!" << std::endl;
        }
        else if(blocks == 0)
        {
            std::vector<std::vector<uint32_t>> blocks = this->getBlocks(inode_ptr);
            for(auto vec : blocks)
            {
                for(auto block : vec)
                {
                    this->freeDataBlock(superblock_ptr, block);
                }
            }
        }
        else if(blocks > old_blocks)
        {
            auto added = blocks - old_blocks;
            if(blocks < 9)
            {
                auto addr = inode_ptr->addr();
                for(auto i = old_blocks; i < old_blocks + added; i++)
                {
                    std::cout << "adding i-node in spot " << i << std::endl;
                    addr[i] = this->allocateDataBlock(superblock_ptr);
                }
                inode_ptr->addr(addr);
                levels = this->getBlocks(inode_ptr);
                level = 0;
                for(auto vec : levels)
                {
                    std::cout << "Level" << ++level << ": size " << vec.size() << std::endl;
                    for(auto member : vec)
                    {
                        std::cout << member << ", ";
                    }
                    std::cout << std::endl;
                }
            }
            else if(blocks < 2304)
            {
                throw std::runtime_error("Medium blocks not yet supported");
            }
            else if(blocks < 589824)
            {
                throw std::runtime_error("Large blocks not yet supported");
            }
            else if(blocks < 150994944)
            {
                throw std::runtime_error("Massive blocks not yet supported");
            }
            else
            {
                throw std::runtime_error("Excessive size=" + std::to_string(blocks) +": this should never happen!");
            }
        }
        else if(blocks < old_blocks)
        {
            auto subtracted = old_blocks - blocks;
            std::cout << "subtracted=" << subtracted << ", old_blocks=" << old_blocks << ", blocks=" << blocks
                << std::endl;
            if(blocks < 9)
            {
                auto addr = inode_ptr->addr();
                for(auto i = old_blocks - subtracted; i < old_blocks; i++)
                {
                    std::cout << "removing i-node in spot " << i << std::endl;
                    this->freeDataBlock(superblock_ptr, addr[i]);
                    addr[i] = 0;
                }
                inode_ptr->addr(addr);
            }
            else if(blocks < 2304)
            {
                throw std::runtime_error("Medium blocks not yet supported");
            }
            else if(blocks < 589824)
            {
                throw std::runtime_error("Large blocks not yet supported");
            }
            else if(blocks < 150994944)
            {
                throw std::runtime_error("Massive blocks not yet supported");
            }
            else
            {
                throw std::runtime_error("Excessive size=" + std::to_string(blocks) +": this should never happen!");
            }
        }
    }
    std::vector<std::vector<uint32_t>> FileSystem::getBlocks(std::shared_ptr<INode> inode_ptr)
    {
        std::array<uint32_t,9> addr = inode_ptr->addr();
        std::vector<uint32_t> blocks{addr.begin(), addr.end()};
        uint16_t depth = static_cast<uint16_t>(inode_ptr->filesize());
        return this->getBlocks(blocks, depth);
    }
    std::vector<std::vector<uint32_t>> FileSystem::getBlocks(std::vector<uint32_t> blocks, uint16_t depth)
    {
        std::vector<std::vector<uint32_t>> result;
        std::vector<uint32_t> top;
        for(auto block : blocks)
        {
            if(block != 0)
            {
                top.push_back(block);
            }
        }
        result.push_back(top);
        if(depth < 0)
        {
            throw std::runtime_error("Depth of " + std::to_string(depth) + " is invalid!");
        }
        if(depth != 0)
        {
            std::vector<std::vector<uint32_t>> levels;
            for(uint32_t level = 0; level < depth; level++)
            {
                std::vector<uint32_t> blockIdxs;
                levels.push_back(blockIdxs);
            }
            for(auto blockIdx : blocks)
            {
                if(blockIdx == 0)
                {
                    continue;
                }
                auto block_ptr = this->getBlock(blockIdx);
                std::array<uint32_t, 256>& intArray = block_ptr->asIntegers();
                std::vector<uint32_t> intermediate{intArray.begin(), intArray.end()};
                // std::copy_n(intArray.begin(), 256, intermediate.begin());
                std::vector<std::vector<uint32_t>> nextBlocks = this->getBlocks(intermediate, depth - 1);
                for(uint32_t level = 0; level < depth; level++)
                {
                    for(auto nextBlockIdx : nextBlocks[level])
                    {
                        levels[level].push_back(nextBlockIdx);
                    }
                }
            }
            for(auto vec : levels)
            {
                result.push_back(vec);
            }
        }
        return result;
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
    void FileSystem::initfs(uint32_t totalBlocks, uint32_t inodeBlocks)
    {
        std::cout << "Executing initfs " << totalBlocks << " " << inodeBlocks << std::endl;
        // flush blocks
        this->_blocks.clear();
        // total size = totalBlocks * block size = totalBlocks * 1024 bytes
        // i-nodes = 16 i-nodes per block
        // total i-nodes = 16 * inodeBlocks
        // block structure:
        // ______________________________________________________________________________________
        // | boot info | superblock | ... i-node blocks ... |        ... data blocks ...        |
        // |     0     |     1      | 2 to inodeBlocks + 1  | inodeBlocks + 2 to totalBlocks - 1|
        // ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄ ̄
        // 1024 bytes per block
        TOTAL_BLOCKS = totalBlocks;
        BOOT_INFO_BLOCKS = 1;
        SUPERBLOCK_BLOCKS = 1;
        INODE_BLOCKS = inodeBlocks;
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
        ftruncate(this->_fd, 0);
        ftruncate(this->_fd, TOTAL_BLOCKS * 1024);
        this->setDimensions(totalBlocks, inodeBlocks);
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
        if(this->_fd == -1)
        {
            std::cout << "openfs has not been called successfully, aborting cpin" << std::endl;
            return;
        }
        this->_blocks.clear();
    }
    void FileSystem::cpout(const std::string& innerFilename, const std::string& outerFilename)
    {
        std::cout << "Executing cpout " << innerFilename << " " << outerFilename << std::endl;
        if(this->_fd == -1)
        {
            std::cout << "openfs has not been called successfully, aborting cpout" << std::endl;
            return;
        }
        this->_blocks.clear();
    }
    void FileSystem::rm(const std::string& innerFilename)
    {
        std::cout << "Executing rm " << innerFilename << std::endl;
        if(this->_fd == -1)
        {
            std::cout << "openfs has not been called successfully, aborting rm" << std::endl;
            return;
        }
        this->_blocks.clear();
    }
    void FileSystem::mkdir(const std::string& innerFilename)
    {
        std::cout << "Executing mkdir " << innerFilename << std::endl;
        if(this->_fd == -1)
        {
            std::cout << "openfs has not been called successfully, aborting mkdir" << std::endl;
            return;
        }
        auto target = this->getExtendedFilename(this->_working_directory, innerFilename);
        std::vector<std::string> path = this->parseFilename(target);
        for(auto component : path)
        {
            std::cout << "Path component: " << component << std::endl;
        }
        std::vector<uint32_t> inodes = this->getINodesForPath(path);
        for(auto inode : inodes)
        {
            std::cout << "I-nodes for path: " << std::to_string(inode) << std::endl;
        }
        if(path.size() == inodes.size())
        {
            std::cout << "Failed to make directory: something exists there already!" << std::endl;
        }
        else if(path.size() < inodes.size() - 1)
        {
            std::cout << "Failed to make directory: parent is not a directory or does not exist!" << std::endl;
        }
        else
        {
            std::cout << "Creating directory " << innerFilename << std::endl;
            if(path.back() != "")
            {
                throw std::runtime_error("Path " + innerFilename + " was not parsed properly!");
            }
            path.pop_back();
            auto inode = this->createDirectory(path.back(), inodes.back());
            std::cout << "i-node for new directory: " << std::to_string(inode) << std::endl;
        }
        this->_blocks.clear();
    }
    void FileSystem::cd(const std::string& innerFilename)
    {
        std::cout << "Executing cd " << innerFilename << std::endl;
        if(this->_fd == -1)
        {
            std::cout << "openfs has not been called successfully, aborting cd" << std::endl;
            return;
        }
        auto target = this->getExtendedFilename(this->_working_directory, innerFilename);
        std::vector<std::string> path = this->parseFilename(target);
        for(auto component : path)
        {
            std::cout << "Path component: " << component << std::endl;
        }
        std::vector<uint32_t> inodes = this->getINodesForPath(path);
        for(auto inode : inodes)
        {
            std::cout << "I-nodes for path: " << std::to_string(inode) << std::endl;
        }
        if(path.size() == inodes.size())
        {
            auto inode_ptr = this->getINode(inodes.back());
            if(inode_ptr->filetype() != FileType::DIRECTORY)
            {
                std::cout << "Failed to change directory: target is not a directory!" << std::endl;
            }
            else
            {
                this->_working_directory = this->getWorkingDirectory(inodes.back());
                std::cout << "Moving to directory: " << this->_working_directory << std::endl;
            }
        }
        else
        {
            std::cout << "Failed to change directory: target not found!" << std::endl;
        }
        this->_blocks.clear();
    }
    void FileSystem::pwd()
    {
        std::cout << "Executing pwd" << std::endl;
        if(this->_fd == -1)
        {
            std::cout << "openfs has not been called successfully, aborting pwd" << std::endl;
            return;
        }
        std::cout << this->_working_directory << std::endl;
    }
    void FileSystem::ls()
    {
        std::cout << "Executing ls" << std::endl;
        if(this->_fd == -1)
        {
            std::cout << "openfs has not been called successfully, aborting ls" << std::endl;
            return;
        }
        std::vector<uint32_t> inodes = this->getINodesForPath(this->parseFilename(this->_working_directory));
        auto inodeIdx = inodes.back();
        std::vector<std::shared_ptr<File>> file_ptrs = this->getFilesForINode(this->getINode(inodeIdx));
        for(auto file_ptr : file_ptrs)
        {
            std::cout << file_ptr->name();
            if(this->getINode(file_ptr->inode())->filetype() == FileType::DIRECTORY)
            {
                std::cout << "/";
            }
            std::cout << std::endl;
        }
    }
    void FileSystem::sl()
    {
        std::cout << "Executing sl (steam locomotive)" << std::endl;
        std::cout << "     ++      +------ " << std::endl;
        std::cout << "     ||      |+-+ |  " << std::endl;
        std::cout << "   /---------|| | |  " << std::endl;
        std::cout << "  + ========  +-+ |  " << std::endl;
        std::cout << " _|--O========O~\\-+  " << std::endl;
        std::cout << "//// \\_/      \\_/    " << std::endl;
    }
    void FileSystem::test()
    {
        this->openfs("./fs");
        this->initfs(100, 5);
        for(auto i = 3; i <= 33; i++)
        {
            this->mkdir(std::to_string(i));
        }
        this->ls();
        auto inode_ptr = this->getINode(0);
        this->resizeINode(inode_ptr, 64);
        this->ls();
        this->quit();
    }
}