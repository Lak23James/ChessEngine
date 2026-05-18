CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp, build/%.o, $(SRCS))
TARGET = build/engine.exe

all: build_dir $(TARGET)

build_dir:
	@if not exist build mkdir build

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@if exist build rmdir /s /q build

