CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic

.PHONY: all clean
all: a.out

a.out: device.cpp device.hpp recycle.cpp recycle.hpp main.cpp
	$(CXX) $(CXXFLAGS) device.cpp recycle.cpp main.cpp -o $@

clean:
	$(RM) a.out
