TEMPLATE = app
VERSION = 0.15.42.1
INCLUDEPATH += src
QT += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE
CONFIG += no_include_pwd
CONFIG += thread
CONFIG += c++11
CONFIG += object_parallel_to_source

#may need below. cause arg
#
android:DEFINES += BOOST_NO_SCOPED_ENUMS BOOST_NO_CXX11_SCOPED_ENUMS __WORDSIZE=32

isEmpty(COIN_BRAND){
        COIN_BRAND=litecoin
}
# use: qmake "COIN_BRAND=grantcoin" for other than litecoin
contains(COIN_BRAND, litecoin) {
    message(Building for Litecoin)
    DEFINES += BRAND_litecoin
    HASHSCRYPT = 1
    TARGET = litecoin
} else {
contains(COIN_BRAND, grantcoin) {
    message(Building for Grantcoin)
    DEFINES += BRAND_grantcoin
    TARGET = grantcoin
} else {
contains(COIN_BRAND, grantstake) {
    message(Building for Grantstake)
    DEFINES += BRAND_grantstake
    STAKE = 1
    TARGET = grantstake
} else {
contains(COIN_BRAND, catcoin) {
    message(Building for Catcoin)
    DEFINES += BRAND_catcoin
    HASHSCRYPT = 1
    TARGET = catcoin
} else {
contains(COIN_BRAND, givecoin) {
    message(Building for Givecoin)
    DEFINES += BRAND_givecoin
    TARGET = givecoin
    HASHSCRYPT = 1
} else {
contains(COIN_BRAND, hamburger) {
    DEFINES += BRAND_hamburger
    TARGET = hamburger
    HASHSCRYPT = 1
    STAKE = 1
} else {
contains(COIN_BRAND, givestake) { # for testing, for now
    DEFINES += BRAND_givecoin 
    TARGET = givestake
    STAKE = 1
    HASHSCRYPT = 1
} else {
contains(COIN_BRAND, bluecoin) {
    DEFINES += BRAND_bluecoin
    TARGET = bluecoin
    STAKE = 1
    HASHSCRYPT = 1
    HASHX11 = 1
} else {
    DEFINES += BRAND_codecoin
    TARGET = codecoin
	warning ("Building GENERIC codecoin, probably will not work")
}}}}}}}}

#Set up a symlink so QT designer doesn't get confused when editing UI files
#will error on windows build without cygwin.
#This is rather a hack. FIXME
#system("ln -sfT $$TARGET/codecoin.qrc src/qt/res/codecoin.qrc")
#same thing for icons
#system("ln -sfT $$TARGET/icons src/qt/res/icons")

# for boost 1.37, add -mt to the boost libraries
# use: qmake BOOST_LIB_SUFFIX=-mt
# for boost thread win32 with _win32 sufix
# use: BOOST_THREAD_LIB_SUFFIX=_win32-...
# or when linking against a specific BerkelyDB version: BDB_LIB_SUFFIX=-4.8

# Dependency library locations can be customized with:
#    BOOST_INCLUDE_PATH, BOOST_LIB_PATH, BDB_INCLUDE_PATH,
#    BDB_LIB_PATH, OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

# Windows compilation, refer to the link below for detailed instructions
# https://bitcointalk.org/index.php?topic=149479.0
win32 {
    BOOST_LIB_SUFFIX=-mt
    BOOST_THREAD_LIB_SUFFIX=_win32-mt
    BOOST_INCLUDE_PATH=C:/deps/boost1.55-1.55.0+dfsg.orig
    BOOST_LIB_PATH=C:/deps/boost1.55-1.55.0+dfsg.orig/stage/lib
    BDB_INCLUDE_PATH=C:/deps/db5.3-5.3.28/build_unix
    BDB_LIB_PATH=C:/deps/db5.3-5.3.28/build_unix
    OPENSSL_INCLUDE_PATH=
    OPENSSL_LIB_PATH=
    MINIUPNPC_INCLUDE_PATH=C:/deps/
    MINIUPNPC_LIB_PATH=C:/deps/miniupnpc
}

OBJECTS_DIR = build
MOC_DIR = build/moc
UI_DIR = build/ui

