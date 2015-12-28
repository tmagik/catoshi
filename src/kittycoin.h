// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_kittycoin_H
#define CODECOIN_kittycoin_H

static const int RPC_PORT = 9932;
static const int RPC_PORT_TESTNET = 19932;
static const int P2P_PORT = 9933;
static const int P2P_PORT_TESTNET = 19933;

static const int64_t COIN = 100000000;
static const int64_t CENT = 1000000;
static const int COIN_DECIMALS = 8; /* decimal places for coin */
#define COIN_DECIMALS_FMT "08"

static const unsigned int MAX_BLOCK_SIZE = 1000000;                      // 1000KB block hard limit
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/4;         // 250KB  block soft limit

static const unsigned int nMaxClockDrift = 15*60;

/** Dust Soft Limit, allowed with additional fee per output */
static const int64_t DUST_SOFT_LIMIT = 100000; // 0.001 CAT
/** Dust Hard Limit, ignored as wallet inputs (mininput default) */
static const int64_t DUST_HARD_LIMIT = 1000;   // 0.00001 CAT mininput
/** No amount larger than this (in satoshi) is valid */
static const int64_t MAX_MONEY = 21000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

/** main.h CTransaction:AllowFree **/
static const int MIN_FREE_PRIORITY = COIN * 576 / 250;

/** The interval over which we look to calculate the next difficulty **/
static const int RETARGET_INTERVAL = 36;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

/** Minimum block time spacing (hard limit) **/
static const int64_t MINIMUM_BLOCK_SPACING = 60;	// Absolute minimum spacing

#define BRAND "CatCoin"
#define BRAND_upper "CatCoin"
#define BRAND_lower "catcoin"
#define LITECOIN_SCRYPT_POWHASH

#endif
