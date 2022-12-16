// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2017 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_PRIMITIVES_TRANSACTION_H
#define CODECOIN_PRIMITIVES_TRANSACTION_H

#include <codecoin.h>
#include <stdint.h>
#include <amount.h>
#include <script/script.h>
#include <serialize.h>
#include <uintBIG.h>

static const int SERIALIZE_TRANSACTION_NO_WITNESS = 0x40000000;

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class COutPoint
{
public:
    uint256 hash;
    uint32_t n;

    COutPoint(): n((uint32_t) -1) { }
    COutPoint(const uint256& hashIn, uint32_t nIn): hash(hashIn), n(nIn) { }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(hash);
        READWRITE(n);
    }

    void SetNull() { hash.SetNull(); n = (uint32_t) -1; }
    bool IsNull() const { return (hash.IsNull() && n == (uint32_t) -1); }

    friend bool operator<(const COutPoint& a, const COutPoint& b)
    {
        int cmp = a.hash.Compare(b.hash);
        return cmp < 0 || (cmp == 0 && a.n < b.n);
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
    CScriptWitness scriptWitness; //! Only serialized through CTransaction

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
        nSequence = SEQUENCE_FINAL;
    }

    explicit TxIn(COutPoint prevoutIn, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);
    TxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn=CScript(), uint32_t nSequenceIn=SEQUENCE_FINAL);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(prevout);
        READWRITE(scriptSig);
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
    inline void SerializationOp(Stream& s, Operation ser_action) {
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

#if defined(BRAND_grantcoin)
    // TODO move this to StakeTxOut
    void SetEmpty()
    {
        nValue = 0;
        scriptPubKey.clear();
    }

    bool IsEmpty() const
    {
        return (nValue == 0 && scriptPubKey.empty());
    }
#endif

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
};

/* Codecoin TODO: combine below with fancy C++ features (templates & 
 * https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error )
 * to eliminate duplicate code
 */


/**
 * Basic transaction serialization format:
 * - int32_t nVersion
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - uint32_t nLockTime
 */

template <typename Stream, typename TxType>
inline void UnserializeTransaction(TxType &tx, Stream &s) {
    s >> tx.nVersion;
    tx.vin.clear();
    tx.vout.clear();
    s >> tx.vin;
    s >> tx.vout;
    s >> tx.nLockTime;
}

template <typename Stream, typename TxType>
inline void SerializeTransaction(const TxType &tx, Stream &s) {
    s << tx.nVersion;
    s << tx.vin;
    s << tx.vout;
    s << tx.nLockTime;
}

/**
 * Stake transaction serialization format:
 * - int32_t nVersion
 * - uint32_t nTime
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - uint32_t nLockTime
 */

template <typename Stream, typename TxType>
inline void UnserializeStakeTx(TxType &tx, Stream &s) {
    s >> tx.nVersion;
    s >> tx.Time;
    tx.vin.clear();
    tx.vout.clear();
    s >> tx.vin;
    s >> tx.vout;
    s >> tx.nLockTime;
}

template <typename Stream, typename TxType>
inline void SerializeStakeTx(const TxType &tx, Stream &s) {
    s << tx.nVersion;
    s << tx.Time;
    s << tx.vin;
    s << tx.vout;
    s << tx.nLockTime;
}

/**
 * Extended transaction serialization format (class SegwitTx):
 * - int32_t nVersion
 * - unsigned char dummy = 0x00
 * - unsigned char flags (!= 0)
 * - std::vector<CTxIn> vin
 * - std::vector<CTxOut> vout
 * - if (flags & 1):
 *   - CTxWitness wit;
 * - uint32_t nLockTime
 */

template<typename Stream, typename TxType>
inline void UnserializeSegwitTx(TxType& tx, Stream& s) {
    const bool fAllowWitness = !(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

    s >> tx.nVersion;
    unsigned char flags = 0;
    tx.vin.clear();
    tx.vout.clear();
    /* Try to read the vin. In case the dummy is there, this will be read as an empty vector. */
    s >> tx.vin;
    if (tx.vin.size() == 0 && fAllowWitness) {
        /* We read a dummy or an empty vin. */
        s >> flags;
        if (flags != 0) {
            s >> tx.vin;
            s >> tx.vout;
        }
    } else {
        /* We read a non-empty vin. Assume a normal vout follows. */
        s >> tx.vout;
    }
    if ((flags & 1) && fAllowWitness) {
        /* The witness flag is present, and we support witnesses. */
        flags ^= 1;
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s >> tx.vin[i].scriptWitness.stack;
        }
    }
    if (flags) {
        /* Unknown flag in the serialization */
        throw std::ios_base::failure("Unknown transaction optional data");
    }
    s >> tx.nLockTime;
}

