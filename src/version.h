// Copyright (c) 2012 The Bitcoin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_VERSION_H
#define CODECOIN_VERSION_H

#include "clientversion.h"
#include <string>

//
// client versioning
//

static const int CLIENT_VERSION =
                           1000000 * CLIENT_VERSION_MAJOR
                         +   10000 * CLIENT_VERSION_MINOR
                         +     100 * CLIENT_VERSION_REVISION
                         +       1 * CLIENT_VERSION_BUILD;

extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;
extern const std::string CLIENT_DATE;

//
// network protocol versioning
//

// intial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 209;

#if defined(BRAND_ppcoin) || defined(BRAND_bluecoin)
static const int PROTOCOL_VERSION = 71001;
#elif defined(BRAND_givecoin)
static const int PROTOCOL_VERSION = 70003; // bump to 71001 for PoS?
static const int MIN_PEER_PROTO_VERSION = 70002;
#elif defined(BRAND_uro)
static const int PROTOCOL_VERSION = 70013;
static const int MIN_PEER_PROTO_VERSION = 70012;
#else
static const int PROTOCOL_VERSION = 70002;
static const int MIN_PEER_PROTO_VERSION = 70002;
#endif


// earlier versions not supported as of Feb 2012, and are disconnected
// NOTE: as of bitcoin v0.6 message serialization (vSend, vRecv) still
// uses MIN_PROTO_VERSION(209), where message format uses PROTOCOL_VERSION
static const int MIN_PROTO_VERSION = 209;

// nTime field added to CAddress, starting with this version;
// if possible, avoid requesting addresses nodes older than this
static const int CADDR_TIME_VERSION = 31402;

#if defined(BRAND_bluecoin) || defined (BRAND_ppcoin)
// TODO: move to some sort of codecoin object
// only request blocks from nodes outside this range of versions
static const int NOBLKS_VERSION_START = 60002;
static const int NOBLKS_VERSION_END = 70900;
#else
// only request blocks from nodes outside this range of versions
static const int NOBLKS_VERSION_START = 32000;
static const int NOBLKS_VERSION_END = 32400;
#endif

// BIP 0031, pong message, is enabled for all versions AFTER this one
static const int BIP0031_VERSION = 60000;

// "mempool" command, enhanced "getdata" behavior starts with this version:
static const int MEMPOOL_GD_VERSION = 60002;

#endif
