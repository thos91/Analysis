TARGET = wgHough

INCLUDE = -I ../../include
OBJDIR = ../../obj
SRCDIR = ../../src
BINDIR = ../../bin

SRCS = $(shell ls $(SRCDIR)/*.cpp)
OBJS = $(subst $(SRCDIR),$(OBJDIR),$(SRCS:.cpp=.o))	

SRCS += $(TARGET).cpp
OBJS += $(TARGET).o

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs)
ROOTGLIBS = $(shell root-config --glibs)

CXXFLAGS   = -g -MMD -MP -Wall -fPIC 
CXXFLAGS   += $(INCLUDE) $(ROOTCFLAGS)
CXXLIBS    = -lSpectrum
CXXLIBS    += $(INCLUDE) $(ROOTLIBS)
DEPENDS = $(OBJS:.o=.d)
CC = g++

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJS)
	$(CC) $(CXXLIBS) -o $@ $^

$(OBJDIR)/%.o:$(SRCDIR)/%.cpp
	@if [ ! -d $(OBJDIR) ]; \
		then echo "mkdir -p $(OBJDIR)"; mkdir -p $(OBJDIR); \
		fi
	$(CC) $(CXXFLAGS) $(INCLUDE) -o $@ -c $<

clean:
	$(RM) -f $(BINDIR)/$(TARGET) $(OBJS) $(DEPENDS)

-include $(DEPENDS)

.PHONY: all clean

