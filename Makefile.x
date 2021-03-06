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

src/hdlcframe.o: src/debug.hh src/log.hh src/externals.hh src/error.hh
src/hdlcframe.o: /usr/include/errno.h /usr/include/features.h
src/hdlcframe.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/hdlcframe.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/hdlcframe.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/hdlcframe.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/hdlcframe.o: /usr/include/unistd.h /usr/include/getopt.h src/threads.hh
src/hdlcframe.o: /usr/include/pthread.h /usr/include/sched.h
src/hdlcframe.o: /usr/include/time.h /usr/include/semaphore.h
src/hdlcframe.o: src/hdlcframe.hh src/crc32.hh
src/log.o: src/log.hh
src/crc32.o: src/crc32.hh
src/client.o: /usr/include/getopt.h src/externals.hh src/debug.hh src/log.hh
src/client.o: src/error.hh /usr/include/errno.h /usr/include/features.h
src/client.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/client.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/client.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/client.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/client.o: /usr/include/unistd.h src/threads.hh /usr/include/pthread.h
src/client.o: /usr/include/sched.h /usr/include/time.h
src/client.o: /usr/include/semaphore.h src/client.hh src/datalink.hh
src/fileparser.o: src/fileparser.hh
src/debug.o: src/debug.hh src/log.hh src/externals.hh src/error.hh
src/debug.o: /usr/include/errno.h /usr/include/features.h
src/debug.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/debug.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/debug.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/debug.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/debug.o: /usr/include/unistd.h /usr/include/getopt.h src/threads.hh
src/debug.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/time.h
src/debug.o: /usr/include/semaphore.h
src/client2.o: src/externals.hh src/debug.hh src/log.hh src/error.hh
src/client2.o: /usr/include/errno.h /usr/include/features.h
src/client2.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/client2.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/client2.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/client2.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/client2.o: /usr/include/unistd.h /usr/include/getopt.h src/threads.hh
src/client2.o: /usr/include/pthread.h /usr/include/sched.h
src/client2.o: /usr/include/time.h /usr/include/semaphore.h src/client.hh
src/client2.o: src/datalink.hh src/network.hh
src/externals.o: src/externals.hh src/debug.hh src/log.hh src/error.hh
src/externals.o: /usr/include/errno.h /usr/include/features.h
src/externals.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/externals.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/externals.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/externals.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/externals.o: /usr/include/unistd.h /usr/include/getopt.h src/threads.hh
src/externals.o: /usr/include/pthread.h /usr/include/sched.h
src/externals.o: /usr/include/time.h /usr/include/semaphore.h
src/network.o: src/debug.hh src/log.hh src/externals.hh src/error.hh
src/network.o: /usr/include/errno.h /usr/include/features.h
src/network.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/network.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/network.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/network.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/network.o: /usr/include/unistd.h /usr/include/getopt.h src/threads.hh
src/network.o: /usr/include/pthread.h /usr/include/sched.h
src/network.o: /usr/include/time.h /usr/include/semaphore.h src/datalink.hh
src/network.o: src/fileparser.hh
src/server.o: src/debug.hh src/log.hh src/externals.hh src/error.hh
src/server.o: /usr/include/errno.h /usr/include/features.h
src/server.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/server.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/server.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/server.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/server.o: /usr/include/unistd.h /usr/include/getopt.h src/threads.hh
src/server.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/time.h
src/server.o: /usr/include/semaphore.h src/server.hh
src/datalink.o: src/debug.hh src/log.hh src/externals.hh src/error.hh
src/datalink.o: /usr/include/errno.h /usr/include/features.h
src/datalink.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/datalink.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/datalink.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/datalink.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/datalink.o: /usr/include/unistd.h /usr/include/getopt.h src/threads.hh
src/datalink.o: /usr/include/pthread.h /usr/include/sched.h
src/datalink.o: /usr/include/time.h /usr/include/semaphore.h src/datalink.hh
src/datalink.o: src/hdlcframe.hh
src/udp.o: /usr/include/string.h /usr/include/features.h
src/udp.o: /usr/include/stdc-predef.h /usr/include/xlocale.h src/debug.hh
src/udp.o: src/log.hh src/externals.hh src/error.hh /usr/include/errno.h
src/udp.o: src/link.hh src/udp.hh /usr/include/netinet/in.h
src/udp.o: /usr/include/stdint.h /usr/include/endian.h
src/udp.o: /usr/include/arpa/inet.h /usr/include/netdb.h
src/udp.o: /usr/include/rpc/netdb.h /usr/include/unistd.h
src/udp.o: /usr/include/getopt.h src/threads.hh /usr/include/pthread.h
src/udp.o: /usr/include/sched.h /usr/include/time.h /usr/include/semaphore.h
src/link.o: src/debug.hh src/log.hh src/externals.hh src/error.hh
src/link.o: /usr/include/errno.h /usr/include/features.h
src/link.o: /usr/include/stdc-predef.h src/link.hh src/udp.hh
src/link.o: /usr/include/netinet/in.h /usr/include/stdint.h
src/link.o: /usr/include/endian.h /usr/include/arpa/inet.h
src/link.o: /usr/include/netdb.h /usr/include/rpc/netdb.h
src/link.o: /usr/include/unistd.h /usr/include/getopt.h src/threads.hh
src/link.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/time.h
src/link.o: /usr/include/semaphore.h src/fileparser.hh src/hdlcframe.hh
src/error.o: src/error.hh /usr/include/errno.h /usr/include/features.h
src/error.o: /usr/include/stdc-predef.h
