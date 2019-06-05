TARGET = wgChangeConfig

TARGET_LIB = lib$(TARGET)

CURRENT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
INCDIR = ../../include
OBJDIR = ../../obj
SRCDIR = ../../src
BINDIR = ../../bin

SRCS = $(shell ls $(SRCDIR)/*.cpp)
OBJS = $(subst $(SRCDIR),$(OBJDIR),$(SRCS:.cpp=.o))	

LIB_SRC    = $(TARGET_LIB).cpp
LIB_OBJ    = $(TARGET_LIB).o
HEADER     = $(INCDIR)/$(TARGET).hpp
TARGET_SRC = $(TARGET).cpp
TARGET_OBJ = $(TARGET).o 

ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS   := $(shell root-config --libs)

CXXFLAGS   = -g -O0 -MMD -MP -Wall -fPIC
CXXFLAGS   += -I$(INCDIR) $(ROOTCFLAGS)
CXXLIBS    = -L$(BINDIR) -lSpectrum -lboost_system -lboost_filesystem
CXXLIBS    += $(ROOTLIBS)
DEPENDS    = $(OBJS:.o=.d)
CC = g++
AR = ar

all: $(BINDIR)/$(TARGET_LIB) $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJS) $(LIB_OBJ) $(TARGET_OBJ)
	@if [ ! -d $(BINDIR) ]; \
		then echo "mkdir -p $(BINDIR)"; mkdir -p $(BINDIR); \
	fi
	$(CC) -o $@ $^ $(CXXLIBS)

$(BINDIR)/$(TARGET_LIB): $(OBJS) $(LIB_OBJ)
	$(CC) -shared -Wl,-soname,$(TARGET_LIB).so -o $@.so $^ $(CXXLIBS)
	$(AR) rcs $(BINDIR)/$(TARGET_LIB).a $(OBJS) $(LIB_OBJ)

$(LIB_OBJ): $(LIB_SRC) $(HEADER)
	$(CC) $(CXXFLAGS) -o $@ -c $<

$(TARGET_OBJ): $(TARGET_SRC) $(HEADER)
	$(CC) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	@if [ ! -d $(OBJDIR) ]; \
		then echo "mkdir -p $(OBJDIR)"; mkdir -p $(OBJDIR); \
	fi
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) -f $(BINDIR)/$(TARGET) $(BINDIR)/$(TARGET_LIB)* $(OBJS) $(DEPENDS) *.o

-include $(DEPENDS)

.PHONY: all clean

