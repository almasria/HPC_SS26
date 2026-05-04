# Compiler and flags
CXX = mpic++
CXXSTD = -std=c++20
DEBUG_FLAGS = -g -Wall -Wextra -DDEBUG
RELEASE_FLAGS = -O3
INC = -I./inc

# Paths
SRC1 = ./src/2_1.cpp
DEBUG_OUT1 = 2_1_dbg
RELEASE_OUT1 = ./bin/exercise1

SRC2 = ./src/2_2_a.cpp
DEBUG_OUT2 = 2_2_adbg
RELEASE_OUT2 = ./bin/exercise2a

SRC3 = ./src/2_2_b.cpp
DEBUG_OUT3 = 2_2_bdbg
RELEASE_OUT3 = ./bin/exercise2b

# Default target
all: release

# Ensure bin/ exists
bin:
	mkdir -p bin

# Debug build
debug: $(DEBUG_OUT1) $(DEBUG_OUT2) $(DEBUG_OUT3)

$(DEBUG_OUT1): $(SRC1)
	$(CXX) $(SRC1) -o $(DEBUG_OUT1) $(CXXSTD) $(DEBUG_FLAGS) $(INC)

$(DEBUG_OUT2): $(SRC2)
	$(CXX) $(SRC2) -o $(DEBUG_OUT2) $(CXXSTD) $(DEBUG_FLAGS) $(INC)

$(DEBUG_OUT3): $(SRC3)
	$(CXX) $(SRC3) -o $(DEBUG_OUT3) $(CXXSTD) $(DEBUG_FLAGS) $(INC)

# Release build
release: $(RELEASE_OUT1) $(RELEASE_OUT2) $(RELEASE_OUT3)

$(RELEASE_OUT1): $(SRC1) | bin
	$(CXX) $(SRC1) -o $(RELEASE_OUT1) $(CXXSTD) $(RELEASE_FLAGS) $(INC)

$(RELEASE_OUT2): $(SRC2) | bin
	$(CXX) $(SRC2) -o $(RELEASE_OUT2) $(CXXSTD) $(RELEASE_FLAGS) $(INC)

$(RELEASE_OUT3): $(SRC3) | bin
	$(CXX) $(SRC3) -o $(RELEASE_OUT3) $(CXXSTD) $(RELEASE_FLAGS) $(INC)
# Clean
clean:
	rm -f $(DEBUG_OUT1) $(RELEASE_OUT1)
	rm -f $(DEBUG_OUT2) $(RELEASE_OUT2)
	rm -f $(DEBUG_OUT3) $(RELEASE_OUT3)

.PHONY: all debug release clean
