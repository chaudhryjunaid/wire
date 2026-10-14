###############################
# Wire Makefile - version 0.1 #
###############################

#
#.SILENT:
#.IGNORE:
#

#========================
# set the variables here
#========================

# the compiler to be used
CXX = g++

# extra search paths for include files
INCLUDE_PATHS = -I. -Isrc

# extra search paths for libs
LIB_PATHS = -L. -Lsrc

# extra libraries to link
LIBS = -pthread

# symbol defs
DEFS = -D_REENTRANT -DDEBUG_VERSION  # -DFILE_DEBUG_VERSION -DFILE_TRACE_VERSION 

# debug version compiler options
CXX_OPTIONS =  -Wall # -ggdb #-time #-ftime-report
# release version compiler options
#CXX_OPTIONS = -O3 -DNDEBUG -static #-time #-ftime-report

# overall dependencies
GLOBALDEPS =

# the object files the project uses
SERVEROBJS = src/server.o src/log.o src/externals.o src/udp.o src/error.o src/link.o src/datalink.o src/hdlcframe.o src/crc32.o src/network.o src/fileparser.o
CLIENTOBJS = src/client.o src/log.o src/externals.o src/udp.o src/error.o src/link.o src/datalink.o src/hdlcframe.o src/crc32.o src/network.o src/fileparser.o

# devlopment options
CXXFLAGS = $(INCLUDE_PATHS) $(DEFS) $(CXX_OPTIONS)
LIBFLAGS = $(LIB_PATHS) $(LIBS) 


#=====================
# targets to be built
#=====================

all: server client
	@echo '************* Compilation Done ************'

server: $(GLOBALDEPS) $(SERVEROBJS)
	mkdir -p bin && $(CXX) $(CXXFLAGS) $(LIBFLAGS) $(SERVEROBJS)  -o bin/server 

client: $(GLOBALDEPS) $(CLIENTOBJS)
	mkdir -p bin && $(CXX) $(CXXFLAGS) $(LIBFLAGS) $(CLIENTOBJS) -o bin/client

clean:
	rm -f src/*.o
	rm -f src/*.log

distclean: clean
	rm -rf bin

clobber: distclean		#remove all project files except source source and makefiles
	rm -f src/*~		# delete backup files too
	rm -f src/\#*		# delete autosave files too

install: all

query:
	@make -q all || (echo 'At least some targets are out of date.' && /bin/false)
	@echo 'All targets are upto date.'


#===========================================
# the obj files to be built for the targets
#===========================================

.cc.o:  $(GLOBALDEPS)
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

depend: src/*.cc
	makedepend $(INCLUDE_PATHS) $^

# DO NOT DELETE THIS LINE -- make depend needs it

src/client2.o: src/client2.cc src/externals.hh src/debug.hh src/log.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/client.hh \
 src/datalink.hh src/network.hh
src/client.o: src/client.cc src/externals.hh src/debug.hh src/log.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/client.hh \
 src/datalink.hh
src/crc32.o: src/crc32.cc src/crc32.hh
src/datalink.o: src/datalink.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/datalink.hh \
 src/hdlcframe.hh
src/debug.o: src/debug.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh
src/error.o: src/error.cc src/error.hh
src/externals.o: src/externals.cc src/externals.hh src/debug.hh \
 src/log.hh src/error.hh src/link.hh src/udp.hh src/threads.hh
src/fileparser.o: src/fileparser.cc src/fileparser.hh
src/hdlcframe.o: src/hdlcframe.cc src/debug.hh src/log.hh \
 src/externals.hh src/error.hh src/link.hh src/udp.hh src/threads.hh \
 src/hdlcframe.hh src/crc32.hh
src/link.o: src/link.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/fileparser.hh \
 src/hdlcframe.hh
src/log.o: src/log.cc src/log.hh
src/network.o: src/network.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/datalink.hh \
 src/fileparser.hh
src/server.o: src/server.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/server.hh
src/udp.o: src/udp.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh
