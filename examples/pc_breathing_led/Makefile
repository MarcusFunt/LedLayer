CXX = g++
CXXFLAGS = -std=c++11 -I../.. -I../../src
SRCS = main.cpp ../../src/LedLayer.cpp ../../src/Layout.cpp ../../src/Color.cpp
TARGET = pc_breathing_led

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)
