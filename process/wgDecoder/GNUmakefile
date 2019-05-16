TARGET = wgDecoder

TARGET_LIB = lib$(TARGET).so

CURRENT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
INCDIR = ../../include
OBJDIR = ../../obj
SRCDIR = ../../src
BINDIR = ../../bin

SRCS = $(shell ls $(SRCDIR)/*.cc)
OBJS = $(subst $(SRCDIR),$(OBJDIR),$(SRCS:.cc=.o))	

LIB_SRC    = $(TARGET_LIB:.so=.cc)
LIB_OBJ    = $(TARGET_LIB:.so=.o)
HEADER     = $(TARGET).hpp
TARGET_SRC = $(TARGET).cc
TARGET_OBJ = $(TARGET).o 

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs)

CXXFLAGS   = -g -O0 -MMD -MP -Wall -fPIC
CXXFLAGS   += -I$(INCDIR) $(ROOTCFLAGS)
CXXLIBS    = -L$(BINDIR) -lSpectrum
CXXLIBS    += $(ROOTLIBS)
DEPENDS    = $(OBJS:.o=.d)
CC = g++

all: $(BINDIR)/$(TARGET_LIB) $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJS) $(TARGET_OBJ)
	@if [ ! -d $(BINDIR) ]; \
		then echo "mkdir -p $(BINDIR)"; mkdir -p $(BINDIR); \
	fi
	$(CC) -o $@ $^ -Wl,-rpath,$(CURRENT_DIR)/$(BINDIR) $(CXXLIBS) -l$(TARGET)

$(BINDIR)/$(TARGET_LIB): $(OBJS) $(LIB_OBJ)
	$(CC) -shared -Wl,-soname,$(TARGET_LIB) -o $@ $^ $(CXXLIBS)

$(LIB_OBJ): $(LIB_SRC) $(HEADER)
	$(CC) $(CXXFLAGS) -o $@ -c $<

$(TARGET_OBJ): $(TARGET_SRC) $(HEADER)
	$(CC) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.o:$(SRCDIR)/%.cc
	@if [ ! -d $(OBJDIR) ]; \
		then echo "mkdir -p $(OBJDIR)"; mkdir -p $(OBJDIR); \
	fi
	$(CC) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) -f $(BINDIR)/$(TARGET) $(BINDIR)/$(TARGET_LIB) $(OBJS) $(DEPENDS) *.o

-include $(DEPENDS)

.PHONY: all clean

