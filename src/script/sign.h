// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2017 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#ifndef CODECOIN_SCRIPT_SIGN_H
#define CODECOIN_SCRIPT_SIGN_H

#include "codecoin.h"

#include "script/interpreter.h"

class CKeyID;
class CKeyStore;
class CScript;
class CTransaction;

struct CMutableTransaction;

/** Virtual base class for signature creators. */
class BaseSignatureCreator {
protected:
    const CKeyStore* keystore;

public:
    BaseSignatureCreator(const CKeyStore* keystoreIn) : keystore(keystoreIn) {}
    const CKeyStore& KeyStore() const { return *keystore; };
    virtual ~BaseSignatureCreator() {}
    virtual const BaseSignatureChecker& Checker() const =0;

    /** Create a singular (non-script) signature. */
    virtual bool CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& keyid, const CScript& scriptCode) const =0;
};

/** A signature creator for transactions. */
class TransactionSignatureCreator : public BaseSignatureCreator {
    const CTransaction* txTo;
    unsigned int nIn;
    int nHashType;
    const TransactionSignatureChecker checker;

public:
    TransactionSignatureCreator(const CKeyStore* keystoreIn, const CTransaction* txToIn, unsigned int nInIn, int nHashTypeIn=SIGHASH_ALL);
    const BaseSignatureChecker& Checker() const { return checker; }
    bool CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& keyid, const CScript& scriptCode) const;
};

/** A signature creator that just produces 72-byte empty signatyres. */
class DummySignatureCreator : public BaseSignatureCreator {
public:
    DummySignatureCreator(const CKeyStore* keystoreIn) : BaseSignatureCreator(keystoreIn) {}
    const BaseSignatureChecker& Checker() const;
    bool CreateSig(std::vector<unsigned char>& vchSig, const CKeyID& keyid, const CScript& scriptCode) const;
};

/** Produce a script signature using a generic signature creator. */
bool ProduceSignature(const BaseSignatureCreator& creator, const CScript& scriptPubKey, CScript& scriptSig);

/** Produce a script signature for a transaction. */
bool SignSignature(const CKeyStore& keystore, const CScript& fromPubKey, CMutableTransaction& txTo, unsigned int nIn, int nHashType=SIGHASH_ALL);
bool SignSignature(const CKeyStore& keystore, const CTransaction& txFrom, CMutableTransaction& txTo, unsigned int nIn, int nHashType=SIGHASH_ALL);

/** Combine two script signatures using a generic signature checker, intelligently, possibly with OP_0 placeholders. */
CScript CombineSignatures(const CScript& scriptPubKey, const BaseSignatureChecker& checker, const CScript& scriptSig1, const CScript& scriptSig2);

/** Combine two script signatures on transactions. */
CScript CombineSignatures(const CScript& scriptPubKey, const CTransaction& txTo, unsigned int nIn, const CScript& scriptSig1, const CScript& scriptSig2);

#endif // CODECOIN_SCRIPT_SIGN_H
