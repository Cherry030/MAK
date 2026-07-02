#gmake
 
# To Use this, you need to do a make clean, then make PROT=1 where PROT is DIS HLA13 HLA1516 or HLA1516E
SHELL   = /bin/bash
FILES_H = $(wildcard Files-h.txt)
FILES_C = $(wildcard Files-cc.txt)
SRC = $(shell cat $(FILES_C))
OBJS = $(addsuffix .o, $(basename $(SRC)))
 
 
CXX=g++
LIBNAME = libdstAccessory
CXXFLAGS=-I.  -fPIC  -m64  -I../../include -I$(MAK_VRLDIR)/include -O3 -fpermissive
HLA13_CXXFLAGS= $(CXXFLAGS)  -I$(MAK_RTIDIR)/include  -I$(MAK_RTIDIR)/include/HLA13  \
		-DDtHLA=1 -DRTI_USES_STD_FSTREAM=1 -D_PTHREADS -D_POSIX_PTHREAD_SEMANTICS
HLA1516_CXXFLAGS= $(CXXFLAGS)  -I$(MAK_RTIDIR)/include  -I$(MAK_RTIDIR)/include/HLA1516 \
		  -DDtHLA=1 -DDtHLA_1516=1 -DRTI_USES_STD_FSTREAM=1 -D_PTHREADS -D_POSIX_PTHREAD_SEMANTICS
HLA1516E_CXXFLAGS= $(CXXFLAGS)  -I$(MAK_RTIDIR)/include -I$(MAK_RTIDIR)/include/HLA1516E  \
	       -DDtHLA=1 -DDtHLA_1516=1 -DDtHLA_1516_EVOLVED=1 -DRTI_USES_STD_FSTREAM=1 -D_PTHREADS -D_POSIX_PTHREAD_SEMANTICS
DIS_CXXFLAGS=$(CXXFLAGS) -m64 -DDtDIS
 
TOTAL_CXX_FLAGS = "Undefined Protocol"

ifdef HLA13
  TOTAL_CXX_FLAGS = $(HLA13_CXXFLAGS)
  REAL_LIB_NAME = $(LIBNAME)HLA13.so
endif
ifdef HLA1516
  TOTAL_CXX_FLAGS = $(HLA1516_CXXFLAGS)
  REAL_LIB_NAME = $(LIBNAME)HLA1516.so
endif
ifdef HLA1516E
  TOTAL_CXX_FLAGS = $(HLA1516E_CXXFLAGS)
  REAL_LIB_NAME = $(LIBNAME)HLA1516E.so
endif
 
 
all:: $(REAL_LIB_NAME)
 
$(REAL_LIB_NAME): $(OBJS)
	$(CXX) -m64 -shared  -Wl,-rpath,../lib64  -Wl,-rpath,. -Wl,-rpath-link,../../lib64  -o  $(REAL_LIB_NAME) $(TOTAL_LD_FLAGS) $(OBJS)
 
%.o: %.cxx
	$(CXX) -m64 -c $^  $(TOTAL_CXX_FLAGS) -o $@
 
clean:
	rm -f *.o 
