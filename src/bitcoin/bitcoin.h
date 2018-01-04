// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_bitcoin_H
#define CODECOIN_bitcoin_H

/* ========= Things defined in bitcoin@0.14/consensus/consensus.h  ========= */

#include "bitcoin/consensus.h"

/* ========= End defines from bitcoin@0.14/consensus/consensus.h  ========= */

static const int RPC_PORT = 8332;
static const int RPC_PORT_TESTNET = 18332;

/* from net.h */
/** Maximum length of incoming protocol messages (no message over 4 MB is currently acceptable). */
static const unsigned int MAX_PROTOCOL_MESSAGE_LENGTH = 4 * 1000 * 1000;

#include <cstdint>

// TODO: objectize this. as of 0.10.x is in amount.h
static const int64_t COIN = 100000000;
static const int64_t CENT = 1000000;

/** Default for -minrelaytxfee, minimum relay fee for transactions */
static const unsigned int DEFAULT_MIN_RELAY_TX_FEE = 1000;

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
