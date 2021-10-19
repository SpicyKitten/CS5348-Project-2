# Run with "make clean test"
# Run "make clean" to delete the executable

CC = 		g++
FLAGS = 	-Wall -Werror -std=c++1y
EXECUTABLE = 	mod-v6
SRCS = 		$(wildcard ./*.cc)
OBJS = 		$(SRCS:.cc=.o)

all:	test

test:	$(OBJS)
	@$(CC) $(FLAGS) -o $(EXECUTABLE).exe $(OBJS)
	@echo "Built executable. Running..."
	@./$(EXECUTABLE).exe
	@$(MAKE) --no-print-directory -- --clean

edit:
	@nano $(SRCS)

--clean:
	@rm -f $(OBJS)
	@echo "Removed object files"

clean:	--clean
	@rm -f ./$(EXECUTABLE).exe
	@echo "Removed executable"

%.o:	%.cc
	@$(CC) $(FLAGS) -c $(SRCS) -o $(OBJS)

.PHONY: clean
.PHONY: --clean
