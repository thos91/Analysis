ANALYSIS_DIR = ../..

CXX = g++
CXX_FLAGS = -g -O0 -I$(ANALYSIS_DIR)/include
ROOT_FLAGS := `root-config --cflags`
LIB_FLAGS = -L$(ANALYSIS_DIR)/lib -Wl,-rpath,$(ANALYSIS_DIR)/lib
WAGASCI_LIBS = -lwagasci
ROOT_LIBS := `root-config --libs`

all:
	$(CXX) $(ROOT_FLAGS) $(CXX_FLAGS) utConst.cpp -o utConst $(LIB_FLAGS) $(ROOT_LIBS) $(WAGASCI_LIBS)
	$(CXX) $(ROOT_FLAGS) $(CXX_FLAGS) utTopology.cpp -o utTopology $(LIB_FLAGS) $(ROOT_LIBS) $(WAGASCI_LIBS)

clean:
	$(RM) -rf utConst utTopology
