// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2013-2014 The Catcoin developers
// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include "catcoin.h"
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
CBigNum bnProofOfWorkLimit(~uint256(0) >> 12);	// *coin: starting difficulty is 1 / 2^12

const string strMessageMagic = "Catcoin Signed Message:\n";
/* value, in percent of what difficulty value we'll accept for orphans */
const int ORPHAN_WORK_THRESHOLD = 1; // FIXME WAY TOO WIDE right now

// DNS seeds
// Each pair gives a source name and a seed name.
// The first name is used as information source for addrman.
// The second name should resolve to a list of seed addresses.
// FIXME use a single string and/or objectify this
/*
  Catcoin policy for getting on this list:
   1) have a catcoind with port 9933 open
   2) be avaible for #catcoin-dev people to contact you for debug and testing
   3) be willing to take a haircut on generates if we determine on irc and on
      (a future) mailing list that we need to do it to fix the network
 */
const char *strMainNetDNSSeed[][2] = {
    {"catstat.info", "seed.catstat.info"},
    {"catcoinwallets.com", "seed.catcoinwallets.com"},
    {"geekhash.org", "cat.geekhash.org"},
    {NULL, NULL}
};

const char *strTestNetDNSSeed[][2] = {
    {"catstat.info", "testnet-seed.catstat.info"},
    {"catcoinwallets.com", "seed.catcoinwallets.com"},
    {NULL, NULL}
};

int64 GetBlockValue(CBlockIndex *block, int64 nFees)
{
    int64 nSubsidy = 50 * COIN;

    // Making some sort of 'promise' of 21 million coins is like Monsanto trying to patent
    // any roundup-resistant plant, or insisting on only running the 'Genuine IBM PC'
    // Sure you can try to do that, but weeds evolve resistance, China makes clones,
    // and copycatcoins print money faster than Zimbabwe after they got rid of the farmers.
    //
    // Sound currency will come from a robust community that values something in common.
    // like Cat pictures.  -- Farmer Troy
    //
    // FIXME: add Depurrage based on last year of Cat food prices in the blockchain
    // FIXME2: come up with a way to GET cat food prices in the blockchain
    // FIXME3: do this more elegantly while maintaining readability
    // Bitcoin/Litecoin used to have this
    // Subsidy is cut in half every 210000 blocks, which will occur approximately every 4 years
    // nSubsidy >>= (block->nHeight / 210000);

    return nSubsidy + nFees;
}

static const int64 nTargetTimespan = 6 * 60 * 60; // 6 hours
static const int64 nTargetSpacing = 10 * 60;
static const int64 nMinSpacing = 30; 	// Absolute minimum spacing
static const int64 nInterval = nTargetTimespan / nTargetSpacing;

static const int64 nTargetTimespanOld = 14 * 24 * 60 * 60; // two weeks
static const int64 nIntervalOld = nTargetTimespanOld / nTargetSpacing;

//
// minimum amount of work that could possibly be required nTime after
// minimum work required was nBase
//
unsigned int ComputeMinWork(unsigned int nBase, int64 nTime)
{
	// Testnet has min-difficulty blocks
	// after nTargetSpacing*2 time between blocks:
	if (fTestNet && nTime > nTargetSpacing*2)
		return bnProofOfWorkLimit.GetCompact();

	CBigNum bnResult;
	bnResult.SetCompact(nBase);
	while (nTime > 0 && bnResult < bnProofOfWorkLimit)
	{
	// should technically be 112/100 * 36 .. ~40
        bnResult *= 40;
        // and if we have long blocks, max 40 x, as well
        nTime -= nTargetTimespan*40;
	}
	if (bnResult > bnProofOfWorkLimit)
		bnResult = bnProofOfWorkLimit;
	return bnResult.GetCompact();
}

