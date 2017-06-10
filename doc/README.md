*Coin Core 0.14.1
=====================

Copyright (c) 2009-2013 Bitcoin Developers
Copyright (c) 2011-2013 Litecoin Developers
Copyright (c) 2013-2014 Catcoin Developers
Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3

Distributed under the Affero GNU General public license version 3
file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](http://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.


Intro
---------------------
*Coin is a free open source peer-to-peer electronic cash system that is
completely decentralized, without the need for a central server or trusted
parties.  Users hold the crypto keys to their own money and transact directly
with each other, with the help of a P2P network to check for double-spending.

### Unix

You need the Qt4 run-time libraries to run Bitcoin-Qt. On Debian or Ubuntu:

	sudo apt-get install libqtgui4

Unpack the files into a directory and run:

- bin/32/bitcoin-qt (GUI, 32-bit) or bin/32/bitcoind (headless, 32-bit)
- bin/64/bitcoin-qt (GUI, 64-bit) or bin/64/bitcoind (headless, 64-bit)

See the documentation at the [*Coin Wiki](http://bitbucket.org/catoshi/catoshi)
for help and more information.
* Ask for help on [#catoshi](http://webchat.freenode.net?channels=catoshi) on Freenode. If you don't have an IRC client use [webchat here](http://webchat.freenode.net?channels=catoshi).

Development
---------------------
- [Unix Build Notes](build-unix.md)
- [OSX Build Notes](build-osx.md)
- [Coding Guidelines](coding.md)
- [Multiwallet Qt Development](multiwallet-qt.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://localhost/codecoin/doxygen/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [Unit Tests](unit-tests.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
