# Backlog

Known bugs and improvements that are still open, roughly in priority order within each section. INTENT.md describes the goals these move toward; USAGE.md describes how things behave today.

## Bugs

### A duplicate node id aborts instead of reporting an error
Starting a node whose id is already running fails with `terminate called after throwing an instance of 'Errno_exception'`. `bind()` fails in `Udpsocket::bind_socket()` (`src/udp.cc`), and `check_errno()` throws. The throw happens inside the link-layer thread, but only `main()` has `catch` blocks, so the runtime aborts the process. Every other exception thrown from a layer thread (socket, semaphore and mutex setup, `sendto`, `recvfrom`) ends the same way.

**Fix:** catch exceptions at the top of each layer thread function (`link_layer`, `dlink_layer`, `network_layer` and their send/receive elements), print them with `Exception::printerr()`, and exit with a clear message, for example naming the port that is already in use.

### Output from different threads is mixed together on one line
Every layer's send and receive threads, plus `client2`'s delivery thread, write to `cout` with no coordination. Lines regularly get spliced together, for example `CLIENT: [data received from node ~^D…~ to 34]: three to one`. Now that `client2` is interactive, people read this output directly.

**Fix:** route all output (the `dbglog` macro in `src/debug.hh`, plus the direct `cout` calls in `src/network.cc` and `src/client2.cc`) through one mutex-protected writer that emits whole lines. Printing frames as escaped text instead of raw bytes would also stop terminals from showing garbage.

### One message can be silently lost after a hop gives up
When a frame hits `max_timeouts` without an ACK, `dlink_send_element()` (`src/datalink.cc`) gives up, but the receiver may still be waiting for the old sequence bit. The next frame then carries the same bit the receiver treats as a duplicate: the receiver ACKs and discards it, and the sender believes it was delivered.

**Fix:** keep the alternating bit in step after a give-up. Options include not advancing the sequence number for an abandoned frame, or adding a link reset exchange before the next frame.

### Forwarded data packets never age out
Data packets carry an age field (byte 3, initially 16) meant to work like the IP TTL, but `network_recv_element()` (`src/network.cc`) forwards data packets without decrementing it. With consistent routing tables nothing loops, but a bad topology file (for example, links listed on only one side) can make packets circulate forever.

**Fix:** decrement the age on every forward and drop the packet, with a message, when it reaches 0. LSPs already do this.

## Missing features

### Derive the node count from the topology file
`ext::total_nodes` is fixed at 5 in `src/externals.cc`, so node ids are limited to 0–4. Topologies with fewer nodes work; more do not. The link layer opens one socket per node before the network layer reads `topology.txt`, which is why the count can't simply come from the file today.

**Fix:** parse `topology.txt` in `main()` before starting any layer, set `ext::total_nodes` from the number of rows, and pass the parsed graph to the network layer instead of reading the file twice. While there, validate the file: each link listed on both sides with the same weight, and neighbor ids within range.

### Per-link, runtime-configurable link degradation
Drop and bit-error rates are hard-coded in each program's `main()` (`ext::packet_drop_wt`, `ext::packet_err_wt`). They apply to every link of a node, and they are `char`, so values above 127 overflow. INTENT.md calls for configurable degradation per link.

**Fix:** add optional per-link fault settings to `topology.txt` (or command-line flags as a first step), store them per neighbor, and look them up by sender in `link_recv_element()` (`src/link.cc`). Use `int` for the rates.

### Dynamic link-state routing
Routes come only from the static topology file. The link-state code is there but never runs: `create_lsp()` and `create_hello()` (`src/network.cc`) are never called. If LSPs were sent, three things would go wrong:
- **Flooding goes to every node id, not neighbors.** The loop in `enq_pack()` sends to every id from 0 to `cls.size()-1`.
- **Routes are recomputed without locking.** `cls = dijkistra(...)` in `network_recv_element()` replaces the table without holding `cls_mutex`.
- **The age check never fails.** `r_age >= 0` compares an unsigned value, so it is always true.

**Fix:** have nodes send hellos to find live neighbors and send LSPs at startup and on change, flood only to topology neighbors except the previous hop, and recompute routes under `cls_mutex`. Once that works, nodes can route around a neighbor that has stopped.

### Layers above the network layer
`client2` hands messages straight to the network layer. INTENT.md notes that the transport, session and presentation layers are not modeled. A transport layer with end-to-end acknowledgement and ordering would be the natural next one: today reliability exists only hop by hop.

### Implement or remove `server`
`src/server.cc` is an empty `main()`. Both build systems still build it.

## Build and repository

### Build `client2` in both build systems
`client2` is the program that runs the full stack, but `src/Makefile.am` (`noinst_PROGRAMS = server client`) and `Makefile.x` (`all: server client`) don't build it, so USAGE.md has to give a manual `g++` command. Add it to both, and to the `Makefile.x` dependency block.

### Ignore the `client2` binary
`.gitignore` covers `src/client` and `src/server`, but building `client2` in `src/` (as USAGE.md instructs) leaves an untracked binary. Add `src/client2`.

### Update README.md
README.md still says "As of yet, the project is not working". It does now work end to end; README could say so and link to USAGE.md and INTENT.md.

### Clean up the remaining compiler warnings
With `-std=c++23 -Wall -Wextra -pedantic`, these are left, none of them from toolchain changes:
- **Variable-length arrays:** `link`, `send_window` and `recv_window` in `src/datalink.cc` are sized by `ext::total_nodes`. This is a GCC extension, not standard C++, and clang rejects arrays like these of class types. Use `std::vector`.
- **Catching by value:** `Errno_exception` is caught by value in `client.cc` and `client2.cc`; catch by `const&`.
- **Signed 1-bit bitfields:** the `int PF_bit : 1` fields in `src/hdlcframe.hh` store a set bit as -1. It's harmless today because the value is only tested for truth, but they should be `unsigned`.
- **Small cleanups:** unused parameters, an unused variable `b` in `print_myqueue()`, and the always-true `r_age >= 0` noted above.

## Noticed but not previously raised

These turned up while working through `src/datalink.cc` and weren't in any earlier list.

### The ARQ wait loop spins a CPU core
While waiting for an ACK, `dlink_send_element()` loops on `sem_trywait()` and the timeout check with no sleep, so each node with a frame in flight uses a full core for up to `max_timeouts` × 1 s. Use `sem_timedwait()` with the remaining timeout instead.

### One unresponsive neighbor delays every other link
`dlink_send_element()` runs stop-and-wait for each neighbor in turn, in a single thread. While it retries a dead or lossy neighbor (up to about 5 s per frame), frames queued for every other neighbor wait. Running one send loop per link, or interleaving links, would keep a bad link from stalling the rest.

### Frames queued behind a give-up wait for unrelated traffic
When a hop gives up, the `break` leaves the rest of that neighbor's queue (`link[i]`) untouched. Those frames are only retried the next time any packet is queued for sending, even one for a different neighbor.
