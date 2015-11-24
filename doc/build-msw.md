WINDOWS BUILD NOTES
===================

At this point, the easiest way to build windows is to install a debian 8.0
VM in virtualbox, and then run:

hg clone https://bitbucket.org/tmagik/catoshi
git clone https://github.com/mxe/mxe.git
cd mxe
make db qtbase boost openssl
cd ../
mkdir build-windows
cd build-windows
(might have to add mxe tools to path)
../mxe/usr/bin/i686-w64-mingw32.static-qmake-qt5 ../catoshi/codecoin.pro
make



### WARNING: ###
Using MXE may result in using versions of openssl or db that have various
issues. See
https://lists.linuxfoundation.org/pipermail/bitcoin-dev/2015-July/009697.html

TODO: clone mxe and modify openssl/db/etc to our liking, and add a catoshi
garget
