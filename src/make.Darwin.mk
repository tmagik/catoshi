OS_DEFS=-DMAC_OSX

BREW=/usr/local/opt

BOOST=boost\@1.57
BDB=berkeley-db\@53
OPENSSL=openssl\@1.1

BOOST_LIB_SUFFIX=-mt


OS_EXTRA_INCLUDE_PATH= \
	$(BREW)/$(OPENSSL)/include \
	$(BREW)/$(BOOST)/include \
	$(BREW)/$(BDB)/include

OS_EXTRA_LIB_PATH = \
	$(BREW)/$(OPENSSL)/lib \
	$(BREW)/$(BOOST)/lib \
	$(BREW)/$(BDB)/lib \
	/usr/local/lib

OS_LDFLAGS = $(addprefix -L,$(OS_EXTRA_LIB_PATH))
