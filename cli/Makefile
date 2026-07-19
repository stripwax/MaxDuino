CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wno-unused-variable -Wno-sign-compare \
  -include userCLIconfig.h -DCONFIGFILE=-1 -DCLI \
  -I../MaxDuino -I./stubs

MAX = ../MaxDuino
BUILD = build

SRCS = $(wildcard $(MAX)/*.cpp) main.cpp
OBJS = $(patsubst $(MAX)/%.cpp,$(BUILD)/%.o,$(filter $(MAX)/%.cpp,$(SRCS))) \
       $(BUILD)/main.o

TARGET = $(BUILD)/maxduino

all: $(BUILD) $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/%.o: $(MAX)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD)

.PHONY: all clean
