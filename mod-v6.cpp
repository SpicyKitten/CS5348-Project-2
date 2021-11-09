#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <set>
#include "inode.hpp"
#include "filesys.hpp"
#include "superblock.hpp"
#include "directory.hpp"

namespace ModV6FileSystem
{
	inline namespace ArgCheck
	{
		bool expected(std::set<std::string>& supportedCommands, std::string command, std::string targetCommand, 
			std::vector<std::string> arguments, size_t expectedArguments)
		{
			supportedCommands.insert(targetCommand);
			if(command == targetCommand)
			{
				std::string complaint{arguments.size() < expectedArguments ? "few" : "many"};
				if(arguments.size() != expectedArguments)
				{
					std::cout << "Command " << command << " provided with too " 
						<< complaint << " arguments" << std::endl;
					return false;
				}
				return true;
			}
			return false;
		}
	}
}

int main(int argc, char* argv[])
{
	// using ModV6FileSystem::ArgCheck::expected;
	using namespace ModV6FileSystem;
	std::unique_ptr<FileSystem> fs{new FileSystem()};
	std::string input;
	std::string command;
	std::vector<std::string> arguments;
	std::set<std::string> supported;
	while(true)
	{
		std::cout << "Waiting for input. Enter your command below:" << std::endl;
		std::getline(std::cin, input);
		arguments.clear();
		signed int start = 0;
		signed int end = input.find(" ");
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
		// checks if any supported command is called,
		// updating the set of supported commands along the way
		if(expected(supported, command, "q", arguments, 0))
		{
			fs->quit();
			break;
		}
		else if(expected(supported, command, "openfs", arguments, 1))
		{
			fs->openfs(arguments[0]);
		}
		else if(expected(supported, command, "initfs", arguments, 2))
		{
			auto totalBlocks = std::stoul(arguments[0]);
			auto iNodeBlocks = std::stoul(arguments[1]);
			fs->initfs(totalBlocks, iNodeBlocks);
		}
		else if(supported.find(command) == supported.end())
		{
			std::cout << "Unrecognized command, please try again" << std::endl;
		}
	}
	Directory directory{};
	SuperBlock superblock{};
	std::cout << "Directory inode: " << directory.inode() << std::endl;
	std::cout << "Superblock isize: " << superblock.isize() << std::endl;
	std::unique_ptr<INode> inode{new INode()};
	std::cout << *inode << std::endl;
	std::cout << sizeof(inode) << std::endl;
	std::cout << sizeof(*inode) << std::endl;
	return 0;
}