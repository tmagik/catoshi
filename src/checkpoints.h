// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2014 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html
#ifndef CODECOIN_CHECKPOINT_H
#define CODECOIN_CHECKPOINT_H

#include <map>
#include <boost/assign/list_of.hpp>  // for 'map_list_of()'

class uint256;
class CBlockIndex;

/** Block-chain checkpoints are compiled-in sanity checks.
 * They are updated every release or three.
 */
namespace Checkpoints
{
	// Returns true if block passes checkpoint checks
	bool CheckBlock(int nHeight, const uint256& hash);

	// Return conservative estimate of total number of blocks, 0 if unknown
	int GetTotalBlocksEstimate();

	// Returns last CBlockIndex* in mapBlockIndex that is a checkpoint
	CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex);

	double GuessVerificationProgress(CBlockIndex *pindex);

	typedef std::map<int, uint256> MapCheckpoints;

	struct CCheckpointData {
		const MapCheckpoints *mapCheckpoints;
		int64_t nTimeLastCheckpoint;
		int64_t nTransactionsLastCheckpoint;
		double fTransactionsPerDay;
	};

	extern const CCheckpointData data;
}

#endif
