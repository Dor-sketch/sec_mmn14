# Compiler options
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -I$(BOOST_INCLUDE)
LDFLAGS = -L$(BOOST_LIB)

# Boost library options
BOOST_ROOT = /path/to/boost
BOOST_INCLUDE = $(BOOST_ROOT)/include
BOOST_LIB = $(BOOST_ROOT)/lib
BOOST_LIBS = -lboost_system -lboost_filesystem

# Source files
SRCS = main.cpp session.cpp file_handler.cpp message.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable file
EXEC = my_program

# Targets
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(EXEC) $(BOOST_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)
