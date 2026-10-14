# Wire

Hola, and welcome!

Wire started life as a university project assignment for an aggressive
computer networks course. The project is intended to be a simulation of
different layers of a computer network stack including a simulation of the
physical layer that introduces random errors in the transmitted data to test
the integrity and performance of the network stack. It is a basic stack
intended to demonstrate computer network concepts and is by no means intended
as a production stack, nor is any comparison with TCP/IP or any other
production stack fair for this project.

## Status

Wire works end to end. You describe a network of up to five nodes in a text
file, start one node per terminal, and type messages addressed to other nodes.
Each message is routed hop by hop across the network while the simulated links
drop and corrupt frames, and the stack detects the damage and retransmits until
the message gets through.

Three layers are implemented so far:

- **Physical:** UDP sockets on localhost, one port per node, with random
  packet drops and bit errors injected on every link
- **Data link:** HDLC-style framing with bit stuffing, a CRC-32 check, and
  stop-and-wait retransmission between neighboring nodes
- **Network:** shortest-path routing (Dijkstra) over the links in the
  topology file

Some planned parts are still missing, such as layers above the network layer,
routes that update while the network is running, and fault settings per link.
BACKLOG.md lists them.

## Quick start

On Linux, from the `src/` directory:

```sh
g++ -I. -D_REENTRANT -DDEBUG_VERSION -pthread -o client2 \
    client2.cc log.cc externals.cc udp.cc error.cc link.cc datalink.cc \
    hdlcframe.cc crc32.cc network.cc fileparser.cc
```

Then start each node of the shipped five-node network in its own terminal,
all from `src/`:

```sh
./client2 0
./client2 1
./client2 2
./client2 3
./client2 4
```

Type `3 hello there` into node 0's terminal, and node 3 prints:

```
CLIENT: [data received from node 0]: hello there
```

## Documentation

- [USAGE.md](USAGE.md): building, the topology file format, running nodes,
  reading the output, adjusting the link faults, and troubleshooting
- [INTENT.md](INTENT.md): what Wire models and why it follows OSI more
  closely than TCP/IP
- [BACKLOG.md](BACKLOG.md): known gaps and planned improvements

## History

This project was originally called NG-DS and was available in an earlier
non-working form at sourceforge. Over the course of various years the latest
revisions of the project were lost, so this incarnation of the project started
from some incomplete code. In 2026 it was brought up to date with current
compilers and libraries, and the bugs that kept it from working were fixed.

Although I mostly work in JavaScript now, I would like if someone posts any
contributions to the project. I anticipate the project will be helpful to
students and hobbyists learning about networks and might be able to provide
useful code snippets and structure hints to them in making their own basic
network stack projects.

Thanks!
