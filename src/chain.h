// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CHAIN_H
#define BITCOIN_CHAIN_H

#include "primitives/block.h"
#include "pow.h"
#include "tinyformat.h"
#include "uintBIG.h"

#include <vector>

#include <boost/foreach.hpp>

struct CDiskBlockPos
{
    int nFile;
    unsigned int nPos;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(VARINT(nFile));
        READWRITE(VARINT(nPos));
    }

    CDiskBlockPos() {
        SetNull();
    }

    CDiskBlockPos(int nFileIn, unsigned int nPosIn) {
        nFile = nFileIn;
        nPos = nPosIn;
    }

    friend bool operator==(const CDiskBlockPos &a, const CDiskBlockPos &b) {
        return (a.nFile == b.nFile && a.nPos == b.nPos);
    }

    friend bool operator!=(const CDiskBlockPos &a, const CDiskBlockPos &b) {
        return !(a == b);
    }

    void SetNull() { nFile = -1; nPos = 0; }
    bool IsNull() const { return (nFile == -1); }
};

enum BlockStatus {
    //! Unused.
    BLOCK_VALID_UNKNOWN      =    0,

    //! Parsed, version ok, hash satisfies claimed PoW, 1 <= vtx count <= max, timestamp not in future
    BLOCK_VALID_HEADER       =    1,

    //! All parent headers found, difficulty matches, timestamp >= median previous, checkpoint. Implies all parents
    //! are also at least TREE.
    BLOCK_VALID_TREE         =    2,

    /**
     * Only first tx is coinbase, 2 <= coinbase input script length <= 100, transactions valid, no duplicate txids,
     * sigops, size, merkle root. Implies all parents are at least TREE but not necessarily TRANSACTIONS. When all
     * parent blocks also have TRANSACTIONS, CBlockIndex::nChainTx will be set.
     */
    BLOCK_VALID_TRANSACTIONS =    3,

    //! Outputs do not overspend inputs, no double spends, coinbase output ok, immature coinbase spends, BIP30.
    //! Implies all parents are also at least CHAIN.
    BLOCK_VALID_CHAIN        =    4,

    //! Scripts & signatures ok. Implies all parents are also at least SCRIPTS.
    BLOCK_VALID_SCRIPTS      =    5,

    //! All validity bits.
    BLOCK_VALID_MASK         =   BLOCK_VALID_HEADER | BLOCK_VALID_TREE | BLOCK_VALID_TRANSACTIONS |
                                 BLOCK_VALID_CHAIN | BLOCK_VALID_SCRIPTS,

    BLOCK_HAVE_DATA          =    8, //! full block available in blk*.dat
    BLOCK_HAVE_UNDO          =   16, //! undo data available in rev*.dat
    BLOCK_HAVE_MASK          =   BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO,

    BLOCK_FAILED_VALID       =   32, //! stage after last reached validness failed
    BLOCK_FAILED_CHILD       =   64, //! descends from failed block
    BLOCK_FAILED_MASK        =   BLOCK_FAILED_VALID | BLOCK_FAILED_CHILD,
};

/** The block chain is a tree shaped structure starting with the
 * genesis block at the root, with each block potentially having multiple
 * candidates to be the next block. A blockindex may have multiple pprev pointing
 * to it, but at most one of them can be part of the currently active branch.
 */
class CBlockIndex
{
public:
    //! pointer to the hash of the block, if any. memory is owned by this CBlockIndex
    const uint256* phashBlock;

    //! pointer to the index of the predecessor of this block
    CBlockIndex* pprev;

    //! pointer to the index of some further predecessor of this block
    CBlockIndex* pskip;

    //! height of the entry in the chain. The genesis block has height 0
    int nHeight;

    //! Which # file this block is stored in (blk?????.dat)
    int nFile;

    //! Byte offset within blk?????.dat where this block's data is stored
    unsigned int nDataPos;

    //! Byte offset within rev?????.dat where this block's undo data is stored
    unsigned int nUndoPos;

