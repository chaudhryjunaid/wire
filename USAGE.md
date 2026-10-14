# Using Wire

This guide shows how to build Wire, describe a network, bring up its nodes, and send messages between them over deliberately unreliable links. For what the project is trying to model, and why, see INTENT.md.

Wire runs on Linux. You need `g++` with pthreads, plus `autoconf` and `automake` if you want the autotools build.

## 1. Build the node program

Each node runs `client2`, the program that brings up the full stack (physical, data link and network layers). Neither build system builds it yet, so compile it directly from `src/`:

```sh
cd src
g++ -I. -D_REENTRANT -DDEBUG_VERSION -pthread -o client2 \
    client2.cc log.cc externals.cc udp.cc error.cc link.cc datalink.cc \
    hdlcframe.cc crc32.cc network.cc fileparser.cc
```

`-DDEBUG_VERSION` makes every layer print what it is doing, including each simulated drop and bit error. To get quieter output, leave it out (see [Quieter output](#quieter-output)).

## 2. Describe the network

`src/topology.txt` defines the nodes and the links between them. The shipped file describes this network (shown here with its comments trimmed):

```
#  Node  Neigh/Weigh Neigh/Weigh Neigh/Weigh
    0      1  1        2  10              .
    1      0  1        3  20      4  2    .
    2      3  2        0  10              .
    3      2  2        1  20      4  3    .
    4      3  3        1  2               .
```

```
        1         2
   0 ─────── 1 ─────── 4
   │         │         │
 10│       20│         │3
   │         │         │
   2 ─────── 3 ────────┘
        2
```

Rules for the file:

- **One row per node.** Row order sets the node id: the first row is node 0, the second is node 1, and so on. The number at the start of each row is only a label for readers; the program ignores it.
- **Links are `neighbor weight` pairs,** and each row must end with a `.` token.
- **List every link on both nodes' rows, with the same weight.** A link listed on only one side gives the two nodes different views of the network.
- **Weights are costs.** Each node computes shortest paths, so messages travel the route with the lowest total weight. In the shipped topology, node 0 reaches node 3 through 1 and 4 (cost 1 + 2 + 3 = 6), not through 2 (cost 10 + 2 = 12).
- **Comments and blank lines:** `#` starts a comment, and blank lines are ignored.
- **Up to 5 nodes, ids 0–4.** The node count is fixed at 5 in `src/externals.cc` (`total_nodes`). A topology with fewer nodes works fine.

The topology is the only thing that defines links. Every node can technically reach every other over UDP, but routing only ever forwards a packet to a neighbor listed in this file.

## 3. Start the nodes

Open one terminal per node. In each one, `cd` into `src/` (every node reads `topology.txt` from its current directory) and start the node with its id:

```sh
./client2 0        # terminal 1
./client2 1        # terminal 2
./client2 2        # terminal 3
./client2 3        # terminal 4
./client2 4        # terminal 5
```

Node N listens on UDP port `50000 + N` on localhost. After printing its view of the topology and its routing setup, each node prints:

```
Node 0 ready. Send with: <dest_node_id> <message>
```

Start every node in the topology, including ones that will never send. A node that isn't running can't forward packets, so any route through it fails.

## 4. Send messages

Type one message per line in any node's terminal, starting with the destination node's id:

```
3 hello from node zero
```

The message travels hop by hop along the shortest route, and arrives in node 3's terminal as:

```
CLIENT: [data received from node 0]: hello from node zero
```

Any node can send to any other node at any time, and several nodes can send at once. Messages may contain spaces. Blank lines are ignored, and a line that doesn't start with a valid node id gets a short reminder of the format.

On every hop, the receiving node may drop the frame or flip one of its bits (see [Link degradation](#link-degradation)). The data link layer recovers: the CRC check rejects corrupted frames, and the sender retransmits until the neighbor acknowledges. A message only fails if one hop fails 5 times in a row.

## 5. Stop the nodes

- **Ctrl-D** ends a node's input. The node stays up, still forwarding and receiving, because other nodes' routes may pass through it.
- **Ctrl-C** quits the node.

## Reading the output

With `-DDEBUG_VERSION`, each node prints a running log of all three layers. These lines are the most useful (search for them exactly as printed, misspellings included):

| Line | Layer | Meaning |
|---|---|---|
| `CLIENT: [data received from node N]: …` | application | A message addressed to this node arrived |
| `Move to Application layer` | network | A packet for this node is being handed up |
| `Move the packet to the send side` | network | This node is forwarding a packet for someone else |
| `No route to node N, dropping packet.` | network | N isn't in the topology, or can't be reached through it |
| `Ha Ha Ha! Dropped a packet.` | physical | The simulated link lost an incoming frame |
| `Introduced a 1-bit error.` | physical | The simulated link corrupted an incoming frame |
| `Cyclic Redundency Check error: dropping packet.` | data link | The CRC caught a corrupted frame |
| `Timeout occured N time.` | data link | No acknowledgement in time; resending |
| `Acknowlegement accepted.` | data link | The neighbor confirmed the frame |
| `Destination node unreachable.` | data link | Gave up after 5 timeouts on one hop |

Each layer runs in its own threads, and they all write to the same terminal without coordinating. Two lines sometimes get mixed together, and frames are printed as raw bytes, so you will see some garbled characters. When searching saved output, use `grep -a` so the raw bytes don't make `grep` treat the file as binary.

### Quieter output

Build without `-DDEBUG_VERSION` to hide the per-frame link and data link messages:

```sh
g++ -I. -D_REENTRANT -pthread -o client2 \
    client2.cc log.cc externals.cc udp.cc error.cc link.cc datalink.cc \
    hdlcframe.cc crc32.cc network.cc fileparser.cc
```

The startup topology dump and the network layer's per-packet lines remain. The drop, bit-error, CRC and retransmission messages disappear, so you can no longer watch the link misbehave.

## Link degradation

Faults are injected as frames arrive at each node. They are currently set in code, in `main()` of `src/client2.cc`:

```c++
    ext::packet_err_wt = 15;
    ext::packet_drop_wt = 15;
```

`packet_err_wt` controls bit errors and `packet_drop_wt` controls dropped frames. A value of N means roughly 1 frame in N; 0 turns that fault off. Values above 127 don't fit (the variables are `char`). The same settings apply to every link of that node. Rebuild after changing them.

Retransmission is tuned in `src/externals.cc`: `to_val` is how long to wait for an acknowledgement (1 second), and `max_timeouts` is how many tries a hop gets before giving up (5).

## Scripted runs

Each node reads its messages from stdin, so you can drive a whole network from a script. This example starts nodes 1–4 with no input, sends two messages from node 0, then collects what was delivered:

```sh
cd src
for n in 1 2 3 4; do
  timeout 30 ./client2 $n < /dev/null > node$n.log 2>&1 &
done
(sleep 2; printf '3 hello from zero\n2 second message\n') | timeout 30 ./client2 0 > node0.log 2>&1
wait
grep -a 'data received' node*.log
```

The `sleep 2` gives the other nodes time to come up. Nodes never exit on their own, so `timeout` is what stops them.

## The single-link client

`client` is an older, simpler program that brings up only the physical and data link layers: there is no routing, so frames go straight to the destination. Both build systems build it, along with `server`, which is an empty placeholder:

```sh
./autogen.sh && ./configure && make     # builds src/client
make -f Makefile.x                      # or: builds bin/client
```

Run it as `client <my_node_id> <dest_node_id> 5` (the last argument is the node count). Node 0 reads lines from stdin and sends each one to its destination; every other node just prints what it receives:

```sh
./client 1 0 5                 # receiver
./client 0 1 5                 # sender: type lines here
```

## Troubleshooting

| What you see | Cause and fix |
|---|---|
| `File open operation failed: topology.txt` | The node was started outside the directory containing `topology.txt`. Run it from `src/`. |
| `terminate called after throwing an instance of 'Errno_exception'` at startup | Another process already uses this node's port, usually a node with the same id still running. Stop it, for example with `pkill client2`, and start again. |
| The usage text is printed and the node exits | The node id is missing, outside 0–4, or followed by extra arguments. |
| `Destination node unreachable.` | The next node on the route isn't running. Start every node in the topology. |
| `No route to node N, dropping packet.` | N isn't in `topology.txt`, or no chain of links connects to it. |
| Output scrolls too fast to follow | Use the [quieter build](#quieter-output), or save output to a file and search it with `grep -a`. |
