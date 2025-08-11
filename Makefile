# Makefile for Chess AI Project

# Compiler and flags
CXX = g++
# Add -Iinclude to tell the compiler where to find header files
CXXFLAGS = -std=c++17 -Wall -O2 -Iinclude
LDFLAGS =

# Target executable name
TARGET = chessai

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Find all .cpp files in the source directory
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Create a list of object files in the build directory
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $^

# Rule to create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files from src/ into object files in build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET) *.o

# Phony targets
.PHONY: all clean
