# Intent

Wire is a teaching simulation of a layered network stack. Each layer of the stack is its own piece of code with its own job, and you can watch them cooperate to move messages between nodes over links that are deliberately unreliable.

The goal is to make networking concepts visible and testable, not to ship a usable protocol. Wire is not a production stack, and comparing it with TCP/IP or any other real stack is not meaningful (see README.md).

## How it is meant to be used

1. **Describe the network in a text file.** `topology.txt` lists every node and the links between them, with a weight (cost) on each link. That file is the whole network: it decides which nodes exist and how they are connected.
2. **Bring up one instance per node.** Start the Wire binary once for each node in the topology, each in its own terminal, telling every instance which node it is. Each running process *is* that node.
3. **Pass messages between nodes.** Type a message into one node's terminal, address it to another node, and watch it arrive. On the way, each layer does its job: the message is routed across the topology, framed and checked on every hop, and retransmitted when the simulated link damages or loses it.
4. **Degrade the links and observe.** Make the links drop packets or corrupt bits in a configurable way, then watch how the layers above detect errors and recover from them.

## Layer model

Wire follows the OSI model more closely than TCP/IP. Each layer is a separate component that talks only to the layers directly above and below it, through its own send and receive queues.

| OSI layer | What Wire does there | Source |
|---|---|---|
| Network (3) | Link-state routing: link-state packets (LSPs) flooded to every node, shortest paths computed with Dijkstra, and messages forwarded hop by hop toward their destination | `src/network.cc` |
| Data link (2) | HDLC-style framing with flag bytes and bit stuffing, a CRC-32 frame check sequence, and acknowledged retransmission (stop-and-wait ARQ) between neighboring nodes | `src/datalink.cc`, `src/hdlcframe.cc`, `src/crc32.cc` |
| Physical (1) | Moves raw frames between node processes, and is where link degradation is injected | `src/link.cc`, `src/udp.cc` |

### UDP as the physical layer

The bottom of the stack is a UDP datagram socket, and Wire treats it as the **physical medium**, not as a transport protocol. Each node listens on its own local UDP port, and sending a frame to a neighbor means sending a datagram to that neighbor's port. Everything above this "wire" (framing, error detection, retransmission, routing) is Wire's own code.

On top of that medium, the physical layer simulates an imperfect channel. Before a frame reaches the data link layer, it may be:

- **dropped**, so the frame never arrives, or
- **corrupted**, so a bit in the frame is flipped.

How often each happens is configurable. With a degraded link you can watch the CRC check catch a corrupted frame and the ARQ protocol retransmit it until it gets through, or until the node gives up and reports the destination as unreachable.

## OSI first, with TCP/IP ideas borrowed

Wire's structure and terminology come from OSI:
- Reliability is handled hop by hop at the data link layer, as HDLC (an ISO standard) does.
- The routing protocol's "link-state packet" is IS-IS terminology.

Where a TCP/IP idea is the clearer teaching example, Wire uses it:
- UDP sockets are the transmission medium.
- Packets carry an age/hop-limit field that plays the role of the IP TTL.
- Shortest-path routing with Dijkstra works much like OSPF.

## Where the code stands relative to this intent

The project is incomplete (see README.md). Notable gaps between the intent above and the current code:

- **Degradation is global, not per link.** The drop and error rates (`ext::packet_drop_wt`, `ext::packet_err_wt`) apply to everything a process receives, and they are set in each program's `main`. They can't yet be set per link or in the topology file.
- **Only `src/client2.cc` runs the full stack** (physical, data link and network layers). `src/client.cc` stops at the data link layer, and `src/server.cc` is an empty stub.
- **No layers above the network layer yet.** The application reads messages from stdin and hands them straight to the network layer; the transport, session and presentation layers are not modeled as separate layers.