macx {
# Brew for boost, bdb53, openssl. /usr/local for secp256k1 & univalue
    CONFIG += sdk
    QMAKE_MAC_SDK = macosx10.13
    BREW = /usr/local/opt
    BOOST = boost\@1.66
    BDB = berkeley-db\@53
# openssl 1.1 has EVP_MD_CTX_new
    SSL = openssl\@1.1
    DEFINES += HAVE_DECL_EVP_MD_CTX_NEW
    PROTOBUF = protobuf\@3.5
    PROTOBUF_LIB_PATH = $${BREW}/$${PROTOBUF}/lib
    OS_INCLUDE_PATH += /usr/local/include
    OS_LIB_PATH += /usr/local/lib
}

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    # Mac: compile for maximum compatibility (10.5, 32-bit)
    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk
    macx:QMAKE_CFLAGS += -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk
    macx:QMAKE_OBJECTIVE_CFLAGS += -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk
	macx:QMAKE_LFLAGS += -mmacosx-version-min=10.5 -arch i386 -isysroot /Developer/SDKs/MacOSX10.5.sdk

    !win32:!macx {
        # Linux: static link and extra security (see: https://wiki.debian.org/Hardening)
        LIBS += -Wl,-Bstatic -Wl,-z,relro -Wl,-z,now
    }
}

!win32 {
    # for extra security against potential buffer overflows: enable GCCs Stack Smashing Protection
    QMAKE_CXXFLAGS *= -fstack-protector-all
    QMAKE_LFLAGS *= -fstack-protector-all
    # Exclude on Windows cross compile with MinGW 4.2.x, as it will result in a non-working executable!
    # This can be enabled for Windows, when we switch to MinGW >= 4.4.x.
}
# for extra security (see: https://wiki.debian.org/Hardening): this flag is GCC compiler-specific
QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2
# for extra security on Windows: enable ASLR and DEP via GCC linker flags
win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat
# on Windows: enable GCC large address aware linker flag
win32:QMAKE_LFLAGS *= -Wl,--large-address-aware
# i686-w64-mingw32
win32:QMAKE_LFLAGS *= -static-libgcc -static-libstdc++

# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    LIBS += -lqrencode
}

# use: qmake "USE_UPNP=1" ( enabled by default; default)
#  or: qmake "USE_UPNP=0" (disabled by default)
#  or: qmake "USE_UPNP=-" (not supported)
# miniupnpc (http://miniupnp.free.fr/files/) must be installed for support
macx {
message(FIXME WARNING upnp support disabled for now!!!)
USE_UPNP=-
}

android || ios: USE_UPNP=-

contains(USE_UPNP, -) {
    message(Building without UPNP support)
} else {
    message(Building with UPNP support)
    count(USE_UPNP, 0) {
        USE_UPNP=1
    }
    DEFINES += USE_UPNP=$$USE_UPNP 
    INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
    LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
    win32:LIBS += -liphlpapi
    win32:DEFINES += MINIUPNP_STATICLIB
}

# use: qmake "USE_DBUS=1"
contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

# use: qmake "USE_IPV6=1" ( enabled by default; default)
#  or: qmake "USE_IPV6=0" (disabled by default)
#  or: qmake "USE_IPV6=-" (not supported)
contains(USE_IPV6, -) {
    message(Building without IPv6 support)
} else {
    count(USE_IPV6, 0) {
        USE_IPV6=1
    }
    DEFINES += USE_IPV6=$$USE_IPV6
}

contains(BITCOIN_NEED_QT_PLUGINS, 1) {
    DEFINES += BITCOIN_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}

# regenerate src/build.h
!win32|contains(USE_BUILD_INFO, 1) {
    genbuild.depends = FORCE
#TODO: make this use the object build dir!
    genbuild.commands = cd $$PWD; /bin/sh share/genbuild.sh src/build.h
    genbuild.target = src/build.h
    PRE_TARGETDEPS += src/build.h
    QMAKE_EXTRA_TARGETS += genbuild
    DEFINES += HAVE_BUILD_INFO
}

QMAKE_CXXFLAGS_WARN_ON = -fdiagnostics-show-option -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter -Wstack-protector

# Input
DEPENDPATH += src src/json src/qt

