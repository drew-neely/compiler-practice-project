CXX          := clang++
CXXFLAGS     := -std=c++23 -Wall -Wextra -g -Isrc
SANFLAGS     := -O0 -fsanitize=address,undefined -fno-omit-frame-pointer
TARGET       := compiler
TESTER       := compiler_tester
HEADERS      := $(wildcard src/*.h)
SRCS         := $(wildcard src/*.cpp)
LIB_SRCS     := $(filter-out src/main.cpp, $(wildcard src/*.cpp))
TEST_HEADERS := $(wildcard test/src/*.h)
TEST_SRCS    := $(wildcard test/src/*.cpp)

all : build/$(TARGET) build/$(TESTER)

clean :
	rm -rf build

build/$(TARGET) : $(SRCS) $(HEADERS)
	mkdir -p build/
	$(CXX) $(CXXFLAGS) $(SRCS) -o build/$(TARGET)

debug : build/$(TARGET)-debug

build/$(TARGET)-debug : $(SRCS) $(HEADERS)
	mkdir -p build/
	$(CXX) $(CXXFLAGS) $(SANFLAGS) $(SRCS) -o build/$(TARGET)-debug

tester : build/$(TESTER)

build/$(TESTER) : $(LIB_SRCS) $(HEADERS) $(TEST_SRCS) $(TEST_HEADERS)
	mkdir -p build/
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) $(TEST_SRCS) -o build/$(TESTER)

.PHONY : all clean debug


