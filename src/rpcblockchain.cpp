// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The *coin developers
// where * = (Bit, Lite, PP, Peerunity, Blu, Cat, Solar, URO, ...)
// Previously distributed under the MIT/X11 software license, see the
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include "main.h"
#include "codecoinrpc.h"

using namespace json_spirit;
using namespace std;

extern void TxToJSON(const CTransaction& tx, const uint256 hashBlock, json_spirit::Object& entry);
void ScriptPubKeyToJSON(const CScript& scriptPubKey, Object& out);

double nbitstoDifficulty(unsigned int nBits)
{
	// Floating point number that is a multiple of the minimum difficulty,
	// minimum difficulty = 1.0.
	int nShift = (nBits >> 24) & 0xff;

	double dDiff =
		(double)0x0000ffff / (double)(nBits & 0x00ffffff);

	while (nShift < 29)
	{
		dDiff *= 256.0;
		nShift++;
	}
	while (nShift > 29)
	{
		dDiff /= 256.0;
		nShift--;
	}

	return dDiff;
}

double GetDifficulty(const CBlockIndex* blockindex)
{
	if (blockindex == NULL)
	{
		if (pindexBest == NULL)
			return 1.0;
		else
			blockindex = pindexBest;
	}

	return nbitstoDifficulty(blockindex->nBits);
}

#if defined(PPCOINSTAKE)
double GetPoSKernelPS()
{
    int nPoSInterval = 72;
    double dStakeKernelsTriedAvg = 0;
    int nStakesHandled = 0, nStakesTime = 0;

    CBlockIndex* pindex = pindexBest;;
    CBlockIndex* pindexPrevStake = NULL;

    while (pindex && nStakesHandled < nPoSInterval)
    {
        if (pindex->IsProofOfStake())
        {
            dStakeKernelsTriedAvg += GetDifficulty(pindex) * 4294967296.0;
            nStakesTime += pindexPrevStake ? (pindexPrevStake->nTime - pindex->nTime) : 0;
            pindexPrevStake = pindex;
            nStakesHandled++;
        }

        pindex = pindex->pprev;
    }

    return nStakesTime ? dStakeKernelsTriedAvg / nStakesTime : 0;
}
#endif
 
Object blockToJSON(const CBlock& block, const CBlockIndex* blockindex, bool fPrintTransactionDetail)
//Object blockToJSON(const CBlock& block, const CBlockIndex* blockindex)
{
	Object result;
	result.push_back(Pair("hash", block.GetHash().GetHex()));
	CMerkleTx txGen(block.vtx[0]);
	txGen.SetMerkleBranch(&block);
	result.push_back(Pair("confirmations", (int)txGen.GetDepthInMainChain()));
	result.push_back(Pair("size", (int)::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION)));
	result.push_back(Pair("height", blockindex->nHeight));
	result.push_back(Pair("version", block.nVersion));
	result.push_back(Pair("merkleroot", block.hashMerkleRoot.GetHex()));
	Array txs;
	BOOST_FOREACH(const CTransaction&tx, block.vtx)
		txs.push_back(tx.GetHash().GetHex());
	result.push_back(Pair("tx", txs));
	result.push_back(Pair("time", (boost::int64_t)block.GetBlockTime()));
	result.push_back(Pair("nonce", (boost::uint64_t)block.nNonce));
	result.push_back(Pair("bits", HexBits(block.nBits)));
	result.push_back(Pair("difficulty", GetDifficulty(blockindex)));

	if (blockindex->pprev)
		result.push_back(Pair("previousblockhash", blockindex->pprev->GetBlockHash().GetHex()));
	if (blockindex->pnext)
		result.push_back(Pair("nextblockhash", blockindex->pnext->GetBlockHash().GetHex()));