CODECOIN_QT_H = \
    src/qt/addressbookpage.h \
    src/qt/addresstablemodel.h \
    src/qt/askpassphrasedialog.h \
    src/qt/bantablemodel.h \
    src/qt/bitcoinaddressvalidator.h \
    src/qt/bitcoinamountfield.h \
    src/qt/bitcoinunits.h \
    src/qt/callback.h \
    src/qt/clientmodel.h \
    src/qt/codecoingui.h \
    src/qt/coincontroldialog.h \
    src/qt/coincontroltreewidget.h \
    src/qt/csvmodelwriter.h \
    src/qt/editaddressdialog.h \
    src/qt/guiconstants.h \
    src/qt/guiutil.h \
    src/qt/intro.h \
    src/qt/macdockiconhandler.h \
    src/qt/macnotificationhandler.h \
    src/qt/modaloverlay.h \
    src/qt/networkstyle.h \
    src/qt/notificator.h \
    src/qt/openuridialog.h \
    src/qt/optionsdialog.h \
    src/qt/optionsmodel.h \
    src/qt/overviewpage.h \
    src/qt/paymentrequestplus.h \
    src/qt/paymentserver.h \
    src/qt/peertablemodel.h \
    src/qt/platformstyle.h \
    src/qt/qvalidatedlineedit.h \
    src/qt/qvaluecombobox.h \
    src/qt/receivecoinsdialog.h \
    src/qt/receiverequestdialog.h \
    src/qt/recentrequeststablemodel.h \
    src/qt/rpcconsole.h \
    src/qt/sendcoinsdialog.h \
    src/qt/sendcoinsentry.h \
    src/qt/signverifymessagedialog.h \
    src/qt/splashscreen.h \
    src/qt/trafficgraphwidget.h \
    src/qt/transactiondesc.h \
    src/qt/transactiondescdialog.h \
    src/qt/transactionfilterproxy.h \
    src/qt/transactionrecord.h \
    src/qt/transactiontablemodel.h \
    src/qt/transactionview.h \
    src/qt/utilitydialog.h \
    src/qt/walletframe.h \
    src/qt/walletmodel.h \
    src/qt/walletmodeltransaction.h \
    src/qt/walletview.h \
    src/qt/winshutdownmonitor.h

HEADERS += \
    $$CODECOIN_QT_H \
    src/addrman.h \
    src/base58.h \
    src/checkpoints.h \
    src/codecoin.h \
    src/compat.h \
    src/sync.h \
    src/util.h \
    src/hash.h \
    src/uintBIG.h \
    src/kernel.h \
    src/pbkdf2.h \
    src/serialize.h \
    src/validation.h \
    src/undo.h \
    src/net.h \
    src/key.h \
    src/dbwrapper.h \
    src/wallet/walletdb.h \
    src/script/script.h \
    src/init.h \
    src/bloom.h \
    src/checkqueue.h \
    src/rpc/blockchain.h \
    src/rpc/client.h \
    src/rpc/mining.h \
    src/rpc/protocol.h \
    src/rpc/register.h \
    src/rpc/safemode.h \
    src/rpc/server.h \
    src/rpc/util.h \
    src/wallet/coincontrol.h \
    src/wallet/crypter.h \
    src/wallet/db.h \
    src/wallet/feebumper.h \
    src/wallet/fees.h \
    src/wallet/init.h \
    src/wallet/rpcwallet.h \
    src/wallet/walletdb.h \
    src/wallet/walletutil.h \
    src/keystore.h \
    src/wallet/crypter.h \
    src/protocol.h \
    src/ui_interface.h \
    src/version.h \
    src/netbase.h \
    src/clientversion.h \
    src/txdb.h \
    src/threadsafety.h \
    src/limitedmap.h \
    src/$${TARGET}/consensus.h \
    src/$${TARGET}/$${TARGET}.h \
    src/$${TARGET}/seeds.h \
    src/build.h

UTIL_SRC = \
    src/support/lockedpool.cpp \
    src/chainbaseparams.cpp \
    src/clientversion.cpp \
    src/compat/glibc_sanity.cpp \
    src/compat/glibcxx_sanity.cpp \
    src/compat/strnlen.cpp \
    src/fs.cpp \
    src/random.cpp \
    src/rpc/protocol.cpp \
    src/rpc/util.cpp \
    src/support/cleanse.cpp \
    src/sync.cpp \
    src/uint256.cpp \
    src/threadinterrupt.cpp \
    src/util.cpp \
    src/utilmoneystr.cpp \
    src/utilstrencodings.cpp \
    src/utiltime.cpp

CRYPTO_SRC = \
    src/crypto/aes.cpp \
    src/crypto/chacha20.cpp \
    src/crypto/hmac_sha256.cpp \
    src/crypto/hmac_sha512.cpp \
    src/crypto/ripemd160.cpp \
    src/crypto/sha256.cpp \
    src/crypto/sha512.cpp \
    src/crypto/sha1.cpp

COMMON_SRC = \
    src/arith_uint256.cpp \
    src/base58.cpp \
    src/bech32.cpp \
    src/$$TARGET/params.cpp \
    src/coins.cpp \
    src/compressor.cpp \
    src/consensus/merkle.cpp \
    src/core_read.cpp \
    src/core_write.cpp \
    src/hash.cpp \
    src/key.cpp \
    src/keystore.cpp \
    src/netaddress.cpp \
    src/netbase.cpp \
    src/policy/feerate.cpp \
    src/primitives/block.cpp \
    src/primitives/transaction.cpp \
    src/protocol.cpp \
    src/pubkey.cpp \
    src/scheduler.cpp \
    src/script/interpreter.cpp \
    src/script/script.cpp \
    src/script/script_error.cpp \
    src/script/sign.cpp \
    src/script/standard.cpp \
    src/warnings.cpp

QT_BASE_CPP = \
    src/qt/bantablemodel.cpp \
    src/qt/bitcoinaddressvalidator.cpp \
    src/qt/bitcoinamountfield.cpp \
    src/qt/codecoingui.cpp \
    src/qt/bitcoinunits.cpp \
    src/qt/clientmodel.cpp \
    src/qt/csvmodelwriter.cpp \
    src/qt/guiutil.cpp \
    src/qt/intro.cpp \
    src/qt/modaloverlay.cpp \
    src/qt/networkstyle.cpp \
    src/qt/notificator.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/optionsmodel.cpp \
    src/qt/peertablemodel.cpp \
    src/qt/platformstyle.cpp \
    src/qt/qvalidatedlineedit.cpp \
    src/qt/qvaluecombobox.cpp \
    src/qt/rpcconsole.cpp \
    src/qt/splashscreen.cpp \
    src/qt/trafficgraphwidget.cpp \
    src/qt/utilitydialog.cpp

QT_WINDOWS_CPP = qt/winshutdownmonitor.cpp

QT_WALLET_CPP = \
    src/qt/addressbookpage.cpp \
    src/qt/addresstablemodel.cpp \
    src/qt/askpassphrasedialog.cpp \
    src/qt/coincontroldialog.cpp \
    src/qt/coincontroltreewidget.cpp \
    src/qt/editaddressdialog.cpp \
    src/qt/openuridialog.cpp \
    src/qt/overviewpage.cpp \
    src/qt/paymentrequestplus.cpp \
    src/qt/paymentserver.cpp \
    src/qt/receivecoinsdialog.cpp \
    src/qt/receiverequestdialog.cpp \
    src/qt/recentrequeststablemodel.cpp \
    src/qt/sendcoinsdialog.cpp \
    src/qt/sendcoinsentry.cpp \
    src/qt/signverifymessagedialog.cpp \
    src/qt/transactiondesc.cpp \
    src/qt/transactiondescdialog.cpp \
    src/qt/transactionfilterproxy.cpp \
    src/qt/transactionrecord.cpp \
    src/qt/transactiontablemodel.cpp \
    src/qt/transactionview.cpp \
    src/qt/walletframe.cpp \
    src/qt/walletmodel.cpp \
    src/qt/walletmodeltransaction.cpp \
    src/qt/walletview.cpp

QT_SRC = $$QT_BASE_CPP $$QT_WALLET_CPP

MINER_SRC = \
    src/rpc/mining.cpp \
    src/miner.cpp

WALLET_SRC = \
    src/wallet/crypter.cpp \
    src/wallet/db.cpp \
    src/wallet/wallet.cpp \
    src/wallet/walletdb.cpp \
    src/wallet/walletutil.cpp \
    src/wallet/feebumper.cpp \
    src/wallet/fees.cpp \
    src/wallet/init.cpp \
    src/wallet/rpcdump.cpp \
    src/wallet/rpcwallet.cpp \
    src/policy/rbf.cpp \
    src/script/ismine.cpp

