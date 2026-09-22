CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic

.PHONY: all clean
all: a.out

a.out: device.cpp device.hpp main.cpp
	$(CXX) $(CXXFLAGS) device.cpp main.cpp -o $@

clean:
	$(RM) a.out
