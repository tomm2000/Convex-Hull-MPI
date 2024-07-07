# Compiler
MPICXX = mpicxx

# Compiler flags
CXXFLAGS = -Wall -O2

# Target executable
TARGET = quickhull_mpi

# Source files
SRCS = QuickHullMPI.cxx QuickHullDistributed.cxx QuickHullSequential.cxx lib.cxx

# Object files
OBJS = $(SRCS:.cxx=.o)

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(MPICXX) $(CXXFLAGS) -o $@ $^

# Compilation
%.o: %.cxx
	$(MPICXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean