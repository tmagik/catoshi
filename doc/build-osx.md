Mac OS X codecoind build instructions
====================================
This guide will show you how to build codecoind(headless client) for OSX.

It's broken. Please send patches.

Notes
-----

* All of the commands should be executed in a Terminal application. The
built-in one is located in `/Applications/Utilities`.

Preparation
-----------

You need to install XCode with all the options checked so that the compiler
and everything is available in /usr not just /Developer. XCode should be
available on your OS X installation media, but if not, you can get the
current version from https://developer.apple.com/xcode/. If you install
Xcode 4.3 or later, you'll need to install its command line tools. This can
be done in `Xcode > Preferences > Downloads > Components` and generally must
be re-done or updated every time Xcode is updated.

There's also an assumption that you already have `git` installed. If
not, it's the path of least resistance to install [Github for Mac](https://mac.github.com/)
(OS X 10.7+) or
[Git for OS X](https://code.google.com/p/git-osx-installer/). It is also
available via Homebrew.

You will also need to install [Homebrew](http://brew.sh) in order to install library
dependencies.

The installation of the actual dependencies is covered in the Instructions
sections below.

Instructions: Homebrew
----------------------

#### Install dependencies using Homebrew

        brew install boost miniupnpc openssl berkeley-db5.3

(berkely DB 5.3 recipie available at https://github.com/tmagik/homebrew-core)
(yes I mixed Git and Mercurial, that's like crossing the streams or something)

### Building `codecoind`

1. Clone the github tree to get the source code and go into the directory.

        hg clone https://bitbucket.org/catoshi/catoshi
        cd bitcoin

2.  Modify source in order to pick up the `openssl` library.

    Edit `makefile.osx` to account for library location differences. There's a
    diff in `contrib/homebrew/makefile.osx.patch` that shows what you need to
    change, or you can just patch by doing

        patch -p1 < contrib/homebrew/makefile.osx.patch

3.  Build bitcoind:

        cd src
        make -f makefile.osx

4.  It is a good idea to build and run the unit tests, too:

        make -f makefile.osx test

Creating a release build
------------------------

TODO: document this


Running
-------

It's now available at `./*coind`, provided that you are still in the `src`
directory. We have to first create the RPC configuration file, though.

Run `./*coind` to get the filename where it should be put, or just try these
commands:

    echo -e "rpcuser=*coinrpc\nrpcpassword=$(xxd -l 16 -p /dev/urandom)" > "/Users/${USER}/Library/Application Support/*Coin/*coin.conf"
    chmod 600 "/Users/${USER}/Library/Application Support/*Coin/*coin.conf"

When next you run it, it will start downloading the blockchain, but it won't
output anything while it's doing this. This process may take several hours.

Other commands:

    ./*coind --help  # for a list of command-line options.
    ./*coind -daemon # to start the *coin daemon.
    ./*coind help    # When the daemon is running, to get a list of RPC commands