	/* (memory only) Total amount of trust score in the chain up to and including this block
	 * For proof-of-work only coins, this is approximately equivalent to the number of hashes
	 * applied towards finding proof-of-work.
	 * For proof-of-stake only coins, this is (mostly) the coin-age consumed by staking, with
	 * some multipliers to get something that looks like the old Proof-of-Work 'nChainWork'.
	 * For hybrid coins, this is (at least conceptually) PoW-hashes + PoS-CoinAge*multiplier
	 * HERE THERE BE DRAGONS (or at least hackers and stealth-switchpools)
	 * The dragons in the details centers around assumptions about the ratio of PoS CoinAge
	 * with PoS Hashes can go all over the place and change by orders of magnitude when ASIC
	 * hashpower shows up.
	 * There are further issues with the 'nothing-at-stake' attacks, and all of this makes
	 * Proof-of-stake a much more complicated and potentially ambiguous consensus system.
	 * Hybrid systems, with a properly functioning stake/hash trust multiplier can mitigate
	 * some of the ambiguity by requiring a consensus between miners and stakers. */
	/* compatibility defines for legacy Satoshi & Sunny King *coin code */
	#define nChainWork ChainTrust
	#define nChainTrust ChainTrust
	uint256 ChainTrust;

    //! Number of transactions in this block.
    //! Note: in a potential headers-first mode, this number cannot be relied upon
    unsigned int nTx;

    //! (memory only) Number of transactions in the chain up to and including this block.
    //! This value will be non-zero only if and only if transactions for this block and all its parents are available.
    //! Change to 64-bit type when necessary; won't happen before 2030
    unsigned int nChainTx;

    //! Verification status of this block. See enum BlockStatus
    unsigned int nStatus;

#if defined(FEATURE_MONEYSUPPLY)
	int64_t nMoneySupply;
#endif
#if defined(PPCOINSTAKE)
	int64_t nMint;

	// ppcoin: proof-of-stake related block index fields
	unsigned int nFlags;  // ppcoin: block index flags
	enum  
	{
		BLOCK_PROOF_OF_STAKE = (1 << 0), // is proof-of-stake block
		BLOCK_STAKE_ENTROPY  = (1 << 1), // entropy bit for stake modifier
		BLOCK_STAKE_MODIFIER = (1 << 2), // regenerated stake modifier
	};

	uint64_t nStakeModifier; // hash modifier for proof-of-stake
	unsigned int nStakeModifierChecksum; // checksum of index; in-memeory only
	COutPoint prevoutStake;
	unsigned int nStakeTime;
	uint256 hashProofOfStake;
#endif
	//! block header
    int nVersion;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;

    //! (memory only) Sequential id assigned to distinguish order in which blocks are received.
    uint32_t nSequenceId;

    void SetNull()
    {
        phashBlock = NULL;
        pprev = NULL;
        pskip = NULL;
        nHeight = 0;
        nFile = 0;
        nDataPos = 0;
        nUndoPos = 0;
        nChainTrust = 0;
#if defined(FEATURE_MONEYSUPPLY)
		nMoneySupply = 0;
#endif
#if defined(PPCOINSTAKE)
		nMint = 0;
		nFlags = 0;
		nStakeModifier = 0;
		nStakeModifierChecksum = 0;
		hashProofOfStake = 0;
#endif
        nTx = 0;
        nChainTx = 0;
        nStatus = 0;
        nSequenceId = 0;

        nVersion       = 0;
        hashMerkleRoot = 0;
        nTime          = 0;
        nBits          = 0;
        nNonce         = 0;
    }

    CBlockIndex()
    {
        SetNull();
    }

    CBlockIndex(const CBlockHeader& block)
    {
        SetNull();

        nVersion       = block.nVersion;
        hashMerkleRoot = block.hashMerkleRoot;
        nTime          = block.nTime;
        nBits          = block.nBits;
        nNonce         = block.nNonce;
#if defined(PPCOINSTAKE)
		if (block.IsProofOfStake())
		{
			SetProofOfStake();
			prevoutStake = block.vtx[1].vin[0].prevout;
			nStakeTime = block.vtx[1].nTime;
		}
		else
		{
			prevoutStake.SetNull();
			nStakeTime = 0;
		}
#endif
    }

    CDiskBlockPos GetBlockPos() const {
        CDiskBlockPos ret;
        if (nStatus & BLOCK_HAVE_DATA) {
            ret.nFile = nFile;
            ret.nPos  = nDataPos;
        }
        return ret;
    }

    CDiskBlockPos GetUndoPos() const {
        CDiskBlockPos ret;
        if (nStatus & BLOCK_HAVE_UNDO) {
            ret.nFile = nFile;
            ret.nPos  = nUndoPos;
        }
        return ret;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        if (pprev)
            block.hashPrevBlock = pprev->GetBlockHash();
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        return block;
    }

    uint256 GetBlockHash() const
    {
        return *phashBlock;
    }

    uint256 GetBlockPoWHash() const
    {
        return GetBlockHeader().GetPoWHash();
    }

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    enum { nMedianTimeSpan=11 };

