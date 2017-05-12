# Copyright (c) 2009-2010 Satoshi Nakamoto
# Copyright (c) 2011-201? The *Coin developers
# where * = (Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
# Previously distributed under the MIT/X11 software license, see the
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
# Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
# Distributed under the Affero GNU General public license version 3
# file COPYING or http://www.gnu.org/licenses/agpl-3.0.html


# :=0 --> UPnP support turned off by default at runtime
# :=1 --> UPnP support turned on by default at runtime
# :=- --> No UPnP support - miniupnp not required
USE_UPNP:=0

# :=1 --> Enable IPv6 support
# :=0 --> Disable IPv6 support
USE_IPV6:=1

# Build wallet
ENABLE_WALLET:=1

ARCH=$(shell arch)

# :=1 --> enable SSE
# :=0 --> disable SSE
ifeq ($(ARCH), i386)
USE_SSE2:=1
else ifeq ($(ARCH), x86_64)
USE_SSE2:=1
else
USE_SSE2:=0
endif

LINK:=$(CXX)

COIN_BRAND:=litecoin

DEFS=-DBOOST_SPIRIT_THREADSAFE -D_FILE_OFFSET_BITS=64 -DBRAND_$(COIN_BRAND)

#debian jessie libboost-filesystem1.55 needs this
DEFS += -DBOOST_NO_CXX11_SCOPED_ENUMS

OBJDIR = obj/$(COIN_BRAND)
TESTDIR = $(OBJDIR)/test
SUBDIRS = $(OBJDIR)/crypto $(OBJDIR)/primitives $(OBJDIR)/script $(OBJDIR)/compat $(OBJDIR)/univalue $(OBJDIR)/$(COIN_BRAND)
SECP256K1_INCLUDE_PATH = $(CURDIR)/secp256k1/include
SECP256K1_LIB_PATH = $(CURDIR)/secp256k1/.libs

DEFS += $(addprefix -I,$(CURDIR) $(CURDIR)/$(OBJDIR) $(BOOST_INCLUDE_PATH) $(BDB_INCLUDE_PATH) $(SECP256K1_INCLUDE_PATH))
LIBS = $(addprefix -L,$(BOOST_LIB_PATH) $(BDB_LIB_PATH) $(SECP256K1_LIB_PATH))
BOOST_LIBS = -lboost_system -lboost_filesystem -lboost_program_options -lboost_thread

TESTDEFS = -DTEST_DATA_DIR=$(abspath test/data)


LMODE = dynamic
LMODE2 = dynamic
ifdef STATIC
	LMODE = static
	ifeq (${STATIC}, all)
		LMODE2 = static
	endif
else
	TESTDEFS += -DBOOST_TEST_DYN_LINK
endif

# for boost 1.37, add -mt to the boost libraries
D_LIBS = $(LIBS) \
 -Wl,-B$(LMODE) \
   -l boost_system$(BOOST_LIB_SUFFIX) \
   -l boost_filesystem$(BOOST_LIB_SUFFIX) \
   -l boost_program_options$(BOOST_LIB_SUFFIX) \
   -l boost_thread$(BOOST_LIB_SUFFIX) \
   -l db_cxx$(BDB_LIB_SUFFIX) \
   -l secp256k1 \
   -l ssl \
   -l crypto \

TESTLIBS += \
 -Wl,-B$(LMODE) \
   -l boost_unit_test_framework$(BOOST_LIB_SUFFIX)

ifndef USE_UPNP
	override USE_UPNP = -
endif
ifneq (${USE_UPNP}, -)
	D_LIBS += -l miniupnpc
	DEFS += -DUSE_UPNP=$(USE_UPNP)
endif

ifneq (${USE_IPV6}, -)
	DEFS += -DUSE_IPV6=$(USE_IPV6)
endif

D_LIBS+= \
 -Wl,-B$(LMODE2) \
   -l z \
   -l dl \
   -l pthread

#TODO: simplify above
TX_LIBS = $(LIBS) $(BOOST_LIBS) -lsecp256k1 -lssl -lcrypto -lpthread
CLI_LIBS = $(LIBS) $(BOOST_LIBS) -lssl -lcrypto

