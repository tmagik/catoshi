// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin Core developers
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_UNDO_H
#define CODECOIN_UNDO_H

#include "compressor.h" 
#include "primitives/transaction.h"
#include "serialize.h"

/** Undo information for a CTxIn
 *
 *  Contains the prevout's CTxOut being spent, and if this was the
 *  last output of the affected transaction, its metadata as well
 *  (coinbase or not, height, transaction version)
 */
class CTxInUndo
{
public:
    CTxOut txout;         // the txout data before being spent
    bool fCoinBase;       // if the outpoint was the last unspent: whether it belonged to a coinbase
    unsigned int nHeight; // if the outpoint was the last unspent: its height
    int nVersion;         // if the outpoint was the last unspent: its version

    CTxInUndo() : txout(), fCoinBase(false), nHeight(0), nVersion(0) {}
    CTxInUndo(const CTxOut &txoutIn, bool fCoinBaseIn = false, unsigned int nHeightIn = 0, int nVersionIn = 0) : txout(txoutIn), fCoinBase(fCoinBaseIn), nHeight(nHeightIn), nVersion(nVersionIn) { }

    template<typename Stream>
    void Serialize(Stream &s) const {
        ::Serialize(s, VARINT(nHeight*2+(fCoinBase ? 1 : 0)));
        if (nHeight > 0)
            ::Serialize(s, VARINT(this->nVersion));
        ::Serialize(s, CTxOutCompressor(REF(txout)));
    }

    template<typename Stream>
    void Unserialize(Stream &s) {
        unsigned int nCode = 0;
        ::Unserialize(s, VARINT(nCode));
        nHeight = nCode / 2;
        fCoinBase = nCode & 1;
        if (nHeight > 0)
            ::Unserialize(s, VARINT(this->nVersion));
        ::Unserialize(s, REF(CTxOutCompressor(REF(txout))));
    }
};

class TxInUndoStake : public TxInUndo
{
public:
	bool fCoinStake;      // ppcoin: if the outpoint was the last unspent: whether it belonged to a coinstake
	unsigned int nTime;   // ppcoin: if the outpoint was the last unspent: its tx timestamp

	TxInUndoStake() : TxInUndo(), fCoinStake(false), nTime(0) {}
	TxInUndoStake(const CTxOut &txoutIn, bool fCoinBaseIn = false, unsigned int nHeightIn = 0, int nVersionIn = 0, bool fCoinStakeIn = false, unsigned int nTimeIn = 0) : TxInUndo(txoutIn, fCoinBaseIn, nHeightIn, nVersionIn), fCoinStake(fCoinStakeIn), nTime(nTimeIn) { }

	unsigned int GetSerializeSize(int nType, int nVersion) const {
		return ::GetSerializeSize(VARINT(nHeight*4+(fCoinBase ? 1 : 0)+(fCoinStake ? 2 : 0)), nType, nVersion) +
		::GetSerializeSize(VARINT(nTime), nType, nVersion) +
			   (nHeight > 0 ? ::GetSerializeSize(VARINT(this->nVersion), nType, nVersion) : 0) +
			   ::GetSerializeSize(CTxOutCompressor(REF(txout)), nType, nVersion);
	}

	template<typename Stream>
	void Serialize(Stream &s, int nType, int nVersion) const {
		::Serialize(s, VARINT(nHeight*4+(fCoinBase ? 1 : 0) +(fCoinStake ? 2 : 0)), nType, nVersion);
        ::Serialize(s, VARINT(nTime), nType, nVersion);
		if (nHeight > 0)
			::Serialize(s, VARINT(this->nVersion), nType, nVersion);
		::Serialize(s, CTxOutCompressor(REF(txout)), nType, nVersion);
	}

	template<typename Stream>
	void Unserialize(Stream &s, int nType, int nVersion) {
		unsigned int nCode = 0;
		::Unserialize(s, VARINT(nCode), nType, nVersion);
		nHeight = nCode / 4;
		fCoinBase = nCode & 1;
		fCoinStake = nCode & 2;
		::Unserialize(s, VARINT(nTime), nType, nVersion);
		if (nHeight > 0)
			::Unserialize(s, VARINT(this->nVersion), nType, nVersion);
		::Unserialize(s, REF(CTxOutCompressor(REF(txout))), nType, nVersion);
	}
};

#if defined(PPCOINSTAKE) // for compatibility with bitcoin-core legacy code
typedef TxInUndoStake CTxInUndo;
#else
typedef TxInUndo CTxInUndo;
#endif

/** Undo information for a CTransaction */
class CTxUndo
{
public:
    // undo information for all txins
    std::vector<CTxInUndo> vprevout;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(vprevout);
    }
};

/** Undo information for a CBlock */
class CBlockUndo
{
public:
    std::vector<CTxUndo> vtxundo; // for all but the coinbase

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(vtxundo);
    }
};

/* Prototypes for undo.cpp */
/** Undo information for a CBlock */
/* Catoshi sez: We had a perfectly good CBlockUndo:: object, why did we throw it away? */

class CDiskBlockPos;
class uint256;

#include "protocol.h"

bool UndoWriteToDisk(const CBlockUndo& blockundo, CDiskBlockPos& pos, const uint256& hashBlock, const CMessageHeader::MessageStartChars& messageStart);

bool UndoReadFromDisk(CBlockUndo& blockundo, const CDiskBlockPos& pos, const uint256& hashBlock);


#endif // BITCOIN_UNDO_H
