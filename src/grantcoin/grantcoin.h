// Copyright (c) 2014-2021 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_grantcoin_H
#define CODECOIN_grantcoin_H

#include "grantcoin/consensus.h"

/* ========= Things migrated from ~bitcoin@.15/policy/policy.h    ========= */

/** Default for -maxmempool, maximum megabytes of mempool memory usage */
static const unsigned int DEFAULT_MAX_MEMPOOL_SIZE = 500;

/** Min feerate for defining dust. Historically this has been based on the
 * minRelayTxFee, however changing the dust limit changes which transactions are
 * standard and should be done with care and ideally rarely. It makes sense to
 * only increase the dust limit after prior releases were already not creating
 * outputs below the new threshold */
static const unsigned int DUST_RELAY_TX_FEE = 3000;

/* ========= End defines from bitcoin@0.14/policy/policy.h        ========= */

static const int RPC_PORT = 9983;
static const int RPC_PORT_TESTNET = 9985;

/* from net.h */
/** Maximum length of incoming protocol messages (no message over 4 MB is currently accep    table). */
static const unsigned int MAX_PROTOCOL_MESSAGE_LENGTH = 4 * 1000 * 1000;

//! Follow bitcoin-core unles there is a really good reason
// see version.h // static const int PROTOCOL_VERSION = 70012
//! minimum version we will talk to
static const int MIN_PEER_PROTO_VERSION = 60004;

static const int64_t COIN = 1000000;
static const int64_t CENT = 10000;
static const int64_t PERKB_TX_FEE = CENT;
static const int COIN_DECIMALS = 6; /* decimal places for coin */
#define COIN_DECIMALS_FMT "06"

/** Default for -minrelaytxfee, minimum relay fee for transactions */
static const unsigned int DEFAULT_MIN_RELAY_TX_FEE = CENT;

/** No amount larger than this (in satoshi) is valid */
static const int64_t MAX_MONEY = 50000000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }


static const int STAKE_TARGET_SPACING = 1.5 * 60; // 90-second block spacing 
static const unsigned int nStakeMinAge = 60 * 60 * 24; // minimum age for coin age (24 hours)
static const unsigned int nStakeMaxAge = 60 * 60 * 24 * 90; // stake age of full weight
static const uint64_t START_BLOCK_PROOF_OF_STAKE = 250000; // PoS allowed starting at this block

extern const unsigned int nMaxClockDrift;


/** Dust Soft Limit, allowed with additional fee per output */
static const int64_t DUST_SOFT_LIMIT = CENT; // 0.01 GRT
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64_t DUST_HARD_LIMIT = 100;   // 0.00001 GRT mininput

/** main.h CTransaction:AllowFree **/
static const int MIN_FREE_PRIORITY = COIN * 960 / 250;

/** The interval over which we look to calculate the next difficulty **/
// TODO: not needed for GRT???
static const int RETARGET_INTERVAL = 36;

#define FEATURE_MINIMUM_BLOCK_TIME
/** Minimum block time spacing (hard limit) **/
static const int64_t MINIMUM_BLOCK_SPACING = 60;	// Absolute minimum spacing

#define FEATURE_MONEYSUPPLY
#define FEATURE_CFG_MAXFUTURE
extern int32_t nMaxFutureTime;

#define FEATURE_FUTURE_IS_HARDER

#define BRAND "Grantcoin"
#define BRAND_upper "Grantcoin"
#define BRAND_lower "grantcoin"
#define BRAND_domain "grantcoin.org"
#define BRAND_CODE "GRT"

//FIXME: make a define for stake data structures instead of defined(BRAND_grantcoin)
//#define PPCOINSTAKE

// Faster bootstraps with xz. Probably only works on mac/linux
#define USE_BOOTSTRAP_XZ
#define USE_BOOTSTRAP_GZ	// might work on android too

/* primitives/<stuff> needs this */
#define BITCOIN_COMPAT

/* overload the BITCOIN_COMPAT defines*/
#define CTransaction StakeTx
#define CMutableTransaction MutableStakeTx

#define CBlockHeader BlockHeader

/* TODO make BlockGRT (or StakeBlock??) later */
//#define CBlock BlockGRT
#define CBlock Block

#endif
