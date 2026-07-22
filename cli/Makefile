CXX = g++
OPTFLAGS = -O2
CXXFLAGS = -std=c++17 $(OPTFLAGS) -Wall -Wno-unused-variable -Wno-sign-compare \
  -include userCLIconfig.h -DCONFIGFILE=-1 -DCLI \
  -I../MaxDuino -I./stubs

MAX = ../MaxDuino
BUILD = build

SRCS = $(wildcard $(MAX)/*.cpp) main.cpp
OBJS = $(patsubst $(MAX)/%.cpp,$(BUILD)/%.o,$(filter $(MAX)/%.cpp,$(SRCS))) \
       $(BUILD)/main.o

TARGET = $(BUILD)/maxduino

all: $(BUILD) $(TARGET)
	cp maxduinobatch.sh $(BUILD)/maxduinobatch.sh

$(BUILD):
	mkdir -p $(BUILD)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/%.o: $(MAX)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

WINBUILD = $(BUILD)/win
WIN_OBJS = $(patsubst $(MAX)/%.cpp,$(WINBUILD)/%.o,$(filter $(MAX)/%.cpp,$(SRCS))) \
           $(WINBUILD)/main.o

win: $(WINBUILD) $(BUILD)/maxduino.exe

$(WINBUILD):
	mkdir -p $(WINBUILD)

$(BUILD)/maxduino.exe: CXX = x86_64-w64-mingw32-g++
$(BUILD)/maxduino.exe: $(WIN_OBJS)
	$(CXX) $(CXXFLAGS) -static-libgcc -static-libstdc++ -o $@ $^

$(WINBUILD)/%.o: CXX = x86_64-w64-mingw32-g++
$(WINBUILD)/%.o: $(MAX)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(WINBUILD)/main.o: CXX = x86_64-w64-mingw32-g++
$(WINBUILD)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

debug: OPTFLAGS = -g -O0
debug: all

clean:
	rm -rf $(BUILD)

.PHONY: all clean win debug
