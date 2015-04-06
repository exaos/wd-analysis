# -*- mode: makefile-gmake; coding: utf-8; -*-
#  Created by:  Exaos Lee (exaos.lee@gmail.com)
############################################################

CFLAGS = -fPIC -shared -g -Wall -Wuninitialized
CXXFLAGS = -fPIC -shared -g -Wall -Wuninitialized

# ROOT

ifndef ROOTSYS

ROOT_CONFIG := $(shell which root-config 2> /dev/null)
ifdef ROOT_CONFIG
	ROOTSYS := $(shell root-config --prefix)
else
missroot::
	@echo "..."
	@echo "ROOT system is not found!"
	@echo "..."
endif

else
ROOT_CONFIG := ${ROOTSYS}/bin/root-config
endif

ROOT_CFLAGS := $(shell ${ROOT_CONFIG} --cflags)
ROOT_LIBS   := $(shell ${ROOT_CONFIG} --libs)
ROOT_CINT := $(shell which rootcint)

############################################################
CXXFLAGS += ${ROOT_CFLAGS}
LDFLAGS  += ${ROOT_LIBS}

LIBS := libdigipulse.so

all: ${LIBS}

.PHONY: clean

clean:
	@rm -f *.o ${LIBS}

libdigipulse.so: digipulse.o wdpsel.o # wdpDict.o
	$(CXX) $(CXXFLAGS) -o $@ $^

digipulse.o: digipulse.c digipulse.h

wdpsel.o: wdpsel.C wdpsel.h digipulse.h

wdpDict.o: wdpDict.cpp wdpsel.h

wdpDict.cpp: wdpsel.h linkdef.h
	${ROOT_CINT} -f $@ -c $^

