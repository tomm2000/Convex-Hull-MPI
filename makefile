# Compiler
CXX = mpicxx

# Compiler flags
CXXFLAGS = -O3 -fopenmp

# Directories
SRCDIR = .
BUILDDIR = build

# Source files (excluding main files)
SOURCES = $(filter-out $(SRCDIR)/QuickHullMPI.cxx $(SRCDIR)/QuickHullSeq.cxx $(SRCDIR)/QuickHullOpenMP.cxx, $(wildcard $(SRCDIR)/*.cxx))

# Object files
OBJECTS = $(patsubst $(SRCDIR)/%.cxx,$(BUILDDIR)/%.o,$(SOURCES))

# Executable names
EXECUTABLE_MPI = $(BUILDDIR)/QuickHullMPI
EXECUTABLE_SEQ = $(BUILDDIR)/QuickHullSeq
EXECUTABLE_OPENMP = $(BUILDDIR)/QuickHullOpenMP

# Default target
all: $(BUILDDIR) $(EXECUTABLE_MPI) $(EXECUTABLE_SEQ) $(EXECUTABLE_OPENMP)

# Rule to create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Rule to create the MPI executable
$(EXECUTABLE_MPI): $(OBJECTS) $(BUILDDIR)/QuickHullMPI.o
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(BUILDDIR)/QuickHullMPI.o -o $@
	chmod +x $@

# Rule to create the Sequential executable
$(EXECUTABLE_SEQ): $(OBJECTS) $(BUILDDIR)/QuickHullSeq.o
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(BUILDDIR)/QuickHullSeq.o -o $@
	chmod +x $@

# Rule to create the OpenMP executable
$(EXECUTABLE_OPENMP): $(OBJECTS) $(BUILDDIR)/QuickHullOpenMP.o
	$(CXX) $(CXXFLAGS) -fopenmp $(OBJECTS) $(BUILDDIR)/QuickHullOpenMP.o -o $@
	chmod +x $@

# Rule to compile source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/QuickHullMPI $(BUILDDIR)/QuickHullSeq $(BUILDDIR)/QuickHullOpenMP

# Phony targets
.PHONY: all clean
