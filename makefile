CXX      := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -g
TARGET   := compiler
HEADERS  := $(wildcard src/*.h)
SRCS     := $(wildcard src/*.cpp)

all : build/$(TARGET)

clean :
	rm -rf build

build/$(TARGET) : $(SRCS) $(HEADERS)
	mkdir -p build/
	clang++ $(CXXFLAGS) $(SRCS) -o build/$(TARGET)


