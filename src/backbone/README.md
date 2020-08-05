Backbone: link-aggregate and network backbone application demo
==============================================================

The backbone application manages a router node in a "centipede"
network topology.  The setup can look like this:

```
        .---------.     .---------.       .---------.
     .->w0       e0<--->w0       e0<-...->w0       e0
 (H) |  |  node1  |     |  node2  |       |  nodeN  |
     '->w1       e1<--->w1       e1<-...->w1       e1
        '---eth0--'     '---eth0--'       '---eth0--'
             ^               ^                 ^
             |               |                 |
       n1h5<-+         n2h6<-+           nNh7<-+
             |               |                 |
             +->n1h6         +->n2h7           +->nNh8
             |               |                 |
             v               v                 v
```

Each _node_ on the backbone maintains a two-port link aggregate to
both of its neighbors, one _westward_ facing (w0,w1) and one
_eastward_ facing (e0,e1).

All nodes on the backbone exchange _hello_ messages with their
neighbors.  The information in these messages is used to determine
each node's position in the daisy-chain relative to the chain's
_head_.

A node determines it is the head node when it receives hello messages
from itself, i.e. when either w0/w1 or e0/e1 are directly connected
(H). Other nodes will then _attach_ to the head node, increasing the
ID by one for each hop.

Once a node has figured out its ID, it will use that to select the
backbone IP address `192.168.255.{ID}/24`.

A node always uses the address `192.168.0.1/24` on the LAN side
(`eth0`), when attached it will also setup the globally addressable
address `192.168.{ID}.1/24`. Hosts on the LANs can thus communicate
with other hosts on the same LAN using the fixed subnet
`192.168.0.0/24`; they can also address hosts on other LANs using the
globally addressable subnets `192.168.{ID}.0/24`. __NOTE__: If a node
in the middle of the backbone becomes unavailable, all nodes behind it
will change their IDs to reflect the new topology, as a result the
global addresses of hosts connected to the LAN of those nodes will
also change.

In the example above, assuming that all nodes have peered, the
addresses would be as follows:

| Host  |        Backbone |        Global |         Local |
|:------|----------------:|--------------:|--------------:|
| node1 | `192.168.255.1` | `192.168.1.1` | `192.168.0.1` |
| n1h5  |             N/A | `192.168.1.5` | `192.168.0.5` |
| n1h6  |             N/A | `192.168.1.6` | `192.168.0.6` |
| node2 | `192.168.255.2` | `192.168.2.1` | `192.168.0.1` |
| n2h6  |             N/A | `192.168.2.6` | `192.168.0.6` |
| n2h7  |             N/A | `192.168.2.7` | `192.168.0.7` |
| nodeN | `192.168.255.N` | `192.168.N.1` | `192.168.0.1` |
| nNh7  |             N/A | `192.168.N.7` | `192.168.0.7` |
| nNh8  |             N/A | `192.168.N.8` | `192.168.0.8` |

Packets coming from the LAN have their source addresses translated
(SNAT) to their global equivalents before being routed out on the
backbone. Conversely, packets destined for a host on the node's LAN
have their destination addresses translated to the local subnet (DNAT)
before they're send out on `eth0`.


Configuration
-------------

A simple JSON file is used by backbone to determine which port should
be part of which link aggregate and which VID that should be used to
carry the backbone traffic. It is read from `/etc/backbone.json` and
looks something like this:

```json
{
    "vid": 2,

    "west": [ "eth1", "eth2" ],
    "east": [ "eth3", "eth4" ]
}
```


Protocol
--------

Before any L3 forwarding can take place between the LANs, each node on
the backbone must figure out their ID. This is done by continuously
sending _hello_ messages through each backbone port (w0, w1, e0, e1),
and listening to hello messages coming in on those ports.

```
Ethernet:
  .-------.-------.---------.
  | DA(6) | SA(6) | Type(2) |
  '-------'-------'---------'
  DA:      01:80:c2:00:00:02, "Slow protocols".
  SA:      MAC of sending interface.
  Type:    0x8809, "Slow protocols".

Slow Protocol:
  .------------.--------.-------------.
  | Subtype(1) | OUI(3) | Protocol(1) |
  '------------'--------'-------------'
  Subtype:  10, "Organization Specific Slow Protocol" (OSSP).
  OUI:      00:07:7c, "Westermo".
  Protocol: 1, "Backbone".

Backbone:
  .------------.---------.-------.
  | Station(6) | Head(6) | ID(1) |
  '------------'---------'-------'
  Station: MAC of sender.
  Head:    MAC of head that sender is attached to, or zero if detached.
  ID:      ID on backbone, i.e. distance from head when attached.

```

Since the protocol replaces the function typically supplied by LACP,
i.e. negotiating with the link neighbor to determine if they are
compatible in a link aggregate sense, the backbone protocol is layered
on top of a standard IEEE slow protocol header. To differentiate it
from standard protocols, subtype 10, Organization Specific Slow
Protocol, is used.

Using the hello messages received from its neighbors, each node find
the best head node to attach to, taking care to discover if it itself
is the head node.

When attached, for each backbone port, the node's ID is compared to
the ID reported by the neighbor on that port. If the node agrees with
its neighbor about which node is the head and their absolute distance
is one (1), they must be next to each other on the backbone and the
port is enabled (_distributing_ in LACP terms). Otherwise the port is
disabled (_detached_ in LACP terms).


Linux Network Setup
-------------------

```
                        <192.168.255.{ID}>
                            .--------.
                            |backbone|             Bridge
                            '--+--+--'
                         .-----'  '-----.
                      .--+-.          .-+--.
                      |west|          |east|       Link Aggregates
<192.168.{ID}.1>      '+--+'          '+--+'
 <192.168.0.1>       .-'  '-.        .-'  '-.
    .----.        .--+-.  .-+--.  .--+-.  .-+--.
    |eth0|        |eth1|  |eth2|  |eth3|  |eth4|   Ports
    '----'        '----'  '----'  '----'  '----'
```

Following the configuration file described earlier, the four backbone
ports are paired up into two link aggregates using the _Team_ facility
in the kernel. Internally backbone uses `libteam` to control these
interfaces.

In order for traffic to be forwarded to routers further from, or
nearer to, the head than this node, it must be bridged at the Ethernet
level. This application uses the standard Linux `bridge` module for
this purpose. If the ports are part of a hardware switch, the
forwarding will be transparently offloaded.

Packets destined for the current node will ingress on the backbone
interface, where it will be received by the kernel's IP stack. Using
the routes setup by the backbone daemon, packets are routed to the
internal LAN and routed out through `eth0`.

In order for local hosts to transparently use their local source
addresses when communicating with remote hosts, backbone uses Linux's
_Traffic Control_ (TC) subsystem to re-write the source/destination
addresses on egress/ingress to the backbone bridge.