# Hardening
# Make some classes of vulnerabilities unexploitable in case one is discovered.
#
    # This is a workaround for Ubuntu bug #691722, the default -fstack-protector causes
    # -fstack-protector-all to be ignored unless -fno-stack-protector is used first.
    # see: https://bugs.launchpad.net/ubuntu/+source/gcc-4.5/+bug/691722
    HARDENING=-fno-stack-protector

    # Stack Canaries
    # Put numbers at the beginning of each stack frame and check that they are the same.
    # If a stack buffer if overflowed, it writes over the canary number and then on return
    # when that number is checked, it won't be the same and the program will exit with
    # a "Stack smashing detected" error instead of being exploited.
    HARDENING+=-fstack-protector-all -Wstack-protector

    # Make some important things such as the global offset table read only as soon as
    # the dynamic linker is finished building it. This will prevent overwriting of addresses
    # which would later be jumped to.
    LDHARDENING+=-Wl,-z,relro -Wl,-z,now

    # Build position independent code to take advantage of Address Space Layout Randomization
    # offered by some kernels.
    # see doc/build-unix.txt for more information.
    PIE=1
    ifdef PIE
        HARDENING+=-fPIE
        LDHARDENING+=-pie
    endif

    # -D_FORTIFY_SOURCE=2 does some checking for potentially exploitable code patterns in
    # the source such overflowing a statically defined buffer.
    HARDENING+=-D_FORTIFY_SOURCE=2
#


DEBUGFLAGS=-g

# CXXFLAGS can be specified on the make command line, so we use xCXXFLAGS that only
# adds some defaults in front. Unfortunately, CXXFLAGS=... $(CXXFLAGS) does not work.
xCXXFLAGS=-O2 -std=c++11 -pthread -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter \
    $(DEBUGFLAGS) $(DEFS) $(HARDENING) $(CXXFLAGS)

# LDFLAGS can be specified on the make command line, so we use xLDFLAGS that only
# adds some defaults in front. Unfortunately, LDFLAGS=... $(LDFLAGS) does not work.
xLDFLAGS=$(LDHARDENING) $(LDFLAGS)

STAKE_OBJS = \
    $(OBJDIR)/kernel.o \
    $(OBJDIR)/kernelrecord.o

UTIL_OBJS= \
    $(OBJDIR)/compat/glibc_sanity.o \
    $(OBJDIR)/compat/glibcxx_sanity.o \
    $(OBJDIR)/compat/strnlen.o \
    $(OBJDIR)/$(COIN_BRAND)/base.o \
    $(OBJDIR)/clientversion.o \
    $(OBJDIR)/random.o \
    $(OBJDIR)/rpcprotocol.o \
    $(OBJDIR)/rpcclient.o \
    $(OBJDIR)/sync.o \
    $(OBJDIR)/uint256.o \
    $(OBJDIR)/util.o \
    $(OBJDIR)/utilmoneystr.o \
    $(OBJDIR)/utilstrencodings.o \
    $(OBJDIR)/utiltime.o 

CRYPTO_OBJS = \
    $(OBJDIR)/crypto/hmac_sha256.o \
    $(OBJDIR)/crypto/hmac_sha512.o \
    $(OBJDIR)/crypto/ripemd160.o \
    $(OBJDIR)/crypto/rfc6979_hmac_sha256.o \
    $(OBJDIR)/crypto/sha256.o \
    $(OBJDIR)/crypto/sha512.o \
    $(OBJDIR)/crypto/sha1.o

UNIVALUE_OBJS = \
    $(OBJDIR)/univalue/univalue.o \
    $(OBJDIR)/univalue/univalue_read.o \
    $(OBJDIR)/univalue/univalue_write.o \

COMMON_OBJS = \
    $(OBJDIR)/allocators.o \
    $(OBJDIR)/amount.o \
    $(OBJDIR)/base58.o \
    $(OBJDIR)/$(COIN_BRAND)/params.o \
    $(OBJDIR)/coins.o \
    $(OBJDIR)/compressor.o \
    $(OBJDIR)/core_read.o \
    $(OBJDIR)/core_write.o \
    $(OBJDIR)/ecwrapper.o \
    $(OBJDIR)/eccryptoverify.o \
    $(OBJDIR)/hash.o \
    $(OBJDIR)/key.o \
    $(OBJDIR)/keystore.o \
    $(OBJDIR)/netbase.o \
    $(OBJDIR)/primitives/block.o \
    $(OBJDIR)/primitives/transaction.o \
    $(OBJDIR)/protocol.o \
    $(OBJDIR)/pubkey.o \
    $(OBJDIR)/script/interpreter.o \
    $(OBJDIR)/script/script.o \
    $(OBJDIR)/script/script_error.o \
    $(OBJDIR)/script/sign.o \
    $(OBJDIR)/script/standard.o


