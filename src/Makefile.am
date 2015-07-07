# what flags you want to pass to the C compiler & linker
# CFLAGS = --pedantic -Wall -std=c99 -O2
AM_CPPFLAGS = -I. -D_REENTRANT -DDEBUG_VERSION  # -DFILE_DEBUG_VERSION -DFILE_TRACE_VERSION -Wall # -ggdb #-time #-ftime-report
AM_LDFLAGS = -L. -pthread

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
noinst_PROGRAMS = server client
server_SOURCES = server.cc server.hh threads.hh log.cc log.hh externals.cc externals.hh udp.cc udp.hh error.cc error.hh link.cc link.hh datalink.cc datalink.hh hdlcframe.cc hdlcframe.hh crc32.cc crc32.hh network.cc network.hh fileparser.cc fileparser.hh
client_SOURCES = client.cc client.hh threads.hh log.cc log.hh externals.cc externals.hh udp.cc udp.hh error.cc error.hh link.cc link.hh datalink.cc datalink.hh hdlcframe.cc hdlcframe.hh crc32.cc crc32.hh network.cc network.hh fileparser.cc fileparser.hh
