CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -I.
LDFLAGS = -pthread
TARGET = server

SRCS = main.cpp \
       core/server.cpp \
       pool/threadpool.cpp \
       log/log.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run