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
INCLUDE_PATHS = .

# extra search paths for libs
LIB_PATHS = .

# extra libraries to link
LIBS = pthread

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
CXXFLAGS = -I$(INCLUDE_PATHS) $(DEFS) $(CXX_OPTIONS)
LIBFLAGS = -L$(LIB_PATHS) -l$(LIBS) 


#=====================
# targets to be built
#=====================

all: server client
	@echo '************* Compilation Done ************'

# update dependency list
deplist.mk: src/*.cc src/*.hh
	g++ -MM src/*.cc > deplist.mk

server: $(GLOBALDEPS) $(SERVEROBJS)
	$(CXX) $(CXXFLAGS) $(LIBFLAGS) $(SERVEROBJS)  -o server 

client: $(GLOBALDEPS) $(CLIENTOBJS)
	$(CXX) $(CXXFLAGS) $(LIBFLAGS) $(CLIENTOBJS) -o client

run: client
	@echo 'make: Invoking client executable...'
	@./client
	@echo 'make: Client finished at' `date`

clean:
	rm -f src/*.o
	rm -f src/*.log

distclean: clean
	rm -f server client

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
	$(CXX) $(CXXFLAGS) -c $<

include deplist.mk
