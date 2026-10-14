# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Wire (formerly NG-DS, a 2007 university project) is a C++/pthreads simulation of a layered network stack: a simulated physical layer over UDP that injects random drops and bit errors, an HDLC-style datalink with CRC-32 and stop-and-wait ARQ, and a link-state network layer using Dijkstra routing. README.md still calls the project not working. Since the 2026 bitrot and logic fixes, `client2` delivers messages end to end across the topology, but parts of the intent are unfinished. There are no tests and no lint configuration.

Read `INTENT.md` for the project's design goals (OSI-first layering, UDP modeled as the physical layer, configurable link degradation) and the known gaps between those goals and the current code. Keep changes aligned with it.

`USAGE.md` is the user-facing guide to building, running and troubleshooting a network of nodes. Update it whenever command-line usage, `topology.txt` rules, output messages or fault settings change.

## Building

The autotools build is the intended path (requires autoconf/automake):

```sh
./autogen.sh && ./configure && make      # produces src/server and src/client
```

`Makefile.x` is the legacy hand-written Makefile (`make -f Makefile.x`, binaries in `bin/`). Its dependency block (below `# DO NOT DELETE`) lists project headers only and was generated with `g++ -MM`. `make -f Makefile.x depend` runs `makedepend`, which also writes absolute system-header paths; those break the build whenever glibc removes a header, as happened with `xlocale.h` and `rpc/netdb.h`.

The autotools build compiles with `-g -O2`, but `Makefile.x` and the direct `g++` command pass no `-O`, and for unoptimized builds GCC 15+ libstdc++ enables `_GLIBCXX_ASSERTIONS`. Out-of-range `std::string`/container access then aborts with `Assertion '__pos <= size()' failed` instead of silently reading garbage.

Building directly without either build system works (run from `src/`):

```sh
g++ -I. -D_REENTRANT -DDEBUG_VERSION -pthread client2.cc \
    log.cc externals.cc udp.cc error.cc link.cc datalink.cc hdlcframe.cc crc32.cc network.cc fileparser.cc \
    -o client2
```

## Entry points

- `src/client2.cc` is the only `main` that brings up all three layers (link → datalink → network). It is **not** listed in `src/Makefile.am` or `Makefile.x`, so build it manually as above. Usage: `./client2 <my_node_id>`, then type one message per line as `<dest_node_id> <message>`. Any node can send to any node, and a separate thread prints each delivery with its source node. At end of input the node keeps forwarding and receiving (it may be a hop on other routes) until Ctrl-C. For scripted tests, pipe each node's input (`printf "3 hello\n" | ./client2 0`) and stop the processes with `timeout`.
- `src/client.cc` brings up link + datalink only (no routing). Usage: `./client <my_node_id> <dest_node_id> <total_nodes>` (all three args required). Node 0 reads lines from stdin and sends them straight to the destination over one link; other nodes print what they receive.
- `src/server.cc` is an empty stub.

Run one process per node, from `src/`, because `init_network_layer()` opens `topology.txt` relative to the CWD. Node N binds UDP `localhost.localdomain:50000+N` and sends to node M at port `50000+M`.

## Architecture

**Layer/thread model.** Each layer exposes a `*_layer(void*)` pthread entry point (`link_layer`, `dlink_layer`, `network_layer`). It runs `init_*_layer()`, spawns two detached threads (`*_send_element` and `*_recv_element`), then exits. `main` creates each layer thread and `pthread_join`s it purely to wait for initialization before starting the next layer. Bring layers up bottom-up.

**Inter-layer queues.** Layers talk only through global triples of `std::queue` + `pthread_mutex_t` + `sem_t` (the semaphore is posted once per push). They are declared `extern` in `externals.hh` and defined in the `.cc` of the layer that owns them:

```
app ─nl_send_queue─▶ network ─dlink_send_queue─▶ datalink ─link_send_queue─▶ link ─UDP─▶
app ◀nl_recv_queue── network ◀dlink_recv_queue── datalink ◀link_recv_queue── link ◀UDP──
```

The datalink also keeps private `ack_send_queue`/`ack_recv_queue` so the recv element can hand ACKs to the send element.

**One struct for every layer.** `dgram` (in `link.hh`, aliased as `packet`) is `{char address; string frame;}` everywhere. `address` means *destination / next hop* on the send path and *sender / previous hop* on the receive path. `link_send_element` appends the sender's address byte after the closing HDLC flag, and `link_recv_element` reads it back from there.

**Link layer (`link.cc`).** This is where errors are injected. For each received datagram it drops it with probability 1/`packet_drop_wt`, or flips one bit in a random byte between the frame's flags (within the first `max_drop_idx` bytes) with probability 1/`packet_err_wt`. A weight of 0 disables that kind of fault. Both drivers set both weights to 15 and seed `rand()` with time and PID, so nodes started together don't share fault patterns.

**Datalink (`datalink.cc`, `hdlcframe.cc`, `crc32.cc`).** `frame()` builds `addr | 2-byte control | payload | CRC-32`, then bit-stuffs it and wraps it in `0x7e` flags. `deframe()` reverses this and returns non-zero on CRC failure. Control fields are bitfield unions (`mk_ictrl`/`mk_sctrl`, `bk_ictrl`/`bk_sctrl`). ARQ is stop-and-wait with a 1-bit sequence number per destination, busy-polling for ACKs until `ext::to_val` times out, giving up after `ext::max_timeouts` attempts.

**Network (`network.cc`).** The first payload byte is the packet type:
- 0 = data `[type, src, dest, age, payload…]`
- 1 = hello `[type, node]`
- 2 = LSP `[type, node, seq, age, (neighbor, weight)…]`

The initial graph comes from `topology.txt` through `File_parser`. `dijkistra()` builds the routing table `cls` (a vector of `{dest, weigh, nhop}`). LSPs are flooded to every node except the previous hop.

**`topology.txt` format.** One row per node: `node  neigh weight  neigh weight … .`, where each row must end with a `.` token and `#` starts a comment. The leading node-id token is discarded, so **row order defines node ids**. `ext::total_nodes` (default 5) must match the row count.

**Globals.** Runtime parameters live in `namespace ext` (`externals.cc`): `total_nodes`, `my_address`, `packet_drop_wt`, `packet_err_wt`, `max_drop_idx`, `max_timeouts`, `to_val`, `time_to_quit`.

## Conventions

- **Debug macros (`debug.hh`).** `dbglog(x)`, `dbgcode(code)`, and the `fin()`/`fout()` function-trace macros compile to nothing unless `DEBUG_VERSION` is defined (both build systems define it) **and** the file has `#define DEBUG_THIS_FILE` before including `debug.hh`. Currently `link.cc`, `datalink.cc` and `network.cc` opt in, so they print a lot to stdout. Defining `FILE_DEBUG_VERSION` or `FILE_TRACE_VERSION` redirects the output to `program.debug.log` / `program.trace.log`.
- **Errors (`error.hh`).** Wrap syscall return values with `check_errno(rv, "where")` (throws `Errno_exception` on -1) or `checkval_0(rv, Exception("where", "msg"))` (throws on non-zero). The `main`s catch `Errno_exception`, `int` and `char*`.
- **Headers are not self-contained.** For example, `link.hh` uses `string` without including it, and `externals.hh` (no include guard) and `debug.hh` include each other. Keep the existing pattern of including `debug.hh` first (after any `DEBUG_THIS_FILE`), or compilation breaks.