template<typename Stream, typename TxType>
inline void SerializeSegwitTx(const TxType& tx, Stream& s) {
    const bool fAllowWitness = !(s.GetVersion() & SERIALIZE_TRANSACTION_NO_WITNESS);

    s << tx.nVersion;
    unsigned char flags = 0;
    // Consistency check
    if (fAllowWitness) {
        /* Check whether witnesses need to be serialized. */
        if (tx.HasWitness()) {
            flags |= 1;
        }
    }
    if (flags) {
        /* Use extended format in case witnesses are to be serialized. */
        std::vector<TxIn> vinDummy;
        s << vinDummy;
        s << flags;
    }
    s << tx.vin;
    s << tx.vout;
    if (flags & 1) {
        for (size_t i = 0; i < tx.vin.size(); i++) {
            s << tx.vin[i].scriptWitness.stack;
        }
    }
    s << tx.nLockTime;
}

class MutableTransaction;

/** The basic transaction that is broadcasted on the network and contained in
 * blocks.  A transaction can contain multiple inputs and outputs.
 */
class Transaction
{
public:
    // Default transaction version.
#if defined(PPCOINSTAKE) || defined(BRAND_grantcoin) // hack for grantcoin/ppcoin
    // TODO: fix this with proper initializers
    static const int32_t CURRENT_VERSION=1;
    static const int32_t MAX_STANDARD_VERSION=2;

#else // bitcoin/litecoin
    static const int32_t CURRENT_VERSION=2;
    // Changing the default transaction version requires a two step process: first
    // adapting relay policy by bumping MAX_STANDARD_VERSION, and then later date
    // bumping the default CURRENT_VERSION at which point both CURRENT_VERSION and
    // MAX_STANDARD_VERSION will be equal.
    static const int32_t MAX_STANDARD_VERSION=2;
#endif
    // The local variables are made const to prevent unintended modification
    // without updating the cached hash value. However, CTransaction is not
    // actually immutable; deserialization and assignment are implemented,
    // and bypass the constness. This is safe, as they update the entire
    // structure, including the hash.
    const std::vector<TxIn> vin;
    const std::vector<TxOut> vout;
    const uint32_t nLockTime;
    const int32_t nVersion;

/* TODO figure this out later */
protected:
    /** Memory only. */
    const uint256 hash;
    //uint256 hash;

    uint256 ComputeHash() const;

public:
    /** Construct a CTransaction that qualifies as IsNull() */
    Transaction();

    /** Convert a CMutableTransaction into a CTransaction. */
    Transaction(const MutableTransaction &tx, bool inithash = true);
    Transaction(MutableTransaction &&tx, bool inithash = true);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }

    /** This deserializing constructor is provided instead of an Unserialize method.
     *  Unserialize is not possible, since it would require overwriting const fields. */
    template <typename Stream>
    Transaction(deserialize_type, Stream& s) : Transaction(MutableTransaction(deserialize, s)) {}

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

    /**
     * Get the total transaction size in bytes, including witness data.
     * "Total Size" defined in BIP141 and BIP144.
     * @return Total transaction size in bytes
     */
    unsigned int GetTotalSize() const;

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

class MutableSegwitTx;

class SegwitTx : public Transaction {
public:
    SegwitTx();

    SegwitTx(const MutableSegwitTx &tx);
    SegwitTx(MutableSegwitTx &&tx);
    
    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeSegwitTx(*this, s);
    }

    /** This deserializing constructor is provided instead of an Unserialize method.
     *  Unserialize is not possible, since it would require overwriting const fields. */
    template <typename Stream>
    SegwitTx(deserialize_type, Stream& s) : SegwitTx(MutableSegwitTx(deserialize, s)) {}

    // Compute a hash that includes both transaction and witness data
    uint256 GetWitnessHash() const;

    bool HasWitness() const
    {
	// TODO: for (TxIn &in : vin ){ 
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
};

#if defined(BRAND_grantcoin) /* FIXME update this later */
class MutableStakeTx;

enum GetMinFee_mode
{       
    GMF_BLOCK,
    GMF_RELAY,
    GMF_SEND,
};