static int fork3Block = 27260; // FIXME move to top...
static int fork4Block = 27680; // Acceptblock needs this

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
    int64 nTargetTimespanLocal = 0;
    int64 nIntervalLocal = 0;
    int forkBlock = 20290 - 1;
    int fork2Block = 21346;

	 // moved variable inits to the top where they belong
	 
    unsigned int nProofOfWorkLimit = bnProofOfWorkLimit.GetCompact();
    int64 nActualTimespan;
    const CBlockIndex* pindexFirst = pindexLast;

	 int64 error;    
	 //int64 diffcalc;
    double pGainUp=-0.005125;	// Theses values can be changed to tune the PID formula
    double iGainUp=-0.0225;	// Theses values can be changed to tune the PID formula
    double dGainUp=-0.0075; 	// Theses values can be changed to tune the PID formula

    double pGainDn=-0.005125;	// Theses values can be changed to tune the PID formula
    double iGainDn=-0.0525;	// Theses values can be changed to tune the PID formula
    double dGainDn=-0.0075; 	// Theses values can be changed to tune the PID formula

    double pCalc;
    double iCalc;
    double dCalc;
    double dResult;
    int64 result;
    CBigNum bResult;
    CBigNum bnNew;
    int i;
    //CBigNum bLowLimit; // Limit for PID calc to never go below this
    
    if(fTestNet){
        forkBlock = -1;
        fork2Block = 36;
    }

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // Starting from block 20,290 the network diff was set to 16
    // and the retarget interval was changed to 36
	if(pindexLast->nHeight < forkBlock && !fTestNet) 
	{
        nTargetTimespanLocal = nTargetTimespanOld;
        nIntervalLocal = nIntervalOld;
    } 
    else if(pindexLast->nHeight == forkBlock && !fTestNet) 
    {
        	bnNew.SetCompact(0x1c0ffff0); // Difficulty 16
        	return bnNew.GetCompact();
    } 
    else // Keep in for a resync
    {
        nTargetTimespanLocal = nTargetTimespan;
        nIntervalLocal = nInterval;
    }

    // after fork2Block we retarget every block   
    if(pindexLast->nHeight < fork2Block && !fTestNet)
    {
        // Only change once per interval
        if ((pindexLast->nHeight+1) % nIntervalLocal != 0 && !fTestNet)
        {
            // Special difficulty rule for testnet:
            if (fTestNet)
            {
                // If the new block's timestamp is more than 2* 10 minutes
                // then allow mining of a min-difficulty block.
                if (pblock->nTime > pindexLast->nTime + nTargetSpacing*2)
                    return nProofOfWorkLimit;
                else
                {
                    // Return the last non-special-min-difficulty-rules-block
                    const CBlockIndex* pindex = pindexLast;
                    while (pindex->pprev && pindex->nHeight % nIntervalLocal != 0 && pindex->nBits == nProofOfWorkLimit)
                        pindex = pindex->pprev;
                    return pindex->nBits;
                }
            }

            return pindexLast->nBits;
        }
    }


	if(pindexLast->nHeight < fork3Block && !fTestNet) // let it walk through 2nd fork stuff if below fork3Block, and ignore if on testnet
	{
#warning retabme
    // Catcoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = nIntervalLocal-1;
    if ((pindexLast->nHeight+1) != nIntervalLocal)
        blockstogoback = nIntervalLocal;

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;
    assert(pindexFirst);

    // Limit adjustment step
    int numerator = 4;
    int denominator = 1;
    if(pindexLast->nHeight >= fork2Block){
        numerator = 112;
        denominator = 100;
    }
    int64 nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    int64 lowLimit = nTargetTimespanLocal*denominator/numerator;
    int64 highLimit = nTargetTimespanLocal*numerator/denominator;
    printf("  nActualTimespan = %"PRI64d"  before bounds\n", nActualTimespan);
    if (nActualTimespan < lowLimit)
        nActualTimespan = lowLimit;
    if (nActualTimespan > highLimit)
        nActualTimespan = highLimit;

    // Retarget
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= nTargetTimespanLocal;

    if (bnNew > bnProofOfWorkLimit)
        bnNew = bnProofOfWorkLimit;

    /// debug print
    printf("GetNextWorkRequired RETARGET\n");
    printf("nTargetTimespan = %"PRI64d"    nActualTimespan = %"PRI64d"\n", nTargetTimespanLocal, nActualTimespan);
    printf("Before: %08x  %s\n", pindexLast->nBits, CBigNum().SetCompact(pindexLast->nBits).getuint256().ToString().c_str());
    printf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.getuint256().ToString().c_str());
	}
