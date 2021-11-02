#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include "inode.hpp"
#include "filesys.hpp"
using namespace std;

int main(int argc, char* argv[])
{
	FileSystem fs;
	string input;
	string command;
	vector<string> arguments;
	while(true)
	{
		cout << "Waiting for input. Enter your command below:" << endl;
		getline(cin, input);
		arguments.clear();
		signed int start = 0;
		signed int end = input.find(" ");
		if(input == "q")
		{
			break;
		}
		else if(end == -1)
		{
			cout << "Please enter a valid command!" << endl;
			continue;
		}
		command = input.substr(start, end-start);
		while(end != -1)
		{
			start = end + 1;
			end = input.find(" ", start);
			arguments.push_back(input.substr(start, end-start));
		}
		cout << "Command: " << command <<endl;
		for(auto arg : arguments)
		{
			cout << "Argument: " << arg << endl;
		}
		if(command == "openfs" && arguments.size() == 1)
		{
			fs.openfs(arguments[0]);
		}
		else if(command == "initfs" && arguments.size() == 2)
		{
			auto totalBlocks = stoul(arguments[0]);
			auto iNodeBlocks = stoul(arguments[1]);
			fs.initfs(totalBlocks, iNodeBlocks);
		}
		else
		{
			cout << "Unrecognized command, please try again" << endl;
		}
	}
	unique_ptr<INode> inode{new INode()};
	cout << *inode << endl;
	cout << sizeof(inode) << endl;
	cout << sizeof(*inode) << endl;
	return 0;
}
