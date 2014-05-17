# MAKEFILE for Eclipse!
# If you are building from cmdline, set CXXFLAGS=-g -Wall -O0 and BUILDCFG=Debug64bit
# -----------
# NOTE this only builds on Windows, so no need for linux details

TARGET = cube2omx
LIBS = hdf5_hl hdf5 z

# Be sure we have a valid build configuration
ifndef BUILDCFG
  BUILDCFG=Debug
endif
ifndef CXXFLAGS
  CXXFLAGS=-g3 -Wall -Wno-write-strings
endif

SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp, %.o, $(SOURCES))

LDLIBS := $(addprefix -l,$(LIBS))

EXE := $(addprefix $(TARGET), .exe)
SHELL=cmd.exe

BDDIR := $(shell if not exist $(BUILDCFG) mkdir $(BUILDCFG))

#----
OBJDIR = $(BUILDCFG)
OBJEXE = $(addprefix $(OBJDIR)/, $(TARGET).exe)
OBJFLAGS = -static-libgcc

all: $(OBJEXE)

clean:
	rmdir /s /q $(BUILDCFG)

$(OBJDIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(EXTRAFLAGS) -c $< -o $@

$(OBJEXE): $(addprefix $(OBJDIR)/, $(OBJECTS))
	$(CXX) $(OBJFLAGS) $^ $(LDLIBS) -o $@
	$(BINCMD)
