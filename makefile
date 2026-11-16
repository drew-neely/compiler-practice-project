CXX      := clang++
CXXFLAGS := -std=c++23 -Wall -Wextra -g
SANFLAGS := -O0 -fsanitize=address,undefined -fno-omit-frame-pointer
TARGET   := compiler
HEADERS  := $(wildcard src/*.h)
SRCS     := $(wildcard src/*.cpp)

all : build/$(TARGET)

clean :
	rm -rf build

build/$(TARGET) : $(SRCS) $(HEADERS)
	mkdir -p build/
	clang++ $(CXXFLAGS) $(SRCS) -o build/$(TARGET)

debug : build/$(TARGET)-debug

build/$(TARGET)-debug : $(SRCS) $(HEADERS)
	mkdir -p build/
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(SRCS) -o build/$(TARGET)-debug

.PHONY : all clean debug


