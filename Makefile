CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

SRCS = main.cpp bitboard.cpp evaluate.cpp search.cpp uci.cpp movesgen.cpp zobrist.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = engine

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
