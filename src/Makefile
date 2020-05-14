CXX			= g++
CXXFLAGS	= -std=c++11 -Wall -O3
RM			= rm -f

HEADER		= $(shell ls | grep -E '\.hpp')
SRC			= $(shell ls | grep -E '\.cc')
OBJ			= $(SRC:%.cc=%.o)
TAR			= $(OBJ:%.o=%)

all : $(TAR)

$(TAR) : % : %.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ) : %.o : %.cc $(HEADER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean :
	$(RM) $(OBJ) $(TAR)