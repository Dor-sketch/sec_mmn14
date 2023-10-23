# Compiler options
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g -I$(BOOST_INCLUDE)
LDFLAGS = -L$(BOOST_LIB) -L$(FMT_LIB)

# Boost library options
BOOST_ROOT = /path/to/boost
BOOST_INCLUDE = $(BOOST_ROOT)/include
BOOST_LIB = $(BOOST_ROOT)/lib
BOOST_LIBS = -lboost_system -lboost_filesystem

# FMT library options (if needed)
FMT_LIB = /path/to/fmt/lib
FMT_LIBS = -lfmt

# Source files
SRCS = main.cpp Session.cpp FileHandler.cpp Message.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable file
EXEC = server_app

# Targets
.PHONY: all debug clean

all: $(EXEC)

debug: CXXFLAGS += -DENABLE_DEBUG_LOGGING
debug: all

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(EXEC) $(BOOST_LIBS) $(FMT_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
	rm -rf ./backupsvr
	@if [ -f temp.pdf ]; then rm temp.pdf; fi
