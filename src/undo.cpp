// Copyright (c) 2014-2015 Troy Benjegerdes, under AGPLv3
// Distributed under the Affero GNU General public license version 3
// file COPYING or http://www.gnu.org/licenses/agpl-3.0.html#include "main.h"
#include "main.h"
#include "util.h"
#include "clientversion.h"

/** Undo information for a CBlock */
	bool CBlockUndo::WriteToDisk(CDiskBlockPos &pos, const uint256 &hashBlock)
	{
		// Open history file to append
    CAutoFile fileout(OpenUndoFile(pos), SER_DISK, CLIENT_VERSION);
    if (fileout.IsNull())
        return error("%s : OpenUndoFile failed", __func__);

		// Write index header
		unsigned int nSize = fileout.GetSerializeSize(*this);
    fileout << FLATDATA(Params().MessageStart()) << nSize;

		// Write undo data
    long fileOutPos = ftell(fileout.Get());
		if (fileOutPos < 0)
        return error("%s : ftell failed", __func__);
		pos.nPos = (unsigned int)fileOutPos;
		fileout << *this;

		// calculate & write checksum
		CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
		hasher << hashBlock;
		hasher << *this;
		fileout << hasher.GetHash();

		return true;
	}

	bool CBlockUndo::ReadFromDisk(const CDiskBlockPos &pos, const uint256 &hashBlock)
	{
		// Open history file to read
    CAutoFile filein(OpenUndoFile(pos, true), SER_DISK, CLIENT_VERSION);
    if (filein.IsNull())
        return error("%s : OpenBlockFile failed", __func__);

		// Read block
		uint256 hashChecksum;
		try {
			filein >> *this;
			filein >> hashChecksum;
		}
		catch (std::exception &e) {
        return error("%s : Deserialize or I/O error - %s", __func__, e.what());
		}

		// Verify checksum
		CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
		hasher << hashBlock;
		hasher << *this;
		if (hashChecksum != hasher.GetHash())
        return error("%s : Checksum mismatch: File %d Pos: %u",
		__func__, pos.nFile, pos.nPos);

		return true;
	}

