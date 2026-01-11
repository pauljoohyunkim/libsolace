CXX=g++
AR=ar
INCLUDE=include
EIGEN=/usr/include/eigen3
#EIGEN=C:/msys64/mingw64/include/eigen3
override CXXFLAGS+=-g -Wall -I$(INCLUDE) -I$(EIGEN)
SRC=src
OBJ=obj
BIN=bin
DOCS=docs
TESTS=tests

.PHONY: unittest lib docs

OBJS=$(OBJ)/libsolace.o \
	 $(OBJ)/utility.o

DBG_OBJS=$(OBJ)/unittest.o \
		 $(OBJ)/libsolace_dbg.o \
		 $(OBJ)/utility_dbg.o \
		 $(OBJ)/unittest_qubit.o \
		 $(OBJ)/unittest_gate.o \
		 $(OBJ)/unittest_common_gates.o \
		 $(OBJ)/unittest_utility.o

objs: $(OBJS)

$(BIN)/libsolace.so: $(OBJS)
	$(CXX) $(CXXFLAGS) -fPIC -shared $^ -o $@

$(BIN)/libsolace.a: $(OBJS)
	$(AR) rcs $@ $^

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

lib: $(BIN)/libsolace.so $(BIN)/libsolace.a

docs: Doxyfile
	doxygen $<

clean:
	$(RM) $(OBJ)/*.o $(TESTS)/unittest $(BIN)/*.o $(BIN)/*.so
	$(RM) -r $(DOCS)/html

