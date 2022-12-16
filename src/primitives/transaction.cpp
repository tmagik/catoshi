// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include <primitives/transaction.h>

#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>

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

MutableSegwitTx::MutableSegwitTx() : MutableTransaction() {}
MutableSegwitTx::MutableSegwitTx(const SegwitTx& tx) : MutableTransaction(tx) {}

uint256 MutableTransaction::GetHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 Transaction::ComputeHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 SegwitTx::GetWitnessHash() const
{
    if (!HasWitness()) {
        return GetHash();
    }
    return SerializeHash(*this, SER_GETHASH, 0);
}

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
/* TODO: init the hash when we actually ask for it */
/* TODO: make templates of this boilerplate */
Transaction::Transaction()
	: vin(), vout(), nLockTime(0),
	nVersion(Transaction::CURRENT_VERSION), hash() {}
Transaction::Transaction(const MutableTransaction &tx, bool inithash) 
	: vin(tx.vin), vout(tx.vout), nLockTime(tx.nLockTime),
	nVersion(tx.nVersion) {
	if (inithash){ 
#warning "this is probably evil and undefined"
		uint256 *phash = const_cast <uint256*> (&hash);
		*phash = ComputeHash();
	}
}
Transaction::Transaction(MutableTransaction &&tx, bool inithash) 
	: vin(std::move(tx.vin)), vout(std::move(tx.vout)), nLockTime(tx.nLockTime), 
	nVersion(tx.nVersion) {
	if (inithash) {
		uint256 *phash = const_cast <uint256*> (&hash);
		*phash = ComputeHash();
	}
}

/* Segwit transactions keep the same hash as legacy??? */
/* Revisit this: can we do interesting things with object models here */
SegwitTx::SegwitTx() : Transaction() {}
SegwitTx::SegwitTx(const MutableSegwitTx &tx) : Transaction(tx, true) {}
SegwitTx::SegwitTx(MutableSegwitTx &&tx) : Transaction(tx, true) {}

#if defined(BRAND_grantcoin)
MutableStakeTx::MutableStakeTx(): MutableTransaction(), Time(0) {}
MutableStakeTx::MutableStakeTx(const StakeTx & tx): MutableTransaction(tx), Time(tx.Time) {}

StakeTx::StakeTx() : Transaction(), Time(0) {}

/* TODO: is this boilerplate really necessary to duplicate? */
uint256 StakeTx::ComputeHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

StakeTx::StakeTx(const MutableStakeTx &tx)
	: Transaction(tx, false), Time(tx.Time){
	uint256 *phash = const_cast <uint256*> (&hash);
	*phash = ComputeHash();
	//assert (hash == tx.hash);  /* extra sanity check, TODO remove later? */
}
StakeTx::StakeTx(MutableStakeTx &&tx)
	: Transaction(tx, false), Time(tx.Time){
	uint256 *phash = const_cast <uint256*> (&hash);
	*phash = ComputeHash();
	//assert (hash == tx.hash);  /* extra sanity check, TODO remove later? */
}

/* PPCoin/Grantcoin destroy the 0.01 COIN per kb fee to reduce money supply,
 * which does not quite line up with Bitcoin, so replicate the PPCoin code here */
/* FIXME: nBytes is currently not used */
/* FIXME: figure if this is needed here or not */

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

unsigned int Transaction::GetTotalSize() const
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
