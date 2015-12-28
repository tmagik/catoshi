// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_cleanwatercoin_H
#define CODECOIN_cleanwatercoin_H

static const int RPC_PORT = 53590;
static const int RPC_PORT_TESTNET =43590;
static const int P2P_PORT = 53591;
static const int P2P_PORT_TESTNET = 43591;

static const int64_t COIN = 1000000;
static const int64_t CENT = 10000;
static const int COIN_DECIMALS = 6; /* decimal places for coin */
#define COIN_DECIMALS_FMT "06"

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE = 1500000;
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;

/** Dust Soft Limit, allowed with additional fee per output */
static const int64_t DUST_SOFT_LIMIT = 10000; // 0.01 WATER
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64_t DUST_HARD_LIMIT = 100;   // 0.0001 GIVE mininput
/** Minimum criteria for AllowFree */
static const int64_t MIN_FREE_PRIORITY = COIN * 576/250;
/** No amount larger than this (in catoshi) is valid */
static const int64_t MAX_MONEY = 1000000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

/** Minimum block time spacing (AcceptBlockTimestamp determines if this is a hard limit or not **/
static const int64_t MINIMUM_BLOCK_SPACING = 30;

static const int64_t MAX_MINT_PROOF_OF_STAKE = 0.08 * COIN;	// 8% annual interest  default for testing

#define CHARITY_ADDRESS_TESTNET "nVagPzHPNywPCGCXUmsEnfjnCSSYkCz6eB" // Cleanwatercoin Charity Address in TestNet
#define CHARITY_ADDRESS "5anJYFYW7npP4MyNaAtzTLb5ek2Lxgyb7d" // Cleanwatercoin Charity Address
static const int64_t CHARITY_DEFAULT_AMOUNT = 100 * COIN;  // Default amount of Charity sent

/** really only used in rpcmining.cpp **/
static const int RETARGET_INTERVAL = 15;

#define STAKE_TARGET_SPACING nStakeTargetSpacing
#define COINBASE_MATURITY nCoinbaseMaturity

// MODIFIER_INTERVAL: time to elapse before new modifier is computed
static const unsigned int MODIFIER_INTERVAL = 6 * 60 * 60; 
extern unsigned int nModifierInterval;

// MODIFIER_INTERVAL_RATIO:
// ratio of group interval length between the last group and the first group
static const int MODIFIER_INTERVAL_RATIO = 3;

extern unsigned int nStakeMinAge;
extern unsigned int nStakeMaxAge;
extern unsigned int nStakeTargetSpacing;
extern const unsigned int nMaxClockDrift;
extern int nCoinbaseMaturity; // FIXME make unsigned int
//extern const unsigned int nMinTxOutAmount; // set to = CTransaction::nMinTxFee;

#define CUTOFF_POS_BLOCK nCutoff_Pos_Block
extern const int CUTOFF_POS_BLOCK;

#define FEATURE_MONEYSUPPLY
#define FEATURE_CFG_MAXFUTURE

#define BRAND "cleanwatercoin"
#define BRAND_upper "CleanWaterCoin"
#define BRAND_lower "cleanwatercoin"
#define BRAND_domain "givecoin.org"
#define BRAND_CODE "WATER"

#define PPCOINSTAKE
#define PPCOINSTAKE_DEBUG

/* cleanwatercoin uses scrypt directly, not just for the POWhash */
#define LITECOIN_SCRYPT_POWHASH

#endif
