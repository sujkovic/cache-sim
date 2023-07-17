TARGET = cache-sim
CC = g++
CFLAGS = -Wall -Wextra -g

all: $(TARGET)

$(TARGET): cache-sim.o PLRUTree.o
	$(CC) cache-sim.o PLRUTree.o -o $@

cache-sim.o: cache-sim.cpp PLRUTree.cpp
	$(CC) $(CFLAGS) -c cache-sim.cpp -o $@

PLRUTree.o:	PLRUTree.cpp 
	$(CC) $(CFLAGS) -c PLRUTree.cpp -o $@

clean:
	rm -rf *.o $(TARGET)