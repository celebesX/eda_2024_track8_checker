SRC = global.cpp util.cpp rsmt.cpp arch.cpp lib.cpp object.cpp netlist.cpp legal.cpp wirelength.cpp pindensity.cpp main.cpp
OBJ = $(SRC:.cpp=.o)
CC = g++

CFLAGS = -Wall -Wextra -std=c++11
CFLAGS += -g

all: checker

checker: $(OBJ)
	$(CC) $(CFLAGS) -o checker $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f checker $(OBJ)