#if defined(PPCOINSTAKE)    
    result.push_back(Pair("mint", ValueFromAmount(blockindex->nMint)));
    result.push_back(Pair("flags", strprintf("%s%s", blockindex->IsProofOfStake()? "proof-of-stake" : "proof-of-work", blockindex->GeneratedStakeModifier()? " stake-modifier": "")));
    result.push_back(Pair("proofhash", blockindex->IsProofOfStake()? blockindex->hashProofOfStake.GetHex() : blockindex->GetBlockHash().GetHex()));
    result.push_back(Pair("entropybit", (int)blockindex->GetStakeEntropyBit()));
    result.push_back(Pair("modifier", strprintf("%016" PRIx64, blockindex->nStakeModifier)));
    result.push_back(Pair("modifierchecksum", strprintf("%08x", blockindex->nStakeModifierChecksum)));
    Array txinfo;
    BOOST_FOREACH (const CTransaction& tx, block.vtx)
    {
        if (fPrintTransactionDetail)
        {
            Object entry;

            entry.push_back(Pair("txid", tx.GetHash().GetHex()));
            TxToJSON(tx, 0, entry);

            txinfo.push_back(entry);
        }
        else
            txinfo.push_back(tx.GetHash().GetHex());
    }

    result.push_back(Pair("tx", txinfo));
    result.push_back(Pair("signature", HexStr(block.vchBlockSig.begin(), block.vchBlockSig.end())));
#endif /* PPCCOINSTAKE */
	return result;
}


Value getblockcount(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 0)
		throw runtime_error(
			"getblockcount\n"
			"Returns the number of blocks in the longest block chain.");

	return nBestHeight;
}

Value getbestblockhash(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 0)
		throw runtime_error(
			"getbestblockhash\n"
			"Returns the hash of the best (tip) block in the longest block chain.");

	return hashBestChain.GetHex();
}

Value getdifficulty(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 0)
		throw runtime_error(
			"getdifficulty\n"
#if defined(PPCOINSTAKE)
            "Returns the difficulty as a multiple of the minimum difficulty.");

    Object obj;
    obj.push_back(Pair("proof-of-work",        GetDifficulty()));
    obj.push_back(Pair("proof-of-stake",       GetDifficulty(GetLastBlockIndex(pindexBest, true))));
    obj.push_back(Pair("search-interval",      (int)nLastCoinStakeSearchInterval));
    return obj;
#else
			"Returns the proof-of-work difficulty as a multiple of the minimum difficulty.");

	return GetDifficulty();
#endif
}


Value settxfee(const Array& params, bool fHelp)
{
	if (fHelp || params.size() < 1 || params.size() > 1)
		throw runtime_error(
			"settxfee <amount>\n"
			"<amount> is a real and is rounded to the nearest 0.00000001");

	// Amount
	int64_t nAmount = 0;
	if (params[0].get_real() != 0.0)
		nAmount = AmountFromValue(params[0]);		 // rejects 0.0 amounts

	nTransactionFee = nAmount;
	return true;
}

Value getrawmempool(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 0)
		throw runtime_error(
			"getrawmempool\n"
			"Returns all transaction ids in memory pool.");

	vector<uint256> vtxid;
	mempool.queryHashes(vtxid);

	Array a;
	BOOST_FOREACH(const uint256& hash, vtxid)
		a.push_back(hash.ToString());

	return a;
}

Value getblockhash(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 1)
		throw runtime_error(
			"getblockhash <index>\n"
			"Returns hash of block in best-block-chain at <index>.");

	int nHeight = params[0].get_int();
	if (nHeight < 0 || nHeight > nBestHeight)
		throw runtime_error("Block number out of range.");

	CBlockIndex* pblockindex = FindBlockByHeight(nHeight);
	return pblockindex->phashBlock->GetHex();
}

