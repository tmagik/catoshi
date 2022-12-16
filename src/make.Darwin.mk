OS_DEFS=-DMAC_OSX


ifdef BREW
BREW=/usr/local/opt

BOOST=boost\@1.66
BDB=berkeley-db\@53
OPENSSL=openssl\@1.1

OS_EXTRA_INCLUDE_PATH= \
        $(BREW)/$(OPENSSL)/include \
        $(BREW)/$(BOOST)/include \
        $(BREW)/$(BDB)/include

OS_EXTRA_LIB_PATH = \
        $(BREW)/$(OPENSSL)/lib \
        $(BREW)/$(BOOST)/lib \
        $(BREW)/$(BDB)/lib \
	/usr/local/lib
else
OS_EXTRA_INCLUDE_PATH= \
	/opt/local/include \
	/usr/local/BerkeleyDB.5.3/include

OS_EXTRA_LIB_PATH= \
	/opt/local/lib \
	/usr/local/BerkeleyDB.5.3/lib
endif

BOOST_LIB_SUFFIX=-mt

OS_LDFLAGS = $(addprefix -L,$(OS_EXTRA_LIB_PATH))
