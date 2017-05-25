// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_PRIMITIVES_TRANSACTION_H
#define CODECOIN_PRIMITIVES_TRANSACTION_H

#include "codecoin.h"
#include "amount.h"
#include "script/script.h"
#include "serialize.h"
#include "uintBIG.h"

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    uint256 hash;
    uint32_t n;

    COutPoint() { SetNull(); }
    COutPoint(uint256 hashIn, uint32_t nIn) { hash = hashIn; n = nIn; }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(hash);
        READWRITE(n);
    }

    void SetNull() { hash.SetNull(); n = (uint32_t) -1; }
    bool IsNull() const { return (hash.IsNull() && n == (uint32_t) -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash < b.hash || (a.hash == b.hash && a.n < b.n));
    }

    friend bool operator==(const COutPoint& a, const COutPoint& b)
    {
        return (a.hash == b.hash && a.n == b.n);
    }

    friend bool operator!=(const COutPoint& a, const COutPoint& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class TxIn
{
public:
    COutPoint prevout;
    CScript scriptSig;
    uint32_t nSequence;

    /* Setting nSequence to this value for every input in a transaction
     * disables nLockTime. */
    static const uint32_t SEQUENCE_FINAL = 0xffffffff;

    /* Below flags apply in the context of BIP 68*/
    /* If this flag set, CTxIn::nSequence is NOT interpreted as a
     * relative lock-time. */
    static const uint32_t SEQUENCE_LOCKTIME_DISABLE_FLAG = (1 << 31);

    /* If CTxIn::nSequence encodes a relative lock-time and this flag
     * is set, the relative lock-time has units of 512 seconds,
     * otherwise it specifies blocks with a granularity of 1. */
    static const uint32_t SEQUENCE_LOCKTIME_TYPE_FLAG = (1 << 22);

    /* If CTxIn::nSequence encodes a relative lock-time, this mask is
     * applied to extract that lock-time from the sequence field. */
    static const uint32_t SEQUENCE_LOCKTIME_MASK = 0x0000ffff;

    /* In order to use the same number of bits to encode roughly the
     * same wall-clock duration, and because blocks are naturally
     * limited to occur every 600s on average, the minimum granularity
     * for time-based relative lock-time is fixed at 512 seconds.
     * Converting from CTxIn::nSequence to seconds is performed by
     * multiplying by 512 = 2^9, or equivalently shifting up by
     * 9 bits. */
    static const int SEQUENCE_LOCKTIME_GRANULARITY = 9;

    TxIn()
    {
        nSequence = std::numeric_limits<unsigned int>::max();
    }

    explicit TxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);
    TxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(prevout);
        READWRITE(*(CScriptBase*)(&scriptSig));
        READWRITE(nSequence);
    }

    friend bool operator==(const TxIn& a, const TxIn& b)
    {
        return (a.prevout   == b.prevout &&
                a.scriptSig == b.scriptSig &&
                a.nSequence == b.nSequence);
    }

    friend bool operator!=(const TxIn& a, const TxIn& b)
    {
        return !(a == b);
    }

    std::string ToString() const;
};

/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class TxOut
{
public:
    CAmount nValue;
    CScript scriptPubKey;

    TxOut()
    {
        SetNull();
    }

    TxOut(const CAmount& nValueIn, CScript scriptPubKeyIn);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(nValue);
        READWRITE(*(CScriptBase*)(&scriptPubKey));
    }

    void SetNull()
    {
        nValue = -1;
        scriptPubKey.clear();
    }

    bool IsNull() const
    {
        return (nValue == -1);
    }

    uint256 GetHash() const;

    CAmount GetDustThreshold(const CFeeRate &minRelayTxFee) const
    {
        // "Dust" is defined in terms of CTransaction::minRelayTxFee,
        // which has units satoshis-per-kilobyte.
        // If you'd pay more than 1/3 in fees
        // to spend something, then we consider it dust.
        // A typical spendable txout is 34 bytes big, and will
        // need a CTxIn of at least 148 bytes to spend:
        // so dust is a spendable txout less than
        // 546*minRelayTxFee/1000 (in satoshis)
        if (scriptPubKey.IsUnspendable())
            return 0;

        size_t nSize = GetSerializeSize(SER_DISK,0)+148u;
        return 3*minRelayTxFee.GetFee(nSize);
    }

    bool IsDust(const CFeeRate &minRelayTxFee) const
    {
        return (nValue < GetDustThreshold(minRelayTxFee));
    }

    friend bool operator==(const TxOut& a, const TxOut& b)
    {
        return (a.nValue       == b.nValue &&
                a.scriptPubKey == b.scriptPubKey);
    }

    friend bool operator!=(const TxOut& a, const TxOut& b)
    {
        return !(a == b);
    }

    std::string ToString() const;

#if defined BRAND_grantcoin
	bool IsEmpty() const
	{
		return (nValue == 0 && scriptPubKey.empty());
	}
#endif
};

/* Codecoin TODO: combine below with fancy C++ features (templates & 
 * https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error )
 * to eliminate duplicate code
 */

class MutableTransaction;

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
class Transaction
{
/** Memory only */
private: /* only makes sense for this class */
    void UpdateHash() const;
protected: /* derived classes can implement UpdateHash */
    const uint256 hash;

public:
    // Default transaction version.
    static const int32_t CURRENT_VERSION=1;

    // Changing the default transaction version requires a two step process: first
    // adapting relay policy by bumping MAX_STANDARD_VERSION, and then later date
    // bumping the default CURRENT_VERSION at which point both CURRENT_VERSION and
    // MAX_STANDARD_VERSION will be equal.
    static const int32_t MAX_STANDARD_VERSION=2;

    // The local variables are made const to prevent unintended modification
    // without updating the cached hash value. However, CTransaction is not
    // actually immutable; deserialization and assignment are implemented,
    // and bypass the constness. This is safe, as they update the entire
    // structure, including the hash.
    const int32_t nVersion;
    const std::vector<TxIn> vin;
    const std::vector<TxOut> vout;
    const uint32_t nLockTime;

    /** Construct a CTransaction that qualifies as IsNull() */
    Transaction();

    /** Convert a CMutableTransaction into a CTransaction. */
    /** Optional argument allows derived classes to construct
        a base class with no hash update, since the child does
        it again */
    Transaction(const MutableTransaction &tx, bool Update=true);

    Transaction& operator=(const Transaction& tx);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*const_cast<int32_t*>(&this->nVersion));
        nVersion = this->nVersion;
        READWRITE(*const_cast<std::vector<TxIn>*>(&vin));
        READWRITE(*const_cast<std::vector<TxOut>*>(&vout));
        READWRITE(*const_cast<uint32_t*>(&nLockTime));
        if (ser_action.ForRead())
            UpdateHash();
    }

    bool IsNull() const {
        return vin.empty() && vout.empty();
    }

    const uint256& GetHash() const {
        return hash;
    }

    // Return sum of txouts.
    CAmount GetValueOut() const;
    // GetValueIn() is a method on CCoinsViewCache, because
    // inputs must be known to compute value in.

    // Compute priority, given priority of inputs and (optionally) tx size
    double ComputePriority(double dPriorityInputs, unsigned int nTxSize=0) const;

    // Compute modified tx size for priority calculation (optionally given tx size)
    unsigned int CalculateModifiedSize(unsigned int nTxSize=0) const;

    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull());
    }

    friend bool operator==(const Transaction& a, const Transaction& b)
    {
        return a.hash == b.hash;
    }

    friend bool operator!=(const Transaction& a, const Transaction& b)
    {
        return a.hash != b.hash;
    }

    std::string ToString() const;
};

class MutableTransactionGRT;

class TransactionGRT : public Transaction
{
private:
	void UpdateHash() const;
	
	enum GetMinFee_mode
	{
		GMF_BLOCK,
		GMF_RELAY,
		GMF_SEND,
	};

public:
/** TODO: this goes into src/policy/fees.cpp when latest bitcoin code is merged */
/** Fees smaller than this (in satoshi) are considered zero fee (for transaction creation) */
	const static int64_t nMinTxFee = CENT;
/** Fees smaller than this (in satoshi) are considered zero fee (for relaying) */
	const static int64_t nMinRelayTxFee = CENT;

    uint32_t nTime; // FIXME just make this 'Time'  

    /** Construct a CTransaction that qualifies as IsNull() */
    TransactionGRT();

    /** Convert a CMutableTransaction into a CTransaction. */
    TransactionGRT(const MutableTransactionGRT &tx);

    /* must be replicated or serialization is pulled from base object */
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*const_cast<int32_t*>(&this->nVersion));
        nVersion = this->nVersion;
        READWRITE(*const_cast<uint32_t*>(&this->nTime));
        READWRITE(*const_cast<std::vector<TxIn>*>(&vin));
        READWRITE(*const_cast<std::vector<TxOut>*>(&vout));
        READWRITE(*const_cast<uint32_t*>(&nLockTime));
        if (ser_action.ForRead())
            UpdateHash();
    }

	// Apply the effects of this transaction on the UTXO set represented by view
	//void UpdateCoins(const CTransaction& tx, CValidationState &state, CCoinsViewCache &inputs, CTxUndo &txundo, int nHeight, const uint256 &txhash);

	int64_t GetMinFee(unsigned int nBlockSize=1, bool fAllowFree=false, enum GetMinFee_mode mode=GMF_BLOCK, unsigned int nBytes=0) const;

    /* What stake (ppcoin derivatives, + grantcoin do */
    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull() && vout.size() >= 1);
    }


};

/** A mutable version of CTransaction. */
/* let codecoin.h/BITCOIN_COMPAT defines instantiate the right class name */
class MutableTransaction
{
public:
    int32_t nVersion;
#if 0 && defined(PPCOINSTAKE) || defined(BRAND_grantcoin)
    uint32_t nTime;
#endif
    std::vector<TxIn> vin;
    std::vector<TxOut> vout;
    uint32_t nLockTime;

    MutableTransaction();
    MutableTransaction(const Transaction& tx);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
#if 0 && defined(PPCOINSTAKE) || defined(BRAND_grantcoin)
        READWRITE(nTime);
#endif
        READWRITE(vin);
        READWRITE(vout);
        READWRITE(nLockTime);
    }

    /** Compute the hash of this CMutableTransaction. This is computed on the
     * fly, as opposed to GetHash() in CTransaction, which uses a cached result.
     */
    uint256 GetHash() const;
};

class MutableTransactionGRT: public MutableTransaction
{
public:
	uint32_t nTime; // FIXME: evaluate if this should be signed or unsigned

	MutableTransactionGRT();
	MutableTransactionGRT(const TransactionGRT& tx);

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(this->nVersion);
		nVersion = this->nVersion;
		READWRITE(nTime);
		READWRITE(vin);
		READWRITE(vout);
		READWRITE(nLockTime);
	}

	uint256 GetHash() const;
};

#if defined(BITCOIN_COMPAT)
typedef TxIn CTxIn;
typedef TxOut CTxOut;
#endif

#endif // CODECOIN_PRIMITIVES_TRANSACTION_H
