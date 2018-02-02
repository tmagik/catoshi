// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2009-2012 *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_VERSION_H
#define CODECOIN_VERSION_H

/**
 * network protocol versioning
 */

#if 0 /* legacy versions */
#if defined(BRAND_ppcoin) || defined(BRAND_bluecoin)
static const int PROTOCOL_VERSION = 71001;
#elif defined(BRAND_givecoin)
static const int PROTOCOL_VERSION = 70003; // bump to 71001 for PoS?
static const int MIN_PEER_PROTO_VERSION = 70002;
#elif defined(BRAND_uro)
static const int PROTOCOL_VERSION = 70013;
static const int MIN_PEER_PROTO_VERSION = 70012;
#elif defined(BRAND_grantcoin) || defined(BRAND_cleanwatercoin)
// grantstake should probably actually use 70002
static const int PROTOCOL_VERSION = 70001;
static const int MIN_PEER_PROTO_VERSION = 60006;
//! initial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 209;
#endif
#endif

/* Follow Bitcoin-core unless there is a really good reason */

static const int PROTOCOL_VERSION = 70012;

//! initial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 209;

//! In this version, 'getheaders' was introduced.
static const int GETHEADERS_VERSION = 70002;

//! disconnect from peers older than this proto version
static const int MIN_PEER_PROTO_VERSION = GETHEADERS_VERSION;

//! nTime field added to CAddress, starting with this version;
//! if possible, avoid requesting addresses nodes older than this
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

//! BIP 0031, pong message, is enabled for all versions AFTER this one
static const int BIP0031_VERSION = 60000;

//! "filter*" commands are disabled without NODE_BLOOM after and including this version
static const int NO_BLOOM_VERSION = 70011;

//! "sendheaders" command and announcing blocks with headers starts with this version
static const int SENDHEADERS_VERSION = 70012;

//! "feefilter" tells peers to filter invs to you by fee starts with this version
static const int FEEFILTER_VERSION = 70013;

//! short-id-based block download starts with this version
static const int SHORT_IDS_BLOCKS_VERSION = 70014;

//! not banning for invalid compact blocks starts with this version
static const int INVALID_CB_NO_BAN_VERSION = 70015;

#endif // CODECOIN_VERSION_H
