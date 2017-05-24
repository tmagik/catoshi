### Seeds ###

Utility to generate the seeds.txt list that is compiled into the client
(see [src/chainparamsseeds.h](/src/chainparamsseeds.h) and other utilities in [contrib/seeds](/contrib/seeds)).

The seeds compiled into the release are created from sipa's DNS seed data, like this:

    curl -s http://bitcoin.sipa.be/seeds.txt > seeds_main.txt
    python makeseeds.py < seeds_main.txt > nodes_main.txt
The input to makeseeds.py is assumed to be approximately sorted from most-reliable to least-reliable,
with IP:port first on each line (lines that don't match IPv4:port are ignored).

