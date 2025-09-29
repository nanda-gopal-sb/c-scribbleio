# Makefile to build server and test executables into build/ directory

CXX       := g++
CXXFLAGS  := -std=c++17 -O2 -Wall -Wextra -pedantic

BUILD_DIR      := build
SERVER_SRC     := server.cpp
TEST_SRC       := test.cpp
SERVER_TARGET  := $(BUILD_DIR)/server
TEST_TARGET    := $(BUILD_DIR)/test

.PHONY: all clean

all: $(SERVER_TARGET) $(TEST_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SERVER_TARGET): $(SERVER_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TEST_TARGET): $(TEST_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(SERVER_TARGET) $(TEST_TARGET)