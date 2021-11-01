#include <iostream>
#include <memory>
#include "inode.hpp"

int main()
{
	std::cout << "Hello world" << std::endl;
	auto inode = (INode*) new INode();
	std::cout << inode << std::endl;
	std::cout << sizeof(inode) << std::endl;
	std::cout << sizeof(*inode) << std::endl;
	delete inode;
	return 0;
}
