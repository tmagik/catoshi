// Copyright (c) 2009-2012 The Bitcoin developers, under MIT
// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uintBIG.h"

namespace Checkpoints
{
	typedef std::map<int, uint256> MapCheckpoints;

	// How many times we expect transactions after the last checkpoint to
	// be slower. This number is a compromise, as it can't be accurate for
	// every system. When reindexing from a fast disk with a slow CPU, it
	// can be up to 20, while when downloading from a slow network with a
	// fast multicore CPU, it won't be much higher than 1.
	static const double fSigcheckVerificationFactor = 5.0;

	const CCheckpointData &Checkpoints() {
		/* provided by COIN_brand.cpp */
		assert(data.nTimeLastCheckpoint > 1);
		return data;
	}

	bool CheckBlock(int nHeight, const uint256& hash)
	{
		if (fTestNet) return true; // Testnet has no checkpoints
		if (!GetBoolArg("-checkpoints", true))
			return true;

		const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

		MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
		if (i == checkpoints.end()) return true;
		return hash == i->second;
	}

	// Guess how far we are in the verification process at the given block index
	double GuessVerificationProgress(CBlockIndex *pindex) {
	if (pindex==NULL)
		return 0.0;

		int64_t nNow = time(NULL);

		double fWorkBefore = 0.0; // Amount of work done before pindex
		double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
		// Work is defined as: 1.0 per transaction before the last checkoint, and
		// fSigcheckVerificationFactor per transaction after.

		const CCheckpointData &data = Checkpoints();

		if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
			double nCheapBefore = pindex->nChainTx;
			double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
			double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
			fWorkBefore = nCheapBefore;
			fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
		} else {
			double nCheapBefore = data.nTransactionsLastCheckpoint;
			double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
			double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
			fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
			fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
		}

		return fWorkBefore / (fWorkBefore + fWorkAfter);
	}

	int GetTotalBlocksEstimate()
	{
		if (fTestNet) return 0; // Testnet has no checkpoints
		if (!GetBoolArg("-checkpoints", true))
			return 0;

		const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

		return checkpoints.rbegin()->first;
	}

	CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
	{
		if (fTestNet) return NULL; // Testnet has no checkpoints
		if (!GetBoolArg("-checkpoints", true))
			return NULL;

		const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

		BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
		{
			const uint256& hash = i.second;
			std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
			if (t != mapBlockIndex.end())
				return t->second;
		}
		return NULL;
	}
}
