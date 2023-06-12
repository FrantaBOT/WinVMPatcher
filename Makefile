CXX=g++
CXXFLAGS=-I./memflow/memflow-ffi/ -L./memflow/target/release -Wno-multichar
LIBS=-lm -Wl,--no-as-needed -ldl -lpthread -l:libmemflow_ffi.a

OUTDIR=./build
OBJDIR=$(OUTDIR)/obj

$(shell mkdir -p $(OBJDIR))

%.o: %.cpp
	$(CXX) -c -o $(OBJDIR)/$@ $< $(CXXFLAGS)

winvmpatcher: winvmpatcher.o
	$(CXX) -o $(OUTDIR)/$@ $(OBJDIR)/winvmpatcher.o $(CXXFLAGS) $(LIBS)

.PHONY: all
all: winvmpatcher

.DEFAULT_GOAL := all

clean:
	rm -rf $(OUTDIR)
