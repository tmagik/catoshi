// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_hamburger_H
#define CODECOIN_hamburger_H

static const int RPC_PORT = 3123;
static const int P2P_PORT = 3234;
static const int RPC_PORT_TESTNET = 4123;
static const int P2P_PORT_TESTNET = 4234;

static const int64_t COIN = 1000000;
static const int64_t CENT = 10000;
static const int COIN_DECIMALS = 6; /* decimal places for coin */
#define COIN_DECIMALS_FMT "06"

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE = 1000000;
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;

/** Dust Soft Limit, allowed with additional fee per output */
static const int64_t DUST_SOFT_LIMIT = 10000; // 0.001 HAM
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64_t DUST_HARD_LIMIT = 100;   // 0.00001 HAM mininput

/** Minimum criteria for CTransaction:AllowFree */
static const int64_t MIN_FREE_PRIORITY = COIN * 576/250;
/** No amount larger than this (in catoshi) is valid */
static const int64_t MAX_MONEY = 500000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

/** Minimum block time spacing (hard limit) **/
static const int64_t MINIMUM_BLOCK_SPACING = 15;	// Absolute minimum spacing

/** really only used in rpcmining.cpp **/
static const int RETARGET_INTERVAL = 15;

#define STAKE_TARGET_SPACING nStakeTargetSpacing

// MODIFIER_INTERVAL: time to elapse before new modifier is computed
static const unsigned int MODIFIER_INTERVAL = 3 * 60 * 60;  // 3 hours
extern unsigned int nModifierInterval;

// MODIFIER_INTERVAL_RATIO:
// ratio of group interval length between the last group and the first group
static const int MODIFIER_INTERVAL_RATIO = 3;

extern const unsigned int nStakeMinAge;
extern const unsigned int nStakeMaxAge;
extern const unsigned int nStakeTargetSpacing;
extern const unsigned int nMaxClockDrift;
//extern const unsigned int nMinTxOutAmount; // set to = CTransaction::nMinTxFee;

#define CUTOFF_POS_BLOCK nCutoff_Pos_Block
extern const int CUTOFF_POS_BLOCK;

#define BRAND "Hamburger"
#define BRAND_upper "Hamburger"
#define BRAND_lower "hamburger"
#define BRAND_domain "hamburger.org"
#define BRAND_CODE "HAM"

#define PPCOINSTAKE
#define FEATURE_MONEYSUPPLY

#endif
