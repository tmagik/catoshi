// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
        str += strprintf(", scriptSig=%s", scriptSig.ToString().substr(0,24));
    if (nSequence != std::numeric_limits<unsigned int>::max())
        str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
}

TxOut::TxOut(const CAmount& nValueIn, CScript scriptPubKeyIn)
{
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
}

uint256 TxOut::GetHash() const
{
    return SerializeHash(*this);
}

std::string TxOut::ToString() const
{
    return strprintf("TxOut(nValue=%d.%08d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, scriptPubKey.ToString().substr(0,30));
}

MutableTransaction::MutableTransaction() : nVersion(Transaction::CURRENT_VERSION), nLockTime(0) {}
MutableTransaction::MutableTransaction(const Transaction& tx) : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime) {}

uint256 CMutableTransaction::GetHash() const
{
    return SerializeHash(*this);
}

void Transaction::UpdateHash() const
{
    *const_cast<uint256*>(&hash) = SerializeHash(*this);
}

Transaction::Transaction() : hash(0), nVersion(Transaction::CURRENT_VERSION), vin(), vout(), nLockTime(0) { }

Transaction::Transaction(const MutableTransaction &tx) : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime) {
    UpdateHash();
}

#if defined(BRAND_grantcoin)
TransactionGRT::TransactionGRT() : Transaction(), nTime(0) {}

TransactionGRT::TransactionGRT(const MutableTransactionGRT &tx) : Transaction((MutableTransaction)tx), nTime(tx.nTime) {
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

#if 0
/* Bitcoin/litecoin used int*, why is this not int32_t/uint32_t like the header? */
TransactionGRT& TransactionGRT::operator=(const TransactionGRT &tx) {
    *const_cast<int32_t*>(&nVersion) = tx.nVersion;
    *const_cast<uint32_t*>(&nTime) = tx.nTime;
    *const_cast<std::vector<TxIn>*>(&vin) = tx.vin;
    *const_cast<std::vector<TxOut>*>(&vout) = tx.vout;
    *const_cast<uint32_t*>(&nLockTime) = tx.nLockTime;
    *const_cast<uint256*>(&hash) = tx.hash;
    return *this;
}a
#endif
#endif


Transaction& Transaction::operator=(const Transaction &tx) {
    *const_cast<int32_t*>(&nVersion) = tx.nVersion;
    *const_cast<std::vector<TxIn>*>(&vin) = tx.vin;
    *const_cast<std::vector<TxOut>*>(&vout) = tx.vout;
    *const_cast<uint32_t*>(&nLockTime) = tx.nLockTime;
    *const_cast<uint256*>(&hash) = tx.hash;
    return *this;
}

CAmount Transaction::GetValueOut() const
{
    CAmount nValueOut = 0;
    for (std::vector<TxOut>::const_iterator it(vout.begin()); it != vout.end(); ++it)
    {
        nValueOut += it->nValue;
        if (!MoneyRange(it->nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error("Transaction::GetValueOut() : value out of range");
    }
    return nValueOut;
}

double Transaction::ComputePriority(double dPriorityInputs, unsigned int nTxSize) const
{
    nTxSize = CalculateModifiedSize(nTxSize);
    if (nTxSize == 0) return 0.0;

    return dPriorityInputs / nTxSize;
}

unsigned int Transaction::CalculateModifiedSize(unsigned int nTxSize) const
{
    // In order to avoid disincentivizing cleaning up the UTXO set we don't count
    // the constant overhead for each txin and up to 110 bytes of scriptSig (which
    // is enough to cover a compressed pubkey p2sh redemption) for priority.
    // Providing any more cleanup incentive than making additional inputs free would
    // risk encouraging people to create junk outputs to redeem later.
    if (nTxSize == 0)
        nTxSize = ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
    for (std::vector<TxIn>::const_iterator it(vin.begin()); it != vin.end(); ++it)
    {
        unsigned int offset = 41U + std::min(110U, (unsigned int)it->scriptSig.size());
        if (nTxSize > offset)
            nTxSize -= offset;
    }
    return nTxSize;
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
    for (unsigned int i = 0; i < vin.size(); i++)
        str += "    " + vin[i].ToString() + "\n";
    for (unsigned int i = 0; i < vout.size(); i++)
        str += "    " + vout[i].ToString() + "\n";
    return str;
}
