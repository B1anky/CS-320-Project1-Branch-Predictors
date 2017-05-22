
FLAGS = -g -Wall -pedantic -Wextra  -std=c++11

all: clean project1

project1: project1.o 
	g++ $(FLAGS) project1.o -o predictors

project1.o: project1.cpp
	g++ $(FLAGS) -c project1.cpp -o project1.o

clean:
	rm -f *.o predictors