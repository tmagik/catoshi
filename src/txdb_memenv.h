// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CODECOIN_TXDB_MEMENV_H
#define CODECOIN_TXDB_MEMENV_H

#include "txdb.h"
#include "leveldbwrapper.h"
#include "main.h"
#include <leveldb/env.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

static leveldb::Options GetOptions_mem(size_t nCacheSize)
{
    GetOptions(nCacheSize);
    options.env = leveldb::NewMemEnv(leveldb::Env::Default());
    return options;
}

/* this is not ideal, should make GetOptions a member of the class */
class CLevelDBWrapper_mem : public CLevelDBWrapper
{
public
    CLevelDBWrapper_mem(const boost::filesystem::path& path,
	size_t nCacheSize);
	leveldb::Env* penv = nullptr);
    ~CLevelDBWrapper_mem();
}

CLevelDBWrapper_mem::CLevelDBWrapper_mem(const boost::filesystem::path& path,
	size_t nCacheSize,
	bool fWipe)
{
    readoptions.verify_checksums = true;
    iteroptions.verify_checksums = true;
    iteroptions.fill_cache = false;
    syncoptions.sync = true;
    options = GetOptions(nCacheSize);
    options.create_if_missing = true;
    if (fWipe) {
        LogPrintf("Wiping LevelDB in %s\n", path.string());
        leveldb::DestroyDB(path.string(), options);
    }
    TryCreateDirectory(path);
    LogPrintf("Opening LevelDB in %s\n", path.string());
    leveldb::Status status = leveldb::DB::Open(options, path.string(), &pdb);
    HandleError(status);
    LogPrintf("Opened LevelDB successfully\n");
}

/** CCoinsView backed by the LevelDB coin database with memory environment (chainstate/) */
class CCoinsViewDB_mem : public CCoinsViewDB
{
private:
    CLevelDBWrapper_mem db;
public:
    CCoinsViewDB(size_t nCacheSize, leveldb::Env* penv = nullptr ) : 
	db(GetDataDir() / "chainstate", nCacheSize, penv){
    };
};

/** Access to the block database (blocks/index/) */
class CBlockTreeDB_mem : public CBlockTreeDB
{
private:
    CLevelDBWrapper_mem db;
public:
    CBlockTreeDB(size_t nCacheSize, leveldb::Env* penv = nullptr ) :
	db(GetDataDir() / "blocks" / "index", nCacheSize, penv){
    };
};

#endif // CODECOIN_TXDB_MEMENV_LEVELDB_H