    int64_t GetMedianTimePast() const
    {
        int64_t pmedian[nMedianTimeSpan];
        int64_t* pbegin = &pmedian[nMedianTimeSpan];
        int64_t* pend = &pmedian[nMedianTimeSpan];

        const CBlockIndex* pindex = this;
        for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
            *(--pbegin) = pindex->GetBlockTime();

        std::sort(pbegin, pend);
        return pbegin[(pend - pbegin)/2];
    }

    /**
     * Returns true if there are nRequired or more blocks of minVersion or above
     * in the last Params().ToCheckBlockUpgradeMajority() blocks, starting at pstart 
     * and going backwards.
     */
    static bool IsSuperMajority(int minVersion, const CBlockIndex* pstart,
                                unsigned int nRequired);

	/* eventually should be overloadable/subclass/derived for various coins */
	int64_t GetSeigniorage(const int64_t nFees=0, const int64_t CoinAge=0) const;


#if defined(PPCOINSTAKE)
	bool IsProofOfWork() const
	{
		return !(nFlags & BLOCK_PROOF_OF_STAKE);
	}

	bool IsProofOfStake() const
	{
		return (nFlags & BLOCK_PROOF_OF_STAKE);
	}

	void SetProofOfStake()
	{
		nFlags |= BLOCK_PROOF_OF_STAKE;
	}

	unsigned int GetStakeEntropyBit() const
	{
		return ((nFlags & BLOCK_STAKE_ENTROPY) >> 1);
	}

	bool SetStakeEntropyBit(unsigned int nEntropyBit)
	{
		if (nEntropyBit > 1)
			return false;
		nFlags |= (nEntropyBit? BLOCK_STAKE_ENTROPY : 0);
		return true;
	}

	bool GeneratedStakeModifier() const
	{
		return (nFlags & BLOCK_STAKE_MODIFIER);
	}

	void SetStakeModifier(uint64_t nModifier, bool fGeneratedStakeModifier)
	{
		nStakeModifier = nModifier;
		if (fGeneratedStakeModifier)
			nFlags |= BLOCK_STAKE_MODIFIER;
	}

	std::string ToString() const
	{
		return strprintf("CBlockIndex(nprev=%p, pnext=%p,  nHeight=%d, nMint=%s, nMoneySupply=%s, nFlags=(%s)(%d)(%s), nStakeModifier=%016" PRIx64", nStakeModifierChecksum=%08x, hashProofOfStake=%s, prevoutStake=(%s), nStakeTime=%d merkle=%s, hashBlock=%s)",
			pprev, pnext, nHeight,
			FormatMoney(nMint).c_str(), FormatMoney(nMoneySupply).c_str(),
			GeneratedStakeModifier() ? "MOD" : "-", GetStakeEntropyBit(), IsProofOfStake()? "PoS" : "PoW",
			nStakeModifier, nStakeModifierChecksum, 
			hashProofOfStake.ToString().c_str(),
			prevoutStake.ToString().c_str(), nStakeTime,
			hashMerkleRoot.ToString().substr(0,10).c_str(),
			GetBlockHash().ToString().substr(0,20).c_str());
	}
#else
	inline bool IsProofOfWork() const { return true; };
	inline bool IsProofOfStake() const { return false; };	

    std::string ToString() const
    {
        return strprintf("CBlockIndex(pprev=%p, nHeight=%d, merkle=%s, hashBlock=%s)",
            pprev, nHeight,
            hashMerkleRoot.ToString(),
            GetBlockHash().ToString());
    }
#endif

    //! Check whether this block index entry is valid up to the passed validity level.
    bool IsValid(enum BlockStatus nUpTo = BLOCK_VALID_TRANSACTIONS) const
    {
        assert(!(nUpTo & ~BLOCK_VALID_MASK)); // Only validity flags allowed.
        if (nStatus & BLOCK_FAILED_MASK)
            return false;
        return ((nStatus & BLOCK_VALID_MASK) >= nUpTo);
    }

    //! Raise the validity level of this block index entry.
    //! Returns true if the validity was changed.
    bool RaiseValidity(enum BlockStatus nUpTo)
    {
        assert(!(nUpTo & ~BLOCK_VALID_MASK)); // Only validity flags allowed.
        if (nStatus & BLOCK_FAILED_MASK)
            return false;
        if ((nStatus & BLOCK_VALID_MASK) < nUpTo) {
            nStatus = (nStatus & ~BLOCK_VALID_MASK) | nUpTo;
            return true;
        }
        return false;
    }

    //! Build the skiplist pointer for this entry.
    void BuildSkip();

    //! Efficiently find an ancestor of this block.
    CBlockIndex* GetAncestor(int height);
    const CBlockIndex* GetAncestor(int height) const;
};

