// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2013 SolarCoin developers
// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include "solarcoin.h"
#include "alert.h"
#include "checkpoints.h"
#include "db.h"
#include "txdb.h"
#include "net.h"
#include "init.h"
#include "ui_interface.h"
#include "checkqueue.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;
using namespace boost;

uint256 hashGenesisBlock = 0;			// TODO: objectize this for multicoin support
CBigNum bnProofOfWorkLimit(~uint256(0) >> 20);	// *coin: starting difficulty is 1 / 2^20

const string strMessageMagic = "SolarCoin Signed Message:\n";

// DNS seeds
// Each pair gives a source name and a seed name.
// The first name is used as information source for addrman.
// The second name should resolve to a list of seed addresses.
// FIXME use a single string and/or objectify this
const char *strMainNetDNSSeed[][2] = {
	{"catstat.info", "seed.catstat.info"},
	{"solarcoin.org", "seed.solarcoin.org"},
	{NULL, NULL}
};

const char *strTestNetDNSSeed[][2] = {
	{"catstat.info", "testnet-seed.catstat.info"},
	{"solarcoin.org", "seed.solarcoin.org"},
	{NULL, NULL}
};

int64_t GetBlockValue(CBlockIndex *block, int64_t nFees)
{
    int64_t nSubsidy = 100 * COIN; 
	if(block->nHeight < 99) {nSubsidy = 1000000000 * COIN;}
	nSubsidy >>= (block->nHeight / 525600);
    return nSubsidy + nFees;
}

static const int64_t nTargetTimespan_Version1 = 24 * 60 * 60; // SolarCoin: 24 Hours
static const int64_t nTargetSpacing = 60 ; // SolarCoin: 1 Minute Blocks
static const int64_t nInterval_Version1 = nTargetTimespan_Version1 / nTargetSpacing; // SolarCoin: 1440 blocks

//this is used in computeminwork but no longer in getnextwork
static const int64_t nHeight_Version2 = 208440;
static const int64_t nInterval_Version2 = 15;
static const int64_t nTargetTimespan_Version2 = nInterval_Version2 * nTargetSpacing; // 15 minutes

//block to apply patch
static const int64_t DiffChangeBlock = 200000;

//
// minimum amount of work that could possibly be required nTime after
// minimum work required was nBase
//
unsigned int ComputeMinWork(unsigned int nBase, int64_t nTime)
{
    // Testnet has min-difficulty blocks
    // after nTargetSpacing*2 time between blocks:
    if (fTestNet && nTime > nTargetSpacing*2)
        return bnProofOfWorkLimit.GetCompact();

    CBigNum bnResult;
    bnResult.SetCompact(nBase);
    while (nTime > 0 && bnResult < bnProofOfWorkLimit)
    {
        // Maximum 400% adjustment...
        bnResult *= 4;
        // ... in best-case exactly 4-times-normal target time
        nTime -= nTargetTimespan_Version2*4;
    }
    if (bnResult > bnProofOfWorkLimit)
        bnResult = bnProofOfWorkLimit;
    return bnResult.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
    unsigned int nProofOfWorkLimit = bnProofOfWorkLimit.GetCompact();
    int nHeight = pindexLast->nHeight + 1;

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;
        
    int64_t nInterval;
    int64_t nTargetTimespan;
    
    nInterval = nInterval_Version1;
    nTargetTimespan = nTargetTimespan_Version1;
    
    
    if (nHeight >= DiffChangeBlock) {
        nTargetTimespan = 60; //1 min
        nInterval = nTargetTimespan / nTargetSpacing; //60/60 = 1 block
        //target timespan remains the same, 1 min
    }
        
    // Only change once per interval
    if ((pindexLast->nHeight+1) % nInterval != 0)
    {
        // Special difficulty rule for testnet:
        if (fTestNet)
        {
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->nTime > pindexLast->nTime + nTargetSpacing*2)
                return nProofOfWorkLimit;
            
            /* 5/30 - temporarily removing to test difficulty retargeting
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % nInterval != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            } */
        }

        return pindexLast->nBits;
    }

    // Litecoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = nInterval-1;
    if ((pindexLast->nHeight+1) != nInterval)
        blockstogoback = nInterval;

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;
    assert(pindexFirst);

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    printf("  nActualTimespan = %" PRId64"  before bounds\n", nActualTimespan);
    if (nActualTimespan < nTargetTimespan/4)
        nActualTimespan = nTargetTimespan/4;
    if (nActualTimespan > nTargetTimespan*4)
        nActualTimespan = nTargetTimespan*4;
    
    if (nHeight >= DiffChangeBlock) //courtesy RealSolid and WDC
    {
        // amplitude filter - thanks to daft27 for this code
        nActualTimespan = nTargetTimespan + (nActualTimespan - nTargetTimespan)/8;
        if (nActualTimespan < (nTargetTimespan - (nTargetTimespan/4)) ) nActualTimespan = (nTargetTimespan - (nTargetTimespan/4));
        if (nActualTimespan > (nTargetTimespan + (nTargetTimespan/2)) ) nActualTimespan = (nTargetTimespan + (nTargetTimespan/2));
    }

    // Retarget
    CBigNum bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespan;

    if (bnNew > bnProofOfWorkLimit)
        bnNew = bnProofOfWorkLimit;

    /// debug print
    printf("GetNextWorkRequired RETARGET\n");
    printf("nTargetTimespan = %" PRId64"    nActualTimespan = %" PRId64"\n", nTargetTimespan, nActualTimespan);
    printf("Before: %08x  %s\n", pindexLast->nBits, CBigNum().SetCompact(pindexLast->nBits).getuint256().ToString().c_str());
    printf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.getuint256().ToString().c_str());

    return bnNew.GetCompact();
}

const char *pchSLRMain	= "\x04\xf1\x04\xfd";
const char *pchSLRTest	= "\xfd\xc0\x5a\xf2";

unsigned char pchMessageStart[4];

bool LoadBlockIndex()
{
	if (fTestNet)
	{	/* yes this could be more elegant */
		pchMessageStart[0] = pchSLRTest[0];
		pchMessageStart[1] = pchSLRTest[1];
		pchMessageStart[2] = pchSLRTest[2];
		pchMessageStart[3] = pchSLRTest[3];
		hashGenesisBlock = uint256("0xec7987a2ab5225246c5cf9b8d93b4b75bcef383a4a65d5a265bc09ed54006188");
	} else {
		pchMessageStart[0] = pchSLRMain[0];
		pchMessageStart[1] = pchSLRMain[1];
		pchMessageStart[2] = pchSLRMain[2];
		pchMessageStart[3] = pchSLRMain[3];
		hashGenesisBlock = uint256("0xbc3b4ec43c4ebb2fef49e6240812549e61ffa623d9418608aa90eaad26c96296");
	}

	//
	// Load block index from databases
	//
	if (!fReindex && !LoadBlockIndexDB())
		return false;

	return true;
}


bool InitBlockIndex() {
	// Check whether we're already initialized
	if (pindexGenesisBlock != NULL)
		return true;

	// Use the provided setting for -txindex in the new database
	fTxIndex = GetBoolArg("-txindex", false);
	pblocktree->WriteFlag("txindex", fTxIndex);
	printf("Initializing databases...\n");

	// Only add the genesis block if not reindexing (in which case we reuse the one already on disk)
	if (!fReindex) {
		// Genesis Block:
		// CBlock(hash=12a765e31ffd4059bada, PoW=0000050c34a64b415b6b, ver=1, hashPrevBlock=00000000000000000000, hashMerkleRoot=97ddfbbae6, nTime=1317972665, nBits=1e0ffff0, nNonce=2084524493, vtx=1)
		//	 CTransaction(hash=97ddfbbae6, ver=1, vin.size=1, vout.size=1, nLockTime=0)
		//	   CTxIn(COutPoint(0000000000, -1), coinbase 04ffff001d0104404e592054696d65732030352f4f63742f32303131205374657665204a6f62732c204170706c65e280997320566973696f6e6172792c2044696573206174203536)
		//	   CTxOut(nValue=50.00000000, scriptPubKey=040184710fa689ad5023690c80f3a4)
		//	 vMerkleTree: 97ddfbbae6

		// Genesis block
		const char* pszTimestamp = "NY Times - December 23, 2013 - For Today's Babes, Toyland Is Digital";
		CTransaction txNew;
		txNew.vin.resize(1);
		txNew.vout.resize(1);
		txNew.vin[0].scriptSig = CScript() << 486604799 << CBigNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
		txNew.vout[0].nValue = 50 * COIN;
		txNew.vout[0].scriptPubKey = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
		CBlock block;
		block.vtx.push_back(txNew);
		block.hashPrevBlock = 0;
		block.hashMerkleRoot = block.BuildMerkleTree();
		block.nVersion = 1;
		block.nTime    = 1387838302;
		block.nBits    = 0x1e0ffff0;
		block.nNonce   = 588050;

		if (fTestNet)
		{
			block.nTime    = 1387838303; //FIXME testnet0.1
			block.nNonce   = 608937;
		}

		//// debug print
		uint256 hash = block.GetHash();
		printf("%s\n", hash.ToString().c_str());
		printf("%s\n", hashGenesisBlock.ToString().c_str());
		printf("%s\n", block.hashMerkleRoot.ToString().c_str());
		assert(block.hashMerkleRoot == uint256("0x4007a33db5d9cdf2aab117335eb8431c8d13fb86e0214031fdaebe69a0f29cf7"));


		block.print();
		assert(hash == hashGenesisBlock);

		// Start new block file
		try {
			unsigned int nBlockSize = ::GetSerializeSize(block, SER_DISK, CLIENT_VERSION);
			CDiskBlockPos blockPos;
			CValidationState state;
			if (!FindBlockPos(state, blockPos, nBlockSize+8, 0, block.nTime))
				return error("LoadBlockIndex() : FindBlockPos failed");
			if (!block.WriteToDisk(blockPos))
				return error("LoadBlockIndex() : writing genesis block to disk failed");
			if (!block.AddToBlockIndex(state, blockPos))
				return error("LoadBlockIndex() : genesis block not accepted");
		} catch(std::runtime_error &e) {
			return error("LoadBlockIndex() : failed to initialize block database: %s", e.what());
		}
	}

	return true;
}


