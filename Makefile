CXX=g++
INCLUDE=include
CXXFLAGS=-g -Wall -I$(INCLUDE)
SRC=src
OBJ=obj

OBJS=$(OBJ)/libsolace.o

objs: $(OBJS)

$(OBJ)/%.o: $(SRC)/%.cpp $(INCLUDE)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)/*.o
