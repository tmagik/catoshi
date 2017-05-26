// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_bitcoin_H
#define CODECOIN_bitcoin_H

/* ========= Things defined in bitcoin@0.12/consensus/consensus.h  ========= */

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE = 1000000;
/** The maximum allowed number of signature check operations in a block (network rule) */
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

/** Flags for nSequence and nLockTime locks */
enum {
    /* Interpret sequence numbers as relative lock-time constraints. */
    LOCKTIME_VERIFY_SEQUENCE = (1 << 0),

    /* Use GetMedianTimePast() instead of nTime for end point timestamp. */
    LOCKTIME_MEDIAN_TIME_PAST = (1 << 1),
};

/* ========= End defines from bitcoin@0.12/consensus/consensus.h  ========= */

static const int RPC_PORT = 8332;
static const int RPC_PORT_TESTNET = 18332;

#include<cstdint>

// TODO: objectize this. as of 0.10.x is in amount.h
static const int64_t COIN = 100000000;
static const int64_t CENT = 1000000;

/** No amount larger than this (in satoshi) is valid */
static const int64_t MAX_MONEY = 21000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

#define BRAND "Bitcoin"
#define BRAND_upper "Bitcoin"
#define BRAND_lower "bitcoin"
#define BRAND_CODE "BTC"
#define BRAND_file __FILE__
#define BITCOIN_COMPAT

/* allow for overloads.. */
#define CTransaction Transaction
#define CMutableTransaction MutableTransaction

#endif