#put the longest compiling things first for parallel make
D_OBJS= \
    $(OBJDIR)/main.o \
    $(OBJDIR)/init.o \
    $(OBJDIR)/rpcserver.o \
    $(OBJDIR)/net.o \
    $(UNIVALUE_OBJS) \
    $(UTIL_OBJS) \
    $(CRYPTO_OBJS) \
    $(COMMON_OBJS) \
    $(OBJDIR)/addrman.o \
    $(OBJDIR)/alert.o \
    $(OBJDIR)/bitcoind.o \
    $(OBJDIR)/crypter.o \
    $(OBJDIR)/chain.o \
    $(OBJDIR)/checkpoints.o \
    $(OBJDIR)/crypter.o \
    $(OBJDIR)/db.o \
    $(OBJDIR)/merkleblock.o \
    $(OBJDIR)/pow.o \
    $(OBJDIR)/rest.o \
    $(OBJDIR)/rpcblockchain.o \
    $(OBJDIR)/rpcmisc.o \
    $(OBJDIR)/rpcnet.o \
    $(OBJDIR)/rpcrawtransaction.o \
    $(OBJDIR)/script/bitcoinconsensus.o \
    $(OBJDIR)/script/sigcache.o \
    $(OBJDIR)/timedata.o \
    $(OBJDIR)/txmempool.o \
    $(OBJDIR)/undo.o \
    $(OBJDIR)/bloom.o \
    $(OBJDIR)/noui.o \
    $(OBJDIR)/leveldbwrapper.o \
    $(OBJDIR)/txdb.o

#MINER_OBJS
D_OBJS += \
    $(OBJDIR)/rpcmining.o \
    $(OBJDIR)/miner.o

WALLET_OBJS = \
    $(OBJDIR)/wallet.o \
    $(OBJDIR)/walletdb.o \
    $(OBJDIR)/wallet_ismine.o \
    $(OBJDIR)/rpcdump.o \
    $(OBJDIR)/rpcwallet.o 

X11_OBJS = \
    $(OBJDIR)/cubehash.o \
    $(OBJDIR)/luffa.o \
    $(OBJDIR)/aes_helper.o \
    $(OBJDIR)/echo.o \
    $(OBJDIR)/shavite.o \
    $(OBJDIR)/simd.o \
    $(OBJDIR)/blake.o \
    $(OBJDIR)/bmw.o \
    $(OBJDIR)/groestl.o \
    $(OBJDIR)/jh.o \
    $(OBJDIR)/keccak.o \
    $(OBJDIR)/skein.o

SCRYPT_OBJS = $(OBJDIR)/crypto/scrypt.o

ifeq ($(ENABLE_WALLET),1)
DEFS+= -DENABLE_WALLET
D_OBJS+= $(WALLET_OBJS)
endif

ifeq ($(USE_SSE2),1)
DEFS += -DUSE_SSE2
OBJS_SSE2= $(OBJDIR)/crypto/scrypt-sse2.o
SCRYPT_OBJS += $(OBJS_SSE2)
endif

ifeq ($(COIN_BRAND),litecoin)
DEFS += -DUSE_SCRYPT
COMMON_OBJS += $(SCRYPT_OBJS)
endif
#ifeq ($(COIN_BRAND),grantcoin)
#COMMON_OBJS += $(STAKE_OBJS)
#endif
ifeq ($(COIN_BRAND),grantstake)
COMMON_OBJS += $(STAKE_OBJS)
endif
ifeq ($(COIN_BRAND),uro)
COMMON_OBJS += $(X11_OBJS)
endif
ifeq ($(COIN_BRAND),givecoin)
COMMON_OBJS += $(X11_OBJS)
endif
ifeq ($(COIN_BRAND),hamburger)
DEFS += -DUSE_SCRYPT
COMMON_OBJS += $(SCRYPT_OBJS) $(STAKE_OBJS)
endif
ifeq ($(COIN_BRAND),kittycoin)
DEFS += -DUSE_SCRYPT
COMMON_OBJS += $(SCRYPT_OBJS)
endif
ifeq ($(COIN_BRAND),bluecoin)
COMMON_OBJS += $(SCRYPT_OBJS) $(X11_OBJS) $(STAKE_OBJS)
endif

