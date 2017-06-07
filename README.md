Codecoin Catbox
================================

http://7el.us/CATOSHI

Copyright (c) 2009-2017 Bitcoin Developers
Copyright (c) 2011-2017 Litecoin Developers
Copyright (c) 2013-2014 Catcoin Developers
Copyright (c) 2015-2017 Grantcoin Developers
Copyright (c) 2015-2017 Troy Benjegerdes <hozer@hozed.org>

### What is Codecoin

Codecoin is a project to build multiple C++ 'Catoshi' clients from the
same source tree. Default build is Grantcoin. On linux, do this:

    cd src
    make                         # builds grantcoin
    make COIN_BRAND=kittycoin    # builds kittycoin
    make regression # builds & runs regression tests on multiple coins

### What is Grantcoin?
[Grantcoin](http://www.grantcoin.org/) (abbreviated GRT) is the first [cryptocurrency](https://en.wikipedia.org/wiki/Cryptocurrency) distributed primarily as Proof-of-Participation grants according to a transparent business plan by a legally incorporated nonprofit organization. Grantcoin is based on [Peercoin](http://peercoin.net/) (PPCoin), the first digital currency using Proof-of-Stake consensus as a security model, which is more energy efficient than [Bitcoin](http://en.wikipedia.org/wiki/Bitcoin)'s Proof-of-Work mining system. Grantcoin will begin with several months of PoW until the currency is widely circulated, with further evolution determined by the foundation and community.

### What is kittycoin?

Kittycoin used to be a Catcoin-compatible cryptographic currency with
scrypt N=1024 proof of work, with (mostly) the same blockchain parameters
as Bitcoin:
 - 50 coins per block
 - 10 minutes block targets
 - Subsidy halves in 210k blocks
 - 21 million total coins
 - 1 block difficulty adjustment, with a PID ajustment.

It is currently not particularly usefull except as a catbox for n00b
developers. I mean, it's a developer sandbox that's a safe space to try
things that would get you flamed on bitcointalk.

Developers work in their own trees, then submit pull requests when they think
their feature or bug fix is ready.

If it is a simple/trivial/non-controversial change, then one of the Bitcoin
development team members simply pulls it.

If it is a *more complicated or potentially controversial* change, then the patch
submitter will be asked to start a discussion (if they haven't already) on the
[mailing list](http://sourceforge.net/mailarchive/forum.php?forum_name=bitcoin-development).

The patch will be accepted if there is broad consensus that it is a good thing.
Developers should expect to rework and resubmit patches if the code doesn't
match the project's coding conventions (see [doc/coding.md](doc/coding.md)) or are
controversial.

Seymour Cray was a friend of my dad's. I asked him what it was like to know
the genius who had built the world's first supercomputer company. My dad said,
‘Well, actually, son, he wasn't so much smarter than me. He just made mistakes
a hundred times faster.’

~~ Townes Duncan

What do we do with Catcoin better than any other coin? 
	make
			mistakes
					faster
And then land on our feet.

### License

Kittycoin is released under the terms of the Affero GNU General Public License
version 3 (AGPLv3). See COPYRING for more information, or the GNU website at
http://www.gnu.org/licenses/agpl-3.0.html

If you want to give me crap about the license, flame away, or pay me enough 
to buy a farm and I'll license it however the hell you want. Otherwise some
get-rich-quick copycrap startup will accidentally include my AGPLv3 code, and
then I will 0wn them. The revolution will not be televised, it will be quietly
settled out-of-court.

### Development process

The development process is Troy commits stuff when he feels like it. He's not
a computer execution unit following a process, he's a guy who happens to like
cats, and think cryptographic currencies are a better way to make money.

He also likes rough consensus, running code, and good tests. So if you have all
of those, the probability that he'll feel like integrating your change approaches
unity over a year or three.

If you have only rough consensus, or only running code, the others can also be
achieved with sufficient money for a publicly announced contract and request
for proposals. However be warned that it's dangerous to think you can buy a
person who writes code that makes money.

[a href=<the original bitcoin/litecoin/catcoin dev process boilerplate>]

Testing
-------

Testing is good. Do it. It makes aforementioned consensus easier.

### Building
on debian 8.x:

    sudo apt-get install mercurial build-essential libboost-dev \
	libdb++-dev libssl-dev libminiupnpc-dev libboost-filesystem-dev \
	libboost-system-dev libboost-program-options-dev \
	libboost-thread-dev libboost-test-dev libleveldb-dev libevent-dev
    cd src; make 

### Automated Testing

Developers are strongly encouraged to write unit tests for new code, and to
submit new unit tests for old code.

Unit tests for the core code are in `src/test/`. To compile and run them:

    cd src; make test

Unit tests for the GUI code are in `src/qt/test/`. To compile and run them:

    qmake BITCOIN_QT_TEST=1 -o Makefile.test bitcoin-qt.pro
    make -f Makefile.test
    ./catcoin-qt_test

Run with the -testnet option to run with "play bitcoins" on the test network, if you
are testing multi-machine code that needs to operate across the internet.

If you are testing something that can run on one machine, run with the -regtest option.
In regression test mode, blocks can be created on-demand; see qa/rpc-tests/ for tests
that run in -regtest mode.


