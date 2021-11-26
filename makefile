# Run with "make clean test"
# Run "make clean" to delete the executable

CC = 		g++
FLAGS = 	-Wall -Werror -std=c++1y
EXECUTABLE = 	mod-v6
SRCS = 		$(wildcard ./*.cpp)
INCLUDES =  $(wildcard ./*.hpp)
OBJS = 		$(SRCS:.cpp=.o)

all:	test

test:	$(OBJS)
	@$(CC) $(FLAGS) -o $(EXECUTABLE).exe $(OBJS)
	@echo "Built executable. Running..."
	@valgrind ./$(EXECUTABLE).exe
	@$(MAKE) --no-print-directory -- --clean

edit:
	@nano $(SRCS)

--clean:
	@rm -f $(OBJS)
	@echo "Removed object files"

clean:	--clean
	@rm -f ./$(EXECUTABLE).exe
	@echo "Removed executable"

%.o:	%.cpp
	@$(CC) $(FLAGS) -c $^ -o $@

.PHONY: clean all edit test
.PHONY: --clean