TX_OBJS= $(UTIL_OBJS) $(UNIVALUE_OBJS) $(COMMON_OBJS) $(CRYPTO_OBJS) bitcoin-tx.cpp
CLI_OBJS= $(UTIL_OBJS) bitcoin-cli.cpp

all: $(SUBDIRS) $(TESTDIR) $(COIN_BRAND)d $(COIN_BRAND)-cli $(COIN_BRAND)-tx

test check: $(TESTDIR) test_$(COIN_BRAND) FORCE
	./test_$(COIN_BRAND)

regression: test FORCE
	$(MAKE) COIN_BRAND=litecoin test
#	$(MAKE) COIN_BRAND=grantcoin test
#	$(MAKE) COIN_BRAND=grantstake test
#	$(MAKE) COIN_BRAND=hamburger test
#	$(MAKE) COIN_BRAND=kittycoin test
	

$(TESTDIR):
	mkdir -p $(TESTDIR)

$(SUBDIRS):
	mkdir -p $(SUBDIRS)

#
# LevelDB support
#
MAKEOVERRIDES =
D_LIBS += -lleveldb 

# auto-generated dependencies:
-include $(OBJDIR)/*.P
-include $(OBJDIR)/test/*.P

ifeq ($(V),1)
V_CXX=
V_LINK=
else
V_CXX=@echo "   $(CXX)        $<" ;
V_LNK=@echo "   $(LINK)       $<" ;
endif

$(OBJDIR)/build.h: FORCE
	/bin/sh ../share/genbuild.sh $(OBJDIR)/build.h
clientversion.cpp: $(OBJDIR)/build.h
DEFS += -DHAVE_BUILD_INFO -DHAVE_SYS_SELECT_H

$(OBJDIR)/%-sse2.o: %-sse2.cpp
	$(V_CXX) $(CXX) -c $(xCXXFLAGS) -msse2 -MMD -MF $(@:%.o=%.d) -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

$(OBJDIR)/%.o: %.c
	$(V_CXX) $(CXX) -c $(xCXXFLAGS) -fpermissive -MMD -MF $(@:%.o=%.d) -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

$(OBJDIR)/%.o: %.cpp
	$(V_CXX) $(CXX) -c $(xCXXFLAGS) -MMD -MF $(@:%.o=%.d) -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

$(COIN_BRAND)d: $(D_OBJS:$(OBJDIR)/%=$(OBJDIR)/%)
	$(V_LINK) $(LINK) $(xCXXFLAGS) -o $@ $^ $(xLDFLAGS) $(D_LIBS)

$(COIN_BRAND)-tx: $(TX_OBJS:$(OBJDIR)/%=$(OBJDIR)/%)
	$(V_LINK) $(LINK) $(xCXXFLAGS) -o $@ $^ $(xLDFLAGS) $(TX_LIBS)

$(COIN_BRAND)-cli: $(CLI_OBJS:$(OBJDIR)/%=$(OBJDIR)/%)
	$(V_LINK) $(LINK) $(xCXXFLAGS) -o $@ $^ $(xLDFLAGS) $(CLI_LIBS)




TESTOBJS := $(patsubst test/%.cpp,$(OBJDIR)/test/%.o,$(wildcard test/*.cpp))

$(OBJDIR)/test/%.o: test/%.cpp
	$(CXX) -c $(TESTDEFS) $(xCXXFLAGS) -MMD -MF $(@:%.o=%.d) -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

test_$(COIN_BRAND): $(TESTOBJS) $(filter-out $(OBJDIR)/init.o,$(D_OBJS:$(OBJDIR)/%=$(OBJDIR)/%))
	$(LINK) $(xCXXFLAGS) -o $@ $(LIBPATHS) $^ $(TESTLIBS) $(xLDFLAGS) $(LIBS)

clean:
	-rm -f $(COIN_BRAND)d $(COIN_BRAND)-tx $(COIN_BRAND)-cli test_$(COIN_BRAND)
	-rm -f $(OBJDIR)/*.o
	-rm -f $(OBJDIR)/*/*.o
	-rm -f $(OBJDIR)/*.P
	-rm -f $(OBJDIR)/*/*.P
	-rm -f $(OBJDIR)/build.h
	-cd leveldb && $(MAKE) clean || true

FORCE:
