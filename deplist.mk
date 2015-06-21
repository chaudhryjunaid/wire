client.o: src/client.cc src/externals.hh src/debug.hh src/log.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/client.hh \
 src/datalink.hh
client2.o: src/client2.cc src/externals.hh src/debug.hh src/log.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/client.hh \
 src/datalink.hh src/network.hh
crc32.o: src/crc32.cc src/crc32.hh
datalink.o: src/datalink.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/datalink.hh \
 src/hdlcframe.hh
debug.o: src/debug.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh
error.o: src/error.cc src/error.hh
externals.o: src/externals.cc src/externals.hh src/debug.hh src/log.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh
fileparser.o: src/fileparser.cc src/fileparser.hh
hdlcframe.o: src/hdlcframe.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/hdlcframe.hh \
 src/crc32.hh
link.o: src/link.cc src/debug.hh src/log.hh src/externals.hh src/error.hh \
 src/link.hh src/udp.hh src/threads.hh src/fileparser.hh src/hdlcframe.hh
log.o: src/log.cc src/log.hh
network.o: src/network.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/datalink.hh \
 src/fileparser.hh
server.o: src/server.cc src/debug.hh src/log.hh src/externals.hh \
 src/error.hh src/link.hh src/udp.hh src/threads.hh src/server.hh
udp.o: src/udp.cc src/debug.hh src/log.hh src/externals.hh src/error.hh \
 src/link.hh src/udp.hh src/threads.hh
