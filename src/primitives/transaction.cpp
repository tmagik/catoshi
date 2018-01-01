// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include "primitives/transaction.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

std::string COutPoint::ToString() const
{
    return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10), n);
}

TxIn::TxIn(COutPoint prevoutIn, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

TxIn::TxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

std::string TxIn::ToString() const
{
    std::string str;
    str += "TxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
        str += strprintf(", coinbase %s", HexStr(scriptSig));
    else
        str += strprintf(", scriptSig=%s", HexStr(scriptSig).substr(0, 24));
    if (nSequence != SEQUENCE_FINAL)
        str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
}

TxOut::TxOut(const CAmount& nValueIn, CScript scriptPubKeyIn)
{
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
}

std::string TxOut::ToString() const
{
    return strprintf("TxOut(nValue=%d.%08d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, HexStr(scriptPubKey).substr(0, 30));
}

MutableTransaction::MutableTransaction() : nVersion(Transaction::CURRENT_VERSION), nLockTime(0) {}
MutableTransaction::MutableTransaction(const Transaction& tx) : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime) {}

uint256 MutableTransaction::GetHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 Transaction::ComputeHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 Transaction::GetWitnessHash() const
{
    if (!HasWitness()) {
        return GetHash();
    }
    return SerializeHash(*this, SER_GETHASH, 0);
}

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
Transaction::Transaction() : nVersion(Transaction::CURRENT_VERSION), vin(), vout(), nLockTime(0), hash() {}
Transaction::Transaction(const MutableTransaction &tx) : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime), hash(ComputeHash()) {}
Transaction::Transaction(MutableTransaction &&tx) : nVersion(tx.nVersion), vin(std::move(tx.vin)), vout(std::move(tx.vout)), nLockTime(tx.nLockTime), hash(ComputeHash()) {}

#if defined(BRAND_grantcoin)
TransactionGRT::TransactionGRT() : Transaction(), nTime(0) {}

TransactionGRT::TransactionGRT(const MutableTransactionGRT &tx) : 
	Transaction((MutableTransaction)tx, /* Update = */ false), nTime(tx.nTime) {
    UpdateHash();
}

MutableTransactionGRT::MutableTransactionGRT() : MutableTransaction(), nTime(0) {}
MutableTransactionGRT::MutableTransactionGRT(const TransactionGRT& tx) : 
		MutableTransaction((MutableTransaction)tx), nTime(tx.nTime) {}

/* Same as Transaction::UpdateHash() above, but different types.
 * This seems like a good job for a template */
void TransactionGRT::UpdateHash() const
{
    *const_cast<uint256*>(&hash) = SerializeHash(*this);
}

/* PPCoin/Grantcoin destroy the 0.01 COIN per kb fee to reduce money supply,
 * which does not quite line up with Bitcoin, so replicate the PPCoin code here */
/* FIXME: nBytes is currently not used */
int64_t TransactionGRT::GetMinFee(unsigned int nBlockSize, bool fAllowFree,
				enum GetMinFee_mode mode, unsigned int optnBytes) const
{
    // Base fee is either nMinTxFee or nMinRelayTxFee
    int64_t nBaseFee = (mode == GMF_RELAY) ? nMinRelayTxFee : nMinTxFee;

// here be dragons. put on fireproof regression tests before taunting this code
#if defined(BRAND_cleanwatercoin)
    unsigned int nBytes = optnBytes;
#elif defined(BRAND_kittycoin)
    unsigned int nBytes = ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
#else
#warning "hack for coincontroldialog.cpp that needs nBytes"
    unsigned int nBytes = std::max(optnBytes,
                ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION));
#endif
    unsigned int nNewBlockSize = nBlockSize + nBytes;
    int64_t nMinFee = (1 + (int64_t)nBytes / 1000) * nBaseFee;

#if defined(FEATURE_ALLOW_FREE_TX) /* likely safe to deprecate this */
    if (fAllowFree)
    {
        // There is a free transaction area in blocks created by most miners,
        // * If we are relaying we allow transactions up to DEFAULT_BLOCK_PRIORITY_SIZE - 1000
        //   to be considered to fall into this category. We don't want to encourage sending
        //   multiple transactions instead of one big transaction to avoid fees.
        // * If we are creating a transaction we allow transactions up to 1,000 bytes
        //   to be considered safe and assume they can likely make it into this section.
        if (nBytes < (mode == GMF_SEND ? 1000 : (DEFAULT_BLOCK_PRIORITY_SIZE - 1000)))
            nMinFee = 0;
    }
#endif

    // This code can be removed after enough miners have upgraded to version 0.9.
    // Until then, be safe when sending and require a fee if any output
    // is less than CENT:
    if (nMinFee < nBaseFee && mode == GMF_SEND)
    {
        for (const CTxOut& txout: vout)
            if (txout.nValue < CENT)
                nMinFee = nBaseFee;
    }

    // Raise the price as the block approaches full
    if (nBlockSize != 1 && nNewBlockSize >= MAX_BLOCK_SIZE_GEN/2)
    {
        if (nNewBlockSize >= MAX_BLOCK_SIZE_GEN)
            return MAX_MONEY;
        nMinFee *= MAX_BLOCK_SIZE_GEN / (MAX_BLOCK_SIZE_GEN - nNewBlockSize);
    }

    if (!MoneyRange(nMinFee))
        nMinFee = MAX_MONEY;
    return nMinFee;
}
#endif /* BRAND_grantcoin */

CAmount Transaction::GetValueOut() const
{
    CAmount nValueOut = 0;
    for (const auto& tx_out : vout) {
        nValueOut += tx_out.nValue;
        if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nValueOut;
}

{
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

std::string Transaction::ToString() const
{
    std::string str;
    str += strprintf("Transaction(hash=%s, ver=%d, vin.size=%u, vout.size=%u, nLockTime=%u)\n",
        GetHash().ToString().substr(0,10),
        nVersion,
        vin.size(),
        vout.size(),
        nLockTime);
    for (const auto& tx_in : vin)
        str += "    " + tx_in.ToString() + "\n";
    for (const auto& tx_in : vin)
        str += "    " + tx_in.scriptWitness.ToString() + "\n";
    for (const auto& tx_out : vout)
        str += "    " + tx_out.ToString() + "\n";
    return str;
}
