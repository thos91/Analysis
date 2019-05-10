TARGET = wgPreCalib

INCDIR = -I ../../include
OBJDIR = ../../obj
SRCDIR = ../../src
BINDIR = ../../bin

SRCS = $(shell ls $(SRCDIR)/*.cc)
OBJS = $(subst $(SRCDIR),$(OBJDIR),$(SRCS:.cc=.o))	

SRCS += $(TARGET).cc
OBJS += $(TARGET).o

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --libs)
ROOTGLIBS = $(shell root-config --glibs)

CXXFLAGS   = -g -O0 -MMD -MP -Wall -fPIC
CXXFLAGS   += $(INCDIR) $(ROOTCFLAGS)
CXXLIBS    = -lSpectrum
CXXLIBS    += $(ROOTLIBS) $(ROOTGLIBS)
DEPENDS    = $(OBJS:.o=.d)
CC = g++

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJS)
	@if [ ! -d $(BINDIR) ]; \
		then echo "mkdir -p $(BINDIR)"; mkdir -p $(BINDIR); \
		fi
	$(CC) -o $@ $^ $(CXXLIBS)

$(OBJDIR)/%.o:$(SRCDIR)/%.cc
	@if [ ! -d $(OBJDIR) ]; \
		then echo "mkdir -p $(OBJDIR)"; mkdir -p $(OBJDIR); \
		fi
	$(CC) $(CXXFLAGS) $(INCDIR) -o $@ -c $<

clean:
	$(RM) -f $(BINDIR)/$(TARGET) $(OBJS) $(DEPENDS) AutoDict*

-include $(DEPENDS)

.PHONY: all clean

