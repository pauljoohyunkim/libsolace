CXX=g++
INCLUDE=include
EIGEN=/usr/include/eigen3
#EIGEN=C:/msys64/mingw64/include/eigen3
CXXFLAGS=-g -Wall -I$(INCLUDE) -I$(EIGEN)
SRC=src
OBJ=obj
BIN=bin
TESTS=tests

.PHONY: unittest lib

OBJS=$(OBJ)/libsolace.o

DBG_OBJS=$(OBJ)/unittest.o \
		 $(OBJ)/libsolace_dbg.o \
		 $(OBJ)/unittest_qubit.o \
		 $(OBJ)/unittest_gate.o \
		 $(OBJ)/unittest_common_gates.o

objs: $(OBJS)

$(BIN)/libsolace.so: $(OBJS)
	$(CXX) $(CXXFLAGS) -fPIC -shared $^ -o $@

$(OBJ)/%_dbg.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/%_dbg.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/unittest_%.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/unittest_%.o: $(TESTS)/unittest_%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/unittest.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/unittest.o: $(TESTS)/unittest.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -fPIC -shared -c $< -o $@



$(TESTS)/unittest: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(TESTS)/unittest: $(DBG_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ `pkg-config --libs gtest`

unittest: $(TESTS)/unittest

lib: $(BIN)/libsolace.so

clean:
	$(RM) $(OBJ)/*.o $(TESTS)/unittest $(BIN)/*.o $(BIN)/*.so

