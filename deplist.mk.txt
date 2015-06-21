client2.o: client2.cc externals.hh debug.hh log.hh error.hh link.hh \
  udp.hh threads.hh client.hh datalink.hh network.hh
client.o: client.cc externals.hh debug.hh log.hh error.hh link.hh udp.hh \
  threads.hh client.hh datalink.hh
crc32.o: crc32.cc crc32.hh
datalink.o: datalink.cc debug.hh log.hh externals.hh error.hh link.hh \
  udp.hh threads.hh datalink.hh hdlcframe.hh
debug.o: debug.cc debug.hh log.hh externals.hh error.hh link.hh udp.hh \
  threads.hh
error.o: error.cc error.hh
externals.o: externals.cc log.hh
fileparser.o: fileparser.cc fileparser.hh
hdlcframe.o: hdlcframe.cc debug.hh log.hh externals.hh error.hh link.hh \
  udp.hh threads.hh hdlcframe.hh crc32.hh
link.o: link.cc debug.hh log.hh externals.hh error.hh link.hh udp.hh \
  threads.hh fileparser.hh hdlcframe.hh
log.o: log.cc log.hh
network.o: network.cc debug.hh log.hh externals.hh error.hh link.hh \
  udp.hh threads.hh datalink.hh fileparser.hh
server.o: server.cc debug.hh log.hh externals.hh error.hh link.hh udp.hh \
  threads.hh server.hh
udp.o: udp.cc debug.hh log.hh externals.hh error.hh link.hh udp.hh \
  threads.hh
