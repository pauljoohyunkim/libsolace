CXX=g++
AR=ar
PROTOC=protoc
INCLUDE=include
EIGEN=/usr/include/eigen3
#EIGEN=C:/msys64/mingw64/include/eigen3
OPTIMIZATION?=-O3
override CXXFLAGS+=-g $(OPTIMIZATION) -Wall -I$(INCLUDE) -I$(EIGEN) `pkg-config --cflags protobuf`
LDFLAGS=`pkg-config --libs protobuf`
SRC=src
OBJ=obj
BIN=bin
DOCS=docs
TESTS=tests
DEMOS=demos
PROTO=proto

.PHONY: unittest lib docs demos proto

OBJS=$(OBJ)/solace_proto.o \
	 $(OBJ)/libsolace.o \
	 $(OBJ)/utility.o

DBG_OBJS=$(OBJ)/solace_proto.o \
		 $(OBJ)/unittest.o \
		 $(OBJ)/libsolace_dbg.o \
		 $(OBJ)/utility_dbg.o \
		 $(OBJ)/unittest_qubit.o \
		 $(OBJ)/unittest_gate.o \
		 $(OBJ)/unittest_common_gates.o \
		 $(OBJ)/unittest_utility.o \
		 $(OBJ)/unittest_compilation.o

DEMO_BINS=$(DEMOS)/01_hadamard.bin \
		  $(DEMOS)/02_hadamard2.bin \
		  $(DEMOS)/03_hadamard3.bin \
		  $(DEMOS)/04_grover.bin \
		  $(DEMOS)/05_grover2.bin

objs: $(OBJS)

$(BIN)/libsolace.so: $(OBJS)
	$(CXX) $(CXXFLAGS) -fPIC -shared $^ -o $@

$(BIN)/libsolace.a: $(OBJS)
	$(AR) rcs $@ $^

$(OBJ)/solace_proto.o: $(SRC)/solace.pb.cc
	$(CXX) $(CXXFLAGS) -fPIC -shared -c $< -o $@ $(LDFLAGS)

$(OBJ)/%_dbg.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/%_dbg.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

$(OBJ)/demo_%.o: $(DEMOS)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

$(DEMOS)/%.bin: $(OBJ)/demo_%.o $(BIN)/libsolace.a
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ)/unittest_%.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/unittest_%.o: $(TESTS)/unittest_%.cpp $(PROTOFILES)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

$(OBJ)/unittest.o: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(OBJ)/unittest.o: $(TESTS)/unittest.cpp $(PROTOFILES)
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(PROTOFILES)
	$(CXX) $(CXXFLAGS) -fPIC -shared -c $< -o $@ $(LDFLAGS)


PROTOFILES=$(SRC)/solace.pb.cc $(INCLUDE)/solace.pb.h

proto: $(PROTOFILES)

$(SRC)/solace.pb.cc $(INCLUDE)/solace.pb.h &: $(PROTO)/solace.proto
	$(PROTOC) -I=$(PROTO) --cpp_out=$(SRC) $<
	mv $(SRC)/solace.pb.h $(INCLUDE)/solace.pb.h


$(TESTS)/unittest: CXXFLAGS += -DBE_A_QUANTUM_CHEATER `pkg-config --cflags gtest`
$(TESTS)/unittest: $(DBG_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ `pkg-config --libs gtest` $(LDFLAGS)

unittest: $(TESTS)/unittest

lib: $(BIN)/libsolace.so $(BIN)/libsolace.a

docs: Doxyfile
	doxygen $<

demos: $(DEMO_BINS)

clean:
	$(RM) $(OBJ)/*.o $(TESTS)/unittest $(BIN)/*.o $(BIN)/*.so
	$(RM) -r $(DOCS)/html
	$(RM) $(DEMOS)/*.bin
	$(RM) $(PROTOFILES)
	$(RM) *.qbit *.qgate