/*
PID formula
Error = Actual Time - Desired time
P Calc = pGain * Error
I Calc = iGain * Error * (Desired Time / Actual Time) 
D Calc = dGain * (Error / Actual Time) * I Calc

New Diff = (Current Diff + P Calc + I Calc + D Calc)

If New diff < 0, then set static value of 0.0001 or so.
*/	
	
	if(pindexLast->nHeight >= fork3Block || fTestNet)
	// Fork 3 to use a PID routine instead of the other 2 forks 
	{
		pindexFirst = pindexLast->pprev; 	// Set previous block
		for(i=0;i<7;i++) pindexFirst = pindexFirst->pprev; // Set 4th previous block for 8 block filtering 
		nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime(); 	// Get last X blocks time
		nActualTimespan = nActualTimespan / 8; 	// Calculate average for last 8 blocks
		if(pindexLast->nHeight > fork4Block || fTestNet){
			if (nMinSpacing > nActualTimespan){
				printf("WARNING: SANITY CHECK FAILED: PID nActualTimespan %"PRI64d" too small! increased to %"PRI64d"\n",
					nActualTimespan, nMinSpacing );
				nActualTimespan = nMinSpacing;
			}
		}
		bnNew.SetCompact(pindexLast->nBits);	// Get current difficulty
		i=0;					// Zero bit-shift counter
		while(bnNew>0)				// Loop while bnNew > 0
		{
			i++;				// Increment bit-shift counter
			bnNew = bnNew >> 1;		// shift bnNew lower by 1 bit
			if(i>256) bnNew = 0;		// overflow test, just to make sure that it never stays in this loop
		}
		bnNew.SetCompact(pindexLast->nBits);	// Get current difficulty again
		

		error = nActualTimespan - nTargetSpacing;	// Calculate the error to be fed into the PID Calculation
		if(error >= -450 && error <= 450) // Slower gains for when the average time is within 2.5 min and 7.5 min 
		{
			// Calculate P ... pGainUp defined at beginning of routine
			pCalc = pGainUp * (double)error;
			// Calculate I ... iGainUp defined at beginning of routine
			iCalc = iGainUp * (double)error * (double)((double)nTargetSpacing / (double)nActualTimespan);
			// Calculate D ... dGainUp defined at beginning of routine
			dCalc = dGainUp * ((double)error / (double)nActualTimespan) * iCalc;
		}
		else // Faster gains for block averages faster than 2.5 min and greater than 7.5 min 
		{
			// Calculate P ... pGainDn defined at beginning of routine
			pCalc = pGainDn * (double)error;
			// Calculate I ... iGainDn defined at beginning of routine
			iCalc = iGainDn * (double)error * (double)((double)nTargetSpacing / (double)nActualTimespan);
			// Calculate D ... dGainDn defined at beginning of routine
			dCalc = dGainDn * ((double)error / (double)nActualTimespan) * iCalc;
		}

		if(error > -10 && error < 10)
		{
			if(fTestNet) printf("Within dead zone. No change!  error: %"PRI64d"\n", error);
			return(bnNew.GetCompact());
		}	 	
	 	
	 	dResult = pCalc + iCalc + dCalc;	// Sum the PID calculations
	 	
		result = (int64)(dResult * 65536);	// Adjust for scrypt calcuation
		// Bring the result within max range to avoid overflow condition 
		while(result >  8388607) result = result / 2; 
		bResult = result;			// Set the bignum value
		if(i>24) bResult = bResult << (i - 24);	// bit-shift integer value of result to be subtracted from current diff

		if(fTestNet) printf("pCalc: %f, iCalc: %f, dCalc: %f, Result: %"PRI64d" (%f)\n", pCalc, iCalc, dCalc, result, dResult);
		if(fTestNet) printf("Actual Time: %"PRI64d", error: %"PRI64d"\n", nActualTimespan, error); 
		if(fTestNet) printf("Result: %08x %s\n",bResult.GetCompact(), bResult.getuint256().ToString().c_str()); 
		if(fTestNet) printf("Before: %08x %s\n",bnNew.GetCompact(), bnNew.getuint256().ToString().c_str()); 
		bnNew = bnNew - bResult; 	// Subtract the result to set the current diff
		
		// Make sure that diff is not set too low, ever
		if (bnNew.GetCompact() > 0x1e0fffff) bnNew.SetCompact(0x1e0fffff);
		if(fTestNet) printf("After:  %08x %s\n",bnNew.GetCompact(), bnNew.getuint256().ToString().c_str()); 
		
	} // End Fork 3 to use a PID routine instead of the other 2 forks routine

    return bnNew.GetCompact();
}


const char *pchCatMain	= "\xfc\xc1\xb7\xdc";
const char *pchCatTest	= "\xfd\xcb\xb8\xdd";

unsigned char pchMessageStart[4];

bool LoadBlockIndex()
{
	if (fTestNet)
	{	/* add 1 to litecoin values (3 to bitcoins) */
		pchMessageStart[0] = pchCatTest[0];
		pchMessageStart[1] = pchCatTest[1];
		pchMessageStart[2] = pchCatTest[2];
		pchMessageStart[3] = pchCatTest[3];
		hashGenesisBlock = uint256("0xec7987a2ab5225246c5cf9b8d93b4b75bcef383a4a65d5a265bc09ed54006188");
	} else {
		pchMessageStart[0] = pchCatMain[0];
		pchMessageStart[1] = pchCatMain[1];
		pchMessageStart[2] = pchCatMain[2];
		pchMessageStart[3] = pchCatMain[3];
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
        //   CTransaction(hash=97ddfbbae6, ver=1, vin.size=1, vout.size=1, nLockTime=0)
        //     CTxIn(COutPoint(0000000000, -1), coinbase 04ffff001d0104404e592054696d65732030352f4f63742f32303131205374657665204a6f62732c204170706c65e280997320566973696f6e6172792c2044696573206174203536)
        //     CTxOut(nValue=50.00000000, scriptPubKey=040184710fa689ad5023690c80f3a4)
        //   vMerkleTree: 97ddfbbae6

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


