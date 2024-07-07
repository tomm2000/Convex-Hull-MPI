# Compiler
MPICXX = mpicxx

# Compiler flags
CXXFLAGS = -Wall -O3

# Target object file
TARGET = quickhull_mpi.o

# Source files
SRCS = QuickHullMPI.cxx QuickHullDistributed.cxx QuickHullSequential.cxx lib.cxx

# Object files
OBJS = $(SRCS:.cxx=.o)

# Default target
all: $(TARGET)

# Linking (now creates an object file instead of an executable)
$(TARGET): $(OBJS)
	$(MPICXX) -r -o $@ $^

# Compilation
%.o: %.cxx
	$(MPICXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean