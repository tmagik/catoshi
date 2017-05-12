// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html


#ifndef CODECOIN_PRIMITIVES_BLOCK_H
#define CODECOIN_PRIMITIVES_BLOCK_H

#include "codecoin.h"
#include "primitives/transaction.h"
#include "serialize.h"
#include "uintBIG.h"

#if defined(BRAND_grantcoin)
#include "keystore.h"
#endif

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE = 1000000;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
/** Catoshi: TODO: first class validating mobile-friendly wallet that can pull
 * 'historical' blocks that have been checkpoint validated from the p2p network
 * or from trusted servers. Ideally clients that do not keep full blockchains 
 * but are staking would pay fees to retrieve blocks from nodes that have a full
 * copy. Call this proof-of-data, or proof-of-history
 */
class BlockHeader
{
public:
    // header
    static const int32_t CURRENT_VERSION=4;
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    BlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    void SetNull()
    {
        nVersion = BlockHeader::CURRENT_VERSION;
        hashPrevBlock = 0;
        hashMerkleRoot = 0;
        nTime = 0;
        nBits = 0;
        nNonce = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    uint256 GetPoWHash() const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};

#if defined(BRAND_grantcoin)
class BlockHeaderGRT : public BlockHeader
{
public:
    static const int VERSION_STAKE_START=2; // not actually used, remove?
    static const int CURRENT_VERSION=1;

    uint256 GetPowHash() const
    {
        return GetHash();
    }
};
#endif

#if defined(BRAND_grantcoin)
class BlockGRT : public BlockHeaderGRT
#else
class Block : public BlockHeader
#endif
{
public:
    // network and disk
    std::vector<CTransaction> vtx;
#if defined(BRAND_grantcoin)
	// ppcoin: block signature - signed by coin base txout[0]'s owner
	std::vector<unsigned char> BlockSig;
#endif

    // memory only
    mutable std::vector<uint256> vMerkleTree;

#if defined(BRAND_grantcoin)
    BlockGRT()
    {
        SetNull();
    }

    BlockGRT(const BlockHeaderGRT &header)
    {
        SetNull();
        *((BlockHeaderGRT*)this) = header;
    }
#else
    Block()
    {
        SetNull();
    }

    Block(const BlockHeader &header)
    {
        SetNull();
        *((BlockHeader*)this) = header;
    }
#endif

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
#if defined(BRAND_grantcoin)
	READWRITE(BlockSig);
#endif
    }

    void SetNull()
    {
        BlockHeader::SetNull();
        vtx.clear();
#if defined(BRAND_grantcoin)
	BlockSig.clear();
#endif
        vMerkleTree.clear();
    }

    BlockHeader GetBlockHeader() const
    {
        BlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        return block;
    }

    // Build the in-memory merkle tree for this block and return the merkle root.
    // If non-NULL, *mutated is set to whether mutation was detected in the merkle
    // tree (a duplication of transactions in the block leading to an identical
    // merkle root).
    uint256 BuildMerkleTree(bool* mutated = NULL) const;

    std::vector<uint256> GetMerkleBranch(int nIndex) const;
    static uint256 CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex);
    std::string ToString() const;
};

#if 0
#if defined(BRAND_grantcoin)
/* this is probably not the right way to abstract this */
class BlockGRT : public Block
{
	std::vector<unsigned char> BlockSig;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(*(CBlockHeader*)this);
		READWRITE(vtx);
		READWRITE(BlockSig);
	}

	void SetNull()
	{
		BlockHeaderGRT::SetNull()
		vtx.clear();
		BlockSig.clear();
		vMerkleTree.clear();
	}

	bool isProofOfStake() const
	{ return false }

	bool SignBlock(const CKeyStore& keystore);
	bool CheckBlockSignature() const;
};
#endif
#endif

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull()
    {
        return vHave.empty();
    }
};

#endif // CODECOIN_PRIMITIVES_BLOCK_H