/** Used to marshal pointers into hashes for db storage. */
class CDiskBlockIndex : public CBlockIndex
{
public:
    uint256 hashPrev;

    CDiskBlockIndex() {
        hashPrev = 0;
    }

    explicit CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex) {
        hashPrev = (pprev ? pprev->GetBlockHash() : 0);
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (!(nType & SER_GETHASH))
            READWRITE(VARINT(nVersion));

        READWRITE(VARINT(nHeight));
        READWRITE(VARINT(nStatus));
        READWRITE(VARINT(nTx));
        if (nStatus & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO))
            READWRITE(VARINT(nFile));
        if (nStatus & BLOCK_HAVE_DATA)
            READWRITE(VARINT(nDataPos));
        if (nStatus & BLOCK_HAVE_UNDO)
            READWRITE(VARINT(nUndoPos));

#if defined(PPCOINSTAKE) // TODO: is needed for grantcoin?
		READWRITE(nMint);
#if defined(FEATURE_MONEYSUPPLY)
		READWRITE(nMoneySupply);
#endif
		READWRITE(nFlags);
		READWRITE(nStakeModifier);
		if (IsProofOfStake())
		{
			READWRITE(prevoutStake);
			READWRITE(nStakeTime);
			READWRITE(hashProofOfStake);
		}
		else if (fRead)
		{
			const_cast<CDiskBlockIndex*>(this)->prevoutStake.SetNull();
			const_cast<CDiskBlockIndex*>(this)->nStakeTime = 0;
			const_cast<CDiskBlockIndex*>(this)->hashProofOfStake = 0;
		}
#elif defined(FEATURE_MONEYSUPPLY)
		READWRITE(nMoneySupply);
#endif

        // block header
        READWRITE(this->nVersion);
        READWRITE(hashPrev);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    uint256 GetBlockHash() const
    {
        CBlockHeader block;
        block.nVersion        = nVersion;
        block.hashPrevBlock   = hashPrev;
        block.hashMerkleRoot  = hashMerkleRoot;
        block.nTime           = nTime;
        block.nBits           = nBits;
        block.nNonce          = nNonce;
        return block.GetHash();
    }


    std::string ToString() const
    {
        std::string str = "CDiskBlockIndex(";
        str += CBlockIndex::ToString();
        str += strprintf("\n                hashBlock=%s, hashPrev=%s)",
            GetBlockHash().ToString(),
            hashPrev.ToString());
        return str;
    }
};

/** An in-memory indexed chain of blocks. */
class CChain {
private:
    std::vector<CBlockIndex*> vChain;

public:
    /** Returns the index entry for the genesis block of this chain, or NULL if none. */
    CBlockIndex *Genesis() const {
        return vChain.size() > 0 ? vChain[0] : NULL;
    }

    /** Returns the index entry for the tip of this chain, or NULL if none. */
    CBlockIndex *Tip() const {
        return vChain.size() > 0 ? vChain[vChain.size() - 1] : NULL;
    }

    /** Returns the index entry at a particular height in this chain, or NULL if no such height exists. */
    CBlockIndex *operator[](int nHeight) const {
        if (nHeight < 0 || nHeight >= (int)vChain.size())
            return NULL;
        return vChain[nHeight];
    }

    /** Compare two chains efficiently. */
    friend bool operator==(const CChain &a, const CChain &b) {
        return a.vChain.size() == b.vChain.size() &&
               a.vChain[a.vChain.size() - 1] == b.vChain[b.vChain.size() - 1];
    }

    /** Efficiently check whether a block is present in this chain. */
    bool Contains(const CBlockIndex *pindex) const {
        return (*this)[pindex->nHeight] == pindex;
    }

    /** Find the successor of a block in this chain, or NULL if the given index is not found or is the tip. */
    CBlockIndex *Next(const CBlockIndex *pindex) const {
        if (Contains(pindex))
            return (*this)[pindex->nHeight + 1];
        else
            return NULL;
    }

    /** Return the maximal height in the chain. Is equal to chain.Tip() ? chain.Tip()->nHeight : -1. */
    int Height() const {
        return vChain.size() - 1;
    }

    /** Set/initialize a chain with a given tip. */
    void SetTip(CBlockIndex *pindex);

    /** Return a CBlockLocator that refers to a block in this chain (by default the tip). */
    CBlockLocator GetLocator(const CBlockIndex *pindex = NULL) const;

    /** Find the last common block between this chain and a block index entry. */
    const CBlockIndex *FindFork(const CBlockIndex *pindex) const;
};

#endif // BITCOIN_CHAIN_H