SOURCES += \
    src/validation.cpp \
    src/init.cpp \
    src/rpc/server.cpp \
    $$UTIL_SRC \
    $$CRYPTO_SRC \
    $$COMMON_SRC \
    $$QT_SRC \
    $$WALLET_SRC \
    $$MINER_SRC \
    src/addrman.cpp \
    src/addrdb.cpp \
    src/blockencodings.cpp \
    src/chain.cpp \
    src/checkpoints.cpp \
    src/consensus/tx_verify.cpp \
    src/dbwrapper.cpp \
    src/httprpc.cpp \
    src/httpserver.cpp \
    src/merkleblock.cpp \
    src/net.cpp \
    src/net_processing.cpp \
    src/noui.cpp \
    src/policy/fees.cpp \
    src/policy/policy.cpp \
    src/pow.cpp \
    src/rest.cpp \
    src/rpc/blockchain.cpp \
    src/rpc/misc.cpp \
    src/rpc/net.cpp \
    src/rpc/safemode.cpp \
    src/rpc/rawtransaction.cpp \
    src/script/bitcoinconsensus.cpp \
    src/script/sigcache.cpp \
    src/timedata.cpp \
    src/torcontrol.cpp \
    src/txdb.cpp \
    src/txmempool.cpp \
    src/ui_interface.cpp \
    src/validationinterface.cpp \
    src/versionbits.cpp \
    src/undo.cpp \
    src/bloom.cpp \
    src/rpc/client.cpp

SOURCES += src/qt/codecoin.cpp

contains(STAKE, 1){
HEADERS += src/kernel.h src/kernelrecord.h \
    src/kernel.h \
    src/qt/mintingview.h \
    src/qt/mintingtablemodel.h \
    src/qt/mintingfilterproxy.h \
    src/kernelrecord.h 
SOURCES += src/kernel.cpp src/kernelrecord.cpp \
    src/qt/mintingview.cpp \
    src/qt/mintingtablemodel.cpp \
    src/qt/mintingfilterproxy.cpp 
}

contains(HASHX11, 1){
SOURCES += src/cubehash.c src/luffa.c src/aes_helper.c \
    src/echo.c src/shavite.c src/simd.c src/blake.c \
    src/bmw.c src/groestl.c src/jh.c src/keccak.c src/skein.c
}

contains(HASHSCRYPT, 1){
HEADERS += src/crypto/scrypt.h
SOURCES += src/crypto/scrypt.cpp
}

# TODO: figure out how to dereference COIN_BRAND properly
RESOURCES += src/$$TARGET/qt/codecoin.qrc 

#src/qt/codecoin_locale.qrc

FORMS += \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/coincontroldialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/helpmessagedialog.ui \
    src/qt/forms/intro.ui \
    src/qt/forms/modaloverlay.ui \
    src/qt/forms/openuridialog.ui \
    src/qt/forms/optionsdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/receivecoinsdialog.ui \
    src/qt/forms/receiverequestdialog.ui \
    src/qt/forms/debugwindow.ui \
    src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/signverifymessagedialog.ui \
    src/qt/forms/transactiondescdialog.ui

#use sometime later?
MULTISIG_FORMS = \
    src/qt/forms/multisigaddressentry.ui \
    src/qt/forms/multisiginputentry.ui \
    src/qt/forms/multisigdialog.ui

contains(USE_QRCODE, 1) {
HEADERS += src/qt/qrcodedialog.h
SOURCES += src/qt/qrcodedialog.cpp
FORMS += src/qt/forms/qrcodedialog.ui
}

contains(CODECOIN_QT_TEST, 1) {
SOURCES += src/qt/test/test_main.cpp \
    src/qt/test/uritests.cpp
HEADERS += src/qt/test/uritests.h
DEPENDPATH += src/qt/test
QT += testlib
TARGET = codecoin-qt_test
DEFINES += BITCOIN_QT_TEST
  macx: CONFIG -= app_bundle
}

contains(USE_SSE2, 1) {
    DEFINES += USE_SSE2
    gccsse2.input  = SOURCES_SSE2
    gccsse2.output = $$PWD/build/${QMAKE_FILE_BASE}.o
    gccsse2.commands = $(CXX) -c $(CXXFLAGS) $(INCPATH) -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME} -msse2 -mstackrealign
    QMAKE_EXTRA_COMPILERS += gccsse2
    SOURCES_SSE2 += src/scrypt-sse2.cpp
}

# use: qmake "STATICBUILD=1"
contains(STATICBUILD, 1) {
    !macx:CONFIG += static
    !macx:QMAKE_LFLAGS *= -static
}

# Todo: Remove this line when switching to Qt5, as that option was removed
CODECFORTR = UTF-8

# for lrelease/lupdate
# also add new translations to src/qt/codecoin.qrc under translations/
TRANSLATIONS = $$files(src/qt/locale/codecoin_*.ts)

