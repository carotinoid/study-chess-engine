# Makefile for Chess AI Project

# Compiler and flags
CXX = g++
# Add -Iinclude to tell the compiler where to find header files
CXXFLAGS = -std=c++17 -Wall -O2 -Iinclude
LDFLAGS = -pthread

# Target executable name
TARGET = chessai

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Find all .cpp files in the source directory
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/Engine.cpp $(SRC_DIR)/Game.cpp $(SRC_DIR)/AIPlayer.cpp $(SRC_DIR)/Bitboard.cpp $(SRC_DIR)/MoveGen.cpp $(SRC_DIR)/MagicBitboards.cpp $(SRC_DIR)/Zobrist.cpp $(SRC_DIR)/TranspositionTable.cpp $(SRC_DIR)/Book.cpp $(SRC_DIR)/Init.cpp

# Create a list of object files in the build directory
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Rule to create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files from src/ into object files in build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET) *.o test_runner

# Phony targets
.PHONY: all clean

# --- Test ---
TEST_SRC_DIR = test
TEST_BUILD_DIR = build/test
TEST_TARGET = test_runner

# All test source files, now including test_opening.cpp
TEST_SRCS = $(TEST_SRC_DIR)/test_main.cpp $(TEST_SRC_DIR)/test_board.cpp $(TEST_SRC_DIR)/test_moves.cpp $(TEST_SRC_DIR)/test_fen.cpp $(TEST_SRC_DIR)/test_opening.cpp

# Object files for the tests
TEST_OBJS = $(patsubst $(TEST_SRC_DIR)/%.cpp,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))

# Dependencies for the tests (all engine code except main.cpp and Engine.cpp)
TEST_DEPS_SRCS = $(filter-out $(SRC_DIR)/main.cpp $(SRC_DIR)/Engine.cpp, $(SRCS))
TEST_DEPS_SRCS += $(SRC_DIR)/Debug.cpp
TEST_DEPS_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(TEST_DEPS_SRCS))

# Main test target
test: $(TEST_TARGET)
	@./$(TEST_TARGET)

# Link test object files to create the test executable
$(TEST_TARGET): $(TEST_OBJS) $(TEST_DEPS_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TEST_TARGET) $^

# Rule to create the test build directory
$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

# Compile test source files into object files in build/test/
$(TEST_BUILD_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp | $(TEST_BUILD_DIR)
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@

# Add test target to phony
.PHONY: all clean test
