# Compiler options
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -g -I$(BOOST_INCLUDE)  # Added -g here
LDFLAGS = -L$(BOOST_LIB)

# Boost library options
BOOST_ROOT = /path/to/boost
BOOST_INCLUDE = $(BOOST_ROOT)/include
BOOST_LIB = $(BOOST_ROOT)/lib
BOOST_LIBS = -lboost_system -lboost_filesystem

# Source files
SRCS = main.cpp Session.cpp FileHandler.cpp Message.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable file
EXEC = server_app

# Targets
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(EXEC) $(BOOST_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
	rm -rf ./backupsvr
	@if [ -f temp.pdf ]; then rm temp.pdf; fi
