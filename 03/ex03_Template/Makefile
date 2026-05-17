# HPDC SS 2026 - Exercise 3 Makefile

# Compiler and flags
CXX        		= mpic++
CXXSTD     		= -std=c++20
DEBUG_FLAGS   	= -g -Wall -Wextra -DDEBUG
RELEASE_FLAGS 	= -O3 -march=native
INC        		= -I./inc

# Sources
SRC1       		= ./src/3_1.cpp
SRC2       		= ./src/3_2_3.cpp

DEBUG_OUT1    	= 3_1_dbg
RELEASE_OUT1  	= ./bin/exercise1

DEBUG_OUT2    	= 3_2_3_dbg
RELEASE_OUT2  	= ./bin/exercise2

# Default target
all: release

# Ensure bin/ exists
bin:
	mkdir -p bin

# ---- Debug builds ----
debug: $(DEBUG_OUT1) $(DEBUG_OUT2)

$(DEBUG_OUT1): $(SRC1)
	$(CXX) $(SRC1) -o $(DEBUG_OUT1) $(CXXSTD) $(DEBUG_FLAGS) $(INC)

$(DEBUG_OUT2): $(SRC2)
	$(CXX) $(SRC2) -o $(DEBUG_OUT2) $(CXXSTD) $(DEBUG_FLAGS) $(INC)

# ---- Release builds ----
release: $(RELEASE_OUT1) $(RELEASE_OUT2)

$(RELEASE_OUT1): $(SRC1) | bin
	$(CXX) $(SRC1) -o $(RELEASE_OUT1) $(CXXSTD) $(RELEASE_FLAGS) $(INC)

$(RELEASE_OUT2): $(SRC2) | bin
	$(CXX) $(SRC2) -o $(RELEASE_OUT2) $(CXXSTD) $(RELEASE_FLAGS) $(INC)

# ---- Clean ----
clean:
	rm -f $(DEBUG_OUT1) $(RELEASE_OUT1)
	rm -f $(DEBUG_OUT2) $(RELEASE_OUT2)
	rm -rf bin/

.PHONY: all debug release clean
