Collaborative project to implement a modified version of v6 file system (Fall 2021)
Run with "make clean test"  
Delete executable produced with "make clean"

A pretty neat way to handle resizing a file of blocks which unfortunately we didn't have enough time to implement in the codebase. It goes something like this:
```C++
// blockSize is in bytes, describing the range of addresses for the block
// call with root block index, begin address of root block, final address reachable from root block 
// 		if all child blocks filled, desired new size of block, and block size of root block 
// 		if all child blocks filled
uint32_t resize(blockIdx, beginAddress, endAddress, newSize, blockSize)
{
    // this should not happen because the sizes for all blocks should only be:
    // 1024 					bytes (direct block)
    // 1024 * 256				bytes (1st-level indirect block)
    // 1024 * 256 * 256			bytes (2nd-level indirect block)
    // 1024 * 256 * 256 * 256 	bytes (3rd-level indirect block)
    if(blockSize < 1024)
    {
        throw std::runtime_error("Invalid block size: " + std::to_string(blockSize) + " in resize block!")
    }
    // we only need to put a block if resizing up and the block is empty
    if(blockIdx == 0 && beginAddress <= newSize)
    {
        blockIdx = allocate block () // call block allocating function
    }
    auto block_ptr = this->getBlock(blockIdx)
    // bigger blocks need lower level blocks to be processed,
    // but only if there's anything supposed to be there in the first place
    if(blockSize > 1024 && blockIdx != 0)
    {
        // each block has 256 block indices of lower-level blocks
        auto childBlockSize = blockSize / 256
        for(auto child = 0; child < 256; ++child)
        {
            auto childBlockIdx = block as integers [child]
            auto childBlock = this->getBlock(childBlockIdx)
            childBlockIdx = resize(childBlock, beginAddress + childBlockSize * child, 
                    beginAddress + childBlockSize * (child + 1) - 1,
                    newSize, childBlockSize)
            block as integers[child] = childBlockIdx
        }
    }
    // we only free a block if its first address exceeds the desired size of the file
    if(blockIdx != 0 && beginAddress >= newSize)
    {
        free block (blockIdx) // call block freeing function
        blockIdx = 0
    }
    return blockIdx
}

//////////////////////////////////////////////////////////////
// Call the method above with something like the following: //
//////////////////////////////////////////////////////////////

auto inode_ptr = this->getINode(...) // the i-node for the file to be resized
auto newsize = ... // the new size
auto addr = inode_ptr->addr()
auto blockSize = 
    // 1024						if inode_ptr->filesize() == FileSize::SMALL
    // 1024 * 256				if inode_ptr->filesize() == FileSize::MEDIUM
    // 1024 * 256 * 256			if inode_ptr->filesize() == FileSize::LARGE
    // 1024 * 256 * 256 * 256 	if inode_ptr->filesize() == FileSize::MASSIVE
    
for(auto i = 0; i < 9; ++i)
{
    addr[i] = this->resize(addr[i], blockSize * i, blockSize * (i + 1) - 1, newsize, blockSize)
}
inode_ptr->addr(addr)
```
