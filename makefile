# Compiler
CXX = mpicxx

# Compiler flags
CXXFLAGS = -O3 -fopenmp

# Directories
SRCDIR = .
BUILDDIR = build

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cxx)

# Object files
OBJECTS = $(patsubst $(SRCDIR)/%.cxx,$(BUILDDIR)/%.o,$(SOURCES))

# Executable name
EXECUTABLE = $(BUILDDIR)/QuickHullMPI

# Default target
all: $(BUILDDIR) $(EXECUTABLE)

# Rule to create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Rule to create the executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@
	chmod +x $@

# Rule to compile source files to object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/QuickHullMPI
	

# Phony targets
.PHONY: all clean