Value getblock(const Array& params, bool fHelp)
{
	if (fHelp || params.size() < 1 || params.size() > 2)
		throw runtime_error(
			"getblock <hash> [verbose=true]\n"
			"If verbose is false, returns a string that is serialized, hex-encoded data for block <hash>.\n"
			"If verbose is true, returns an Object with information about block <hash>."
		);

	std::string strHash = params[0].get_str();
	uint256 hash(strHash);

	bool fVerbose = true;
	if (params.size() > 1)
		fVerbose = params[1].get_bool();

	if (mapBlockIndex.count(hash) == 0)
		throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

	CBlock block;
	CBlockIndex* pblockindex = mapBlockIndex[hash];
	block.ReadFromDisk(pblockindex);

#if !defined(PPCOINSTAKE)
	if (!fVerbose)
	{
		CDataStream ssBlock(SER_NETWORK, PROTOCOL_VERSION);
		ssBlock << block;
		std::string strHex = HexStr(ssBlock.begin(), ssBlock.end());
		return strHex;
	}
#endif
	return blockToJSON(block, pblockindex, fVerbose);
}

Value gettxoutsetinfo(const Array& params, bool fHelp)
{
	if (fHelp || params.size() != 0)
		throw runtime_error(
			"gettxoutsetinfo\n"
			"Returns statistics about the unspent transaction output set.");

	Object ret;

	CCoinsStats stats;
	if (pcoinsTip->GetStats(stats)) {
		ret.push_back(Pair("height", (boost::int64_t)stats.nHeight));
		ret.push_back(Pair("bestblock", stats.hashBlock.GetHex()));
		ret.push_back(Pair("transactions", (boost::int64_t)stats.nTransactions));
		ret.push_back(Pair("txouts", (boost::int64_t)stats.nTransactionOutputs));
		ret.push_back(Pair("bytes_serialized", (boost::int64_t)stats.nSerializedSize));
		ret.push_back(Pair("hash_serialized", stats.hashSerialized.GetHex()));
		ret.push_back(Pair("total_amount", ValueFromAmount(stats.nTotalAmount)));
	}
	return ret;
}

Value gettxout(const Array& params, bool fHelp)
{
	if (fHelp || params.size() < 2 || params.size() > 3)
		throw runtime_error(
			"gettxout <txid> <n> [includemempool=true]\n"
			"Returns details about an unspent transaction output.");

	Object ret;

	std::string strHash = params[0].get_str();
	uint256 hash(strHash);
	int n = params[1].get_int();
	bool fMempool = true;
	if (params.size() > 2)
		fMempool = params[2].get_bool();

	CCoins coins;
	if (fMempool) {
		LOCK(mempool.cs);
		CCoinsViewMemPool view(*pcoinsTip, mempool);
		if (!view.GetCoins(hash, coins))
			return Value::null;
		mempool.pruneSpent(hash, coins); // TODO: this should be done by the CCoinsViewMemPool
	} else {
		if (!pcoinsTip->GetCoins(hash, coins))
			return Value::null;
	}
	if (n<0 || (unsigned int)n>=coins.vout.size() || coins.vout[n].IsNull())
		return Value::null;

	ret.push_back(Pair("bestblock", pcoinsTip->GetBestBlock()->GetBlockHash().GetHex()));
	if ((unsigned int)coins.nHeight == MEMPOOL_HEIGHT)
		ret.push_back(Pair("confirmations", 0));
	else
		ret.push_back(Pair("confirmations", pcoinsTip->GetBestBlock()->nHeight - coins.nHeight + 1));
	ret.push_back(Pair("value", ValueFromAmount(coins.vout[n].nValue)));
	Object o;
	ScriptPubKeyToJSON(coins.vout[n].scriptPubKey, o);
	ret.push_back(Pair("scriptPubKey", o));
	ret.push_back(Pair("version", coins.nVersion));
	ret.push_back(Pair("coinbase", coins.fCoinBase));

	return ret;
}

Value verifychain(const Array& params, bool fHelp)
{
	if (fHelp || params.size() > 2)
		throw runtime_error(
			"verifychain [check level] [num blocks]\n"
			"Verifies blockchain database.");

	int nCheckLevel = GetArg("-checklevel", 3);
	int nCheckDepth = GetArg("-checkblocks", 288);
	if (params.size() > 0)
		nCheckLevel = params[0].get_int();
	if (params.size() > 1)
		nCheckDepth = params[1].get_int();

	return VerifyDB(nCheckLevel, nCheckDepth);
}

