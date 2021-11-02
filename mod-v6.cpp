#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "inode.hpp"
#include "filesys.hpp"

int main(int argc, char* argv[])
{
	FileSystem fs;
	std::string input;
	std::string command;
	std::vector<std::string> arguments;
	while(true)
	{
		std::cout << "Waiting for input. Enter your command below:" << std::endl;
		std::getline(std::cin, input);
		arguments.clear();
		signed int start = 0;
		signed int end = input.find(" ");
		if(input == "q")
		{
			break;
		}
		else if(end == -1)
		{
			std::cout << "Please enter a valid command!" << std::endl;
			continue;
		}
		command = input.substr(start, end-start);
		while(end != -1)
		{
			start = end + 1;
			end = input.find(" ", start);
			arguments.push_back(input.substr(start, end-start));
		}
		std::cout << "Command: " << command << std::endl;
		for(auto arg : arguments)
		{
			std::cout << "Argument: " << arg << std::endl;
		}
		if(command == "openfs" && arguments.size() == 1)
		{
			fs.openfs(arguments[0]);
		}
		else if(command == "initfs" && arguments.size() == 2)
		{
			auto totalBlocks = std::stoul(arguments[0]);
			auto iNodeBlocks = std::stoul(arguments[1]);
			fs.initfs(totalBlocks, iNodeBlocks);
		}
		else
		{
			std::cout << "Unrecognized command, please try again" << std::endl;
		}
	}
	std::unique_ptr<INode> inode{new INode()};
	std::cout << *inode << std::endl;
	std::cout << sizeof(inode) << std::endl;
	std::cout << sizeof(*inode) << std::endl;
	return 0;
}
