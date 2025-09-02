CXX=g++
CXXFLAGS=-Wall -std=c++14 -O0 -ggdb -fno-strict-aliasing

.PHONY: all clean

all: recext2fs

recext2fs: read.c 
	$(CXX) $(CXXFLAGS) read.c  -o recext2fs

clean:
	rm -f recext2fs

