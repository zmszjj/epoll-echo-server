CXX = g++
CXXFLAGS = -I./include -Wall

TARGET = bin/server
SRCS = src/main.cpp src/epoll_server.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -rf bin