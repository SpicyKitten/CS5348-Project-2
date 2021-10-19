#include <stdio.h>
#include <unistd.h>
#include <cstdlib>

// all child processes get here
void doChildThings(int id)
{
	printf("I am child number %d\n", id);
	// stop child process to prevent accidentally creating a fork() bomb
	std::exit(EXIT_SUCCESS);
}

// only parent process should get here
void doParentThings(int pid)
{
	printf("I am main process and spawned process %d\n", pid);
}

int main()
{
	// 8 processes
	const auto N_PROCS = 8;
	for(auto i = 1; i <= N_PROCS; ++i)
	{
		auto pid = fork();
		// fork works fine but it's not going to stop concurrency ordering errors
		switch(pid)
		{
			case 0:
				doChildThings(i);
			default:
				doParentThings(pid);
		}
	}
	return EXIT_SUCCESS;
}
