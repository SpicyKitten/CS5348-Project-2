#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <set>
#include "inode.hpp"
#include "filesys.hpp"
#include "superblock.hpp"
#include "file.hpp"
#include "block.hpp"

/**
 * Group members:
 * Avilash Rath (asr210008)
 * 		Contributions: Programming
 * Sai Tatireddy (sxt190086)
 * 		Contributions: Planning + Initializing free linked list
 * Haowei Liu (hxl210017)
 * 		Contributions: Planning
 * 
 * How to run our code:
 * 		unzip whatever needs to be unzipped
 * 		put it all in a directory
 * 		cd into the directory from the command line
 * 		run the command "make clean test"
 * 		input openfs 	command such as >openfs ./fs
 * 		input initfs 	command such as >initfs 1000 100
 * 		input q 		command such as >q
 * 
 * Be wary - bad things will happen if you openfs a file and don't initfs it
 */
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
		else if(expected(supported, command, "help", arguments, 0))
		{
			std::cout << "Supported commands:" << std::endl;
			std::cout << "	openfs <filename>" << std::endl;
			std::cout << "	initfs <totalBlocks> <iNodeBlocks>" << std::endl;
		}
		else if(supported.find(command) == supported.end())
		{
			std::cout << "Unrecognized command, please try again" << std::endl;
		}
	}
	// fs->openfs("./fs");
	// std::shared_ptr<Block> block = fs->getBlock(0);
	// std::array<uint8_t, 1024>& bytes = block->asBytes();
	// for(auto i = 0; i < 1024; ++i)
	// {
	// 	bytes[i] = i/64;
	// }
	// for(auto i = 0; i < 1024/64; ++i)
	// {
	// 	inode->flags(0xFFFF);
	// 	std::cout << "INode " << std::to_string(i) << ": " << *inode << std::endl;
	// }
	// fs = std::unique_ptr<FileSystem>{nullptr};
	// File file{};
	// std::cout << "File inode: " << file.inode() << std::endl;
	// std::unique_ptr<INode> inode{new INode()};
	// std::cout << *inode << std::endl;
	// std::cout << sizeof(inode) << std::endl;
	// std::cout << sizeof(*inode) << std::endl;
	// std::shared_ptr<Block> block = fs->getBlock(0);
	// block->asFiles();
	// std::cout << "File: " << file << std::endl;
	// std::cout << "Block size: " << sizeof(block->asFiles()) << " bytes" << std::endl;
	// std::cout << "Block size: " << sizeof(block->asINodes()) << " bytes" << std::endl;
	// std::cout << "Block size: " << sizeof(block->asBytes()) << " bytes" << std::endl;
	// std::cout << "Block size: " << sizeof(block->asIntegers()) << " bytes" << std::endl;
	// INode& node = block->asINodes()[0];
	// // uid = f i l e in binary
	// // gid = n a m e in binary
	// node.uid(0b01000110011010010110110001100101ul);
	// node.gid(0b01001110011000010110110101100101ul);
	// // prints differently because of endian-ness
	// std::cout << "Reinterpreted INode as File: " << block->asFiles()[0] << std::endl;
	// SuperBlock superblock{block};
	// std::cout << "superblock isize=" << superblock.isize() << std::endl;
	return 0;
}