isEmpty(QMAKE_LRELEASE) {
    #if you want to do a native windows build, uncomment 2 lines below, comment 3rd
    #win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    #else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(QM_DIR):QM_DIR = $$PWD/src/qt/locale
# automatically build translations, so they can be included in resource file
TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$QM_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM

# "Other files" to show in Qt Creator
OTHER_FILES += README.md \
    doc/*.rst \
    doc/*.txt \
    doc/*.md \
    src/qt/res/bitcoin-qt.rc \
    src/test/*.cpp \
    src/test/*.h \
    src/qt/test/*.cpp \
    src/qt/test/*.h

# this is probably brokeage, but its simple and mostly works
# https://gist.github.com/buzzySmile/d46f537a810a7d97efc7
# https://github.com/jmesmon/qmake-protobuf-example
PROTOS = src/qt/paymentrequest.proto
include(src/qt/protobuf.pri)

# platform specific defaults, if not overridden on command line
isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = -mt
    windows:BOOST_LIB_SUFFIX = -mgw44-mt-1_53
    android:BOOST_LIB_SUFFIX = -gcc-mt-1_53
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

isEmpty(BDB_LIB_PATH) {
    windows:BDB_LIB_PATH = deps/lib
    macx:BDB_LIB_PATH = $${BREW}/$${BDB}/lib
    android:BDB_LIB_PATH = deps/lib
}

isEmpty(BDB_LIB_SUFFIX) {
    macx:BDB_LIB_SUFFIX = -5.3
}

isEmpty(BDB_INCLUDE_PATH) {
    windows:BDB_INCLUDE_PATH = deps/include
    macx:BDB_INCLUDE_PATH = $${BREW}/$${BDB}/include
    android:BDB_INCLUDE_PATH = deps/include
}

isEmpty(BOOST_LIB_PATH) {
    macx:BOOST_LIB_PATH = $${BREW}/$${BOOST}/lib
}

isEmpty(BOOST_INCLUDE_PATH) {
    macx:BOOST_INCLUDE_PATH = $${BREW}/$${BOOST}/include
}

win32:DEFINES += WIN32
win32:RC_FILE = src/qt/res/codecoin-qt.rc

win32:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

!win32:!macx {
    DEFINES += LINUX
    !android:LIBS += -lrt
    # _FILE_OFFSET_BITS=64 lets 32-bit fopen transparently support large files.
    DEFINES += _FILE_OFFSET_BITS=64
}

macx:HEADERS += src/qt/macdockiconhandler.h src/qt/macnotificationhandler.h
macx:OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm src/qt/macnotificationhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit -framework CoreServices
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0 HAVE_CXX_STDHEADERS
macx:ICON = src/$${TARGET}/qt/icons/coin.icns
macx:QMAKE_CFLAGS_THREAD += -pthread
macx:QMAKE_LFLAGS_THREAD += -pthread
macx:QMAKE_CXXFLAGS_THREAD += -pthread
macx:QMAKE_INFO_PLIST = src/$${TARGET}/qt/Info.plist
macx:OPENSSL_INCLUDE_PATH = $${BREW}/$${SSL}/include
macx:OPENSSL_LIB_PATH = $${BREW}/$${SSL}/lib

# Set libraries and includes at end, to use platform-defined defaults if not overridden
INCLUDEPATH += $$BOOST_INCLUDE_PATH $$BDB_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$QRENCODE_INCLUDE_PATH $$OS_INCLUDE_PATH
LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,) \
    $$join(QRENCODE_LIB_PATH,,-L,) $$join(OS_LIB_PATH,,-L,) $$join(PROTOBUF_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX -lprotobuf -lunivalue -lsecp256k1 -levent_pthreads -levent -lleveldb
# -lgdi32 has to happen after -lcrypto (see  #681)
win32:LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32
LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX
win32:LIBS += -lboost_chrono$$BOOST_LIB_SUFFIX
macx:LIBS += -lboost_chrono$$BOOST_LIB_SUFFIX

# for mingw cross-compile build:
win32:LIBS += -lpthread

contains(RELEASE, 1) {
    !win32:!macx {
        # Linux: turn dynamic linking back on for c/c++ runtime libraries
        LIBS += -Wl,-Bdynamic
    }
}

#DISTFILES += \
#    android/gradle/wrapper/gradle-wrapper.jar \
#    android/AndroidManifest.xml \
#    android/gradlew.bat \
#    android/res/values/libs.xml \
#    android/build.gradle \
#    android/gradle/wrapper/gradle-wrapper.properties \
#    android/gradlew

#ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
