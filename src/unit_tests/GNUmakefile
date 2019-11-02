ANALYSIS_DIR = ../..

CXX = g++
CXX_FLAGS = -g -O0 -I$(ANALYSIS_DIR)/include
ROOT_FLAGS := `root-config --cflags`
LIB_FLAGS = -L$(ANALYSIS_DIR)/lib -Wl,-rpath,$(ANALYSIS_DIR)/lib
WAGASCI_LIBS = -lwagasci
ROOT_LIBS := `root-config --libs`

all: Const Topology RawData

Const:
	$(CXX) $(ROOT_FLAGS) $(CXX_FLAGS) utConst.cpp -o utConst $(LIB_FLAGS) $(ROOT_LIBS) $(WAGASCI_LIBS)

Topology:
	$(CXX) $(ROOT_FLAGS) $(CXX_FLAGS) utTopology.cpp -o utTopology $(LIB_FLAGS) $(ROOT_LIBS) $(WAGASCI_LIBS)

RawData:
	$(CXX) $(ROOT_FLAGS) $(CXX_FLAGS) utRawData.cpp -o utRawData $(LIB_FLAGS) $(ROOT_LIBS) $(WAGASCI_LIBS)

EditXML:
	$(CXX) $(ROOT_FLAGS) $(CXX_FLAGS) utEditXML.cpp -o utEditXML $(LIB_FLAGS) $(ROOT_LIBS) $(WAGASCI_LIBS)

clean:
	$(RM) -rf utConst utTopology utRawData