class StakeTx : public Transaction
{
public:
    const uint32_t Time; // FIXME just make this 'Time'  

    /** Construct a CTransaction that qualifies as IsNull() */
    StakeTx();

    /** Convert a CMutableTransaction into a CTransaction. */
    StakeTx(const MutableStakeTx &tx);
    StakeTx(MutableStakeTx &&tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeStakeTx(*this, s);
    }

    /** This deserializing constructor is provided instead of an Unserialize method.
     *  Unserialize is not possible, since it would require overwriting const fields. */
    template <typename Stream>
    StakeTx(deserialize_type, Stream& s) : StakeTx(MutableStakeTx(deserialize, s)) {}

    uint256 ComputeHash() const;

    /* What stake (ppcoin derivatives, + grantcoin do */
    bool IsCoinBase() const
    {
        return (vin.size() == 1 && vin[0].prevout.IsNull() && vout.size() >= 1);
    }

    bool IsCoinStake() const
    {
        // ppcoin: the coin stake transaction is marked with the first output empty
        // givecoin: Do we want this, or something else?
        return (vin.size() > 0 && (!vin[0].prevout.IsNull()) && vout.size() >= 2 && vout[0].IsEmpty());
    }

    /* ugly compat for witness code. */
    bool HasWitness() const
    {
        return false;
    }

    /* ugly compat for witness code. */
    const uint256 GetWitnessHash() const
    {
	return GetHash();
    }
};
#endif /* BRAND_grantcoin */

/** A mutable version of CTransaction. */
/* let codecoin.h/BITCOIN_COMPAT defines instantiate the right class name */
class MutableTransaction
{
public:
    int32_t nVersion;
    std::vector<TxIn> vin;
    std::vector<TxOut> vout;
    uint32_t nLockTime;

    MutableTransaction();
    MutableTransaction(const Transaction& tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeTransaction(*this, s);
    }


    template <typename Stream>
    inline void Unserialize(Stream& s) {
        UnserializeTransaction(*this, s);
    }

    template <typename Stream>
    MutableTransaction(deserialize_type, Stream& s) {
        Unserialize(s);
    }

    /** Compute the hash of this MutableTransaction. This is computed on the
     * fly, as opposed to GetHash() in Transaction, which uses a cached result.
     */
    uint256 GetHash() const;

    friend bool operator==(const MutableTransaction& a, const MutableTransaction& b)
    {
        return a.GetHash() == b.GetHash();
    }

    bool HasWitness() const
    {
        return false;
    }
};

class MutableSegwitTx : public MutableTransaction
{
public:
    MutableSegwitTx();
    MutableSegwitTx(const Transaction& tx);
    MutableSegwitTx(const SegwitTx& tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeSegwitTx(*this, s);
    }


    template <typename Stream>
    inline void Unserialize(Stream& s) {
        UnserializeSegwitTx(*this, s);
    }

    template <typename Stream>
    MutableSegwitTx(deserialize_type, Stream& s) {
        Unserialize(s);
    }

    bool HasWitness() const
    {
        for (size_t i = 0; i < vin.size(); i++) {
            if (!vin[i].scriptWitness.IsNull()) {
                return true;
            }
        }
        return false;
    }
};


#if defined(BRAND_grantcoin) // FIXME later
class MutableStakeTx: public MutableTransaction
{
public:
    uint32_t Time; // FIXME: evaluate if this should be signed or unsigned

    MutableStakeTx();
    MutableStakeTx(const StakeTx& tx);

    template <typename Stream>
    inline void Serialize(Stream& s) const {
        SerializeStakeTx(*this, s);
    }

    template <typename Stream>
    inline void Unserialize(Stream& s) {
        UnserializeStakeTx(*this, s);
    }

    template <typename Stream>
    MutableStakeTx(deserialize_type, Stream& s) {
        Unserialize(s);
    }

};
#endif

typedef std::shared_ptr<const CTransaction> CTransactionRef;
static inline CTransactionRef MakeTransactionRef() { return std::make_shared<const CTransaction>(); }
template <typename Tx> static inline CTransactionRef MakeTransactionRef(Tx&& txIn) { return std::make_shared<const CTransaction>(std::forward<Tx>(txIn)); }

#if defined(BITCOIN_COMPAT)
typedef TxIn CTxIn;
typedef TxOut CTxOut;
#endif

#endif // CODECOIN_PRIMITIVES_TRANSACTION_H
