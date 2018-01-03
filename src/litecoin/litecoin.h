// This file is part of the Catoshi Codecoin financial cryptography software
// Copyright (c) 2014-2018 7 Elements LLC
// Distributed under the Affero GNU General public license version 3
// see the file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_litecoin_H
#define CODECOIN_litecoin_H

#include "litecoin/consensus.h"

static const int RPC_PORT = 9332;
static const int RPC_PORT_TESTNET = 19332;
static const int P2P_PORT = 9333;
static const int P2P_PORT_TESTNET = 19333;

/* from net.h */
/** Maximum length of incoming protocol messages (no message over 4 MB is currently acceptable). */
static const unsigned int MAX_PROTOCOL_MESSAGE_LENGTH = 4 * 1000 * 1000;

#include <cstdint>

// TODO: objectize this. as of 0.10.x is in amount.h
static const int64_t COIN = 100000000;
static const int64_t CENT = 1000000;

/** Default for -minrelaytxfee, minimum relay fee for transactions */
static const unsigned int DEFAULT_MIN_RELAY_TX_FEE = 100000;

/** No amount larger than this (in satoshi) is valid */
static const int64_t MAX_MONEY = 84000000 * COIN;
inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

#define BRAND "Litecoin"
#define BRAND_upper "Litecoin"
#define BRAND_lower "litecoin"
#define BRAND_CODE "LTC"
#define BRAND_file __FILE__
#define LITECOIN_SCRYPT_POWHASH
#define BITCOIN_COMPAT

/* allow for overloads.. */
#define CTransaction Transaction
#define CMutableTransaction MutableTransaction

#endif
