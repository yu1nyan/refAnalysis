OBJDIR ?= objs
DEPDIR ?= $(OBJDIR)/dep
LIBS = `root-config --libs`
OPT ?= 2
DEBUG ?= -g
STD ?= gnu++0x

GENDEPFLAGS = -MMD -MP -MF $(DEPDIR)/$(@F).d
CXXFLAGS += $(CXXDEFS)
CXXFLAGS += -O$(OPT)
CXXFLAGS += -march=native
CXXFLAGS += $(DEBUG)
CXXFLAGS += -Wall -Wextra
CXXFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CXXFLAGS += -std=$(STD)
CXXFLAGS += `root-config --cflags`
MATHLIB = -lm
LIBS += $(MATHLIB)
ALL_CXXFLAGS = $(GENDEPFLAGS) $(CXXFLAGS)
LDFLAGS = $(LIBS)

CXX := g++
LD := g++
REMOVE := rm -f
REMOVEDIR := rm -rf


TARGET = refAnalysis
SRC = refAnalysis.cc
OBJ = $(OBJDIR)/refAnalysis.o

all: $(TARGET)

refAnalysis: $(OBJDIR)/refAnalysis.o
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o : %.cc
	$(CXX) -c $(ALL_CXXFLAGS) $<  -o  $@

clean:
	-$(REMOVE) $(TARGET)
	-$(REMOVE) $(OBJ)
	-$(REMOVEDIR) $(DEPDIR)
	-$(REMOVEDIR) $(OBJDIR)

$(shell mkdir $(OBJDIR) 2>/dev/null)
-include $(shell mkdir $(DEPDIR) 2>/dev/null) $(wildcard $(DEPDIR)/*.d)

.PRECIOUS: $(OBJ)
