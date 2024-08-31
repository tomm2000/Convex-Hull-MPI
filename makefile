# Compiler
CXX = mpicxx

# Compiler flags
CXXFLAGS = -O3 -fopenmp

# Directories
MAIN_DIR = .
SRC_DIR = src
BUILD_DIR = build

# Source files
MAIN_SRC = $(MAIN_DIR)/main.cxx
LIB_SRCS = $(wildcard $(SRC_DIR)/*.cxx)

# Object files
MAIN_OBJ = $(BUILD_DIR)/main.o
LIB_OBJS = $(patsubst $(SRC_DIR)/%.cxx,$(BUILD_DIR)/%.o,$(LIB_SRCS))

# All object files
OBJS = $(MAIN_OBJ) $(LIB_OBJS)

# Header files
INCLUDES = -I$(MAIN_DIR) -I$(SRC_DIR)

# Executable name
TARGET = $(BUILD_DIR)/main

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile main.cxx
$(MAIN_OBJ): $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile lib/*.cxx files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cxx
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Run the program
run: $(TARGET)
	$(TARGET) npoints=100000000

mpi: $(TARGET)
	mpirun -np 6 $(TARGET)

# Clean up
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all run clean