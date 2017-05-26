// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_grantcoin_H
#define CODECOIN_grantcoin_H

/* ========= Things defined in bitcoin@0.12/consensus/consensus.h  ========= */

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE =   1000000; // Segwit will move this around
static const unsigned int MAX_BLOCK_SIZE_GEN = 1000000/2;

/** The maximum allowed number of signature check operations in a block (network rule) */
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;

/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 500;

/** Flags for nSequence and nLockTime locks */
#warning "check validity for grantcoin"
enum {
    /* Interpret sequence numbers as relative lock-time constraints. */
    LOCKTIME_VERIFY_SEQUENCE = (1 << 0),

    /* Use GetMedianTimePast() instead of nTime for end point timestamp. */
    LOCKTIME_MEDIAN_TIME_PAST = (1 << 1),
};

/* ========= End defines from bitcoin@0.12/consensus/consensus.h  ========= */

static const int RPC_PORT = 9983;
static const int RPC_PORT_TESTNET = 9985;

//static const int P2P_PORT = 9982; // deprecated, in params.cpp
//static const int P2P_PORT_TESTNET = 9984; // deprecated, in params.cpp

static const int64_t COIN = 1000000;
static const int64_t CENT = 10000;
static const int COIN_DECIMALS = 6; /* decimal places for coin */
#define COIN_DECIMALS_FMT "06"

/** No amount larger than this (in satoshi) is valid */
static const int64_t MAX_MONEY = 50000000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }


static const int STAKE_TARGET_SPACING = 1.5 * 60; // 90-second block spacing 
static const unsigned int nStakeMinAge = 60 * 60 * 24; // minimum age for coin age (24 hours)
static const unsigned int nStakeMaxAge = 60 * 60 * 24 * 90; // stake age of full weight
//static const int64 START_BLOCK_PROOF_OF_STAKE = 250000; // PoS allowed starting at this block

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

/* allow for overloads.. */
#define CTransaction TransactionGRT
#define CMutableTransaction MutableTransactionGRT
#define CBlockHeader BlockHeader
#define CBlock BlockGRT

#endif
