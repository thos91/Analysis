SHELL := /bin/bash

ifeq ($(ROOTSYS),)
	ROOT=0
else
	ROOT=1
endif

CC= g++

CXXFLAGS= -O0 -g -Wall -Wno-write-strings -I/opt/pyrame `$(ROOTSYS)/bin/root-config --cflags`

LIBS= -L/opt/pyrame -Wl,-rpath=/opt/pyrame -lecal -lpyrame -lboost_program_options `$(ROOTSYS)/bin/root-config --libs --glibs`

all: om_wagasci

om_wagasci: om_wagasci.C
ifeq ($(ROOT),1)
	$(CC) $(CXXFLAGS) -o om_wagasci om_wagasci.C $(LIBS) -lX11 #-lRHTTP
else
	@echo ROOT IS NOT PRESENT
endif

clean:
	rm -f om_wagasci unit_test_event_loop *~

install: all

uninstall:

unit_test: unit_test_event_loop.cpp om_wagasci.hpp offline_run.sh run_00132_050_dif_1_1_1.raw run_00132_050_dif_1_1_2.raw
	$(CC) $(CXXFLAGS) -o unit_test_event_loop unit_test_event_loop.cpp $(LIBS) -lmapping
	chmod 775 offline_run.sh
	sudo ./offline_run.sh ./
	. $(ROOTSYS)/bin/thisroot.sh && ./unit_test_event_loop
