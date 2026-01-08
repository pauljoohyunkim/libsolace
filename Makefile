CXX=g++
INCLUDE=include
CXXFLAGS=-g -Wall -I$(INCLUDE)
SRC=src
OBJ=obj
TESTS=tests

OBJS=$(OBJ)/libsolace.o
DBG_OBJS=$(OBJ)/unittest.o \
		 $(OBJ)/libsolace_dbg.o \
		 $(OBJ)/unittest_qubit.o

objs: $(OBJS)

$(OBJ)/%_dbg.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/%_dbg.o: $(SRC)/%.cpp $(INCLUDE)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/unittest_%.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/unittest_%.o: $(TESTS)/unittest_%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/unittest.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/unittest.o: $(TESTS)/unittest.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp $(INCLUDE)/%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@



$(TESTS)/unittest: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(TESTS)/unittest: $(DBG_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ `pkg-config --libs gtest`

clean:
	$(RM) $(OBJ)/*.o $(TESTS)/unittest

