// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_SUPPORT_PAGELOCKER_H
#define BITCOIN_SUPPORT_PAGELOCKER_H

#include "support/cleanse.h"

#include <map>

#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>

/**
 * Thread-safe class to keep track of locked (ie, non-swappable) memory pages.
 *
 * Memory locks do not stack, that is, pages which have been locked several times by calls to mlock()
 * will be unlocked by a single call to munlock(). This can result in keying material ending up in swap when
 * those functions are used naively. This class simulates stacking memory locks by keeping a counter per page.
 *
 * @note By using a map from each page base address to lock count, this class is optimized for
 * small objects that span up to a few pages, mostly smaller than a page. To support large allocations,
 * something like an interval tree would be the preferred data structure.
 */
template <class Locker>
class LockedPageManagerBase
{
public:
    LockedPageManagerBase(size_t page_size) : page_size(page_size)
    {
        // Determine bitmask for extracting page from address
        assert(!(page_size & (page_size - 1))); // size must be power of two
        page_mask = ~(page_size - 1);
    }

    ~LockedPageManagerBase()
    {
    }


    // For all pages in affected range, increase lock count
    void LockRange(void* p, size_t size)
    {
        boost::mutex::scoped_lock lock(mutex);
        if (!size)
            return;
        const size_t base_addr = reinterpret_cast<size_t>(p);
        const size_t start_page = base_addr & page_mask;
        const size_t end_page = (base_addr + size - 1) & page_mask;
        for (size_t page = start_page; page <= end_page; page += page_size) {
            Histogram::iterator it = histogram.find(page);
            if (it == histogram.end()) // Newly locked page
            {
                locker.Lock(reinterpret_cast<void*>(page), page_size);
                histogram.insert(std::make_pair(page, 1));
            } else // Page was already locked; increase counter
            {
                it->second += 1;
            }
        }
    }

    // For all pages in affected range, decrease lock count
    void UnlockRange(void* p, size_t size)
    {
        boost::mutex::scoped_lock lock(mutex);
        if (!size)
            return;
        const size_t base_addr = reinterpret_cast<size_t>(p);
        const size_t start_page = base_addr & page_mask;
        const size_t end_page = (base_addr + size - 1) & page_mask;
        for (size_t page = start_page; page <= end_page; page += page_size) {
            Histogram::iterator it = histogram.find(page);
            assert(it != histogram.end()); // Cannot unlock an area that was not locked
            // Decrease counter for page, when it is zero, the page will be unlocked
            it->second -= 1;
            if (it->second == 0) // Nothing on the page anymore that keeps it locked
            {
                // Unlock page and remove the count from histogram
                locker.Unlock(reinterpret_cast<void*>(page), page_size);
                histogram.erase(it);
            }
        }
    }

    // Get number of locked pages for diagnostics
    int GetLockedPageCount()
    {
        boost::mutex::scoped_lock lock(mutex);
        return histogram.size();
    }

private:
    Locker locker;
    boost::mutex mutex;
    size_t page_size, page_mask;
    // map of page base address to lock count
    typedef std::map<size_t, int> Histogram;
    Histogram histogram;
};


/**
 * OS-dependent memory page locking/unlocking.
 * Defined as policy class to make stubbing for test possible.
 */
class MemoryPageLocker
{
public:
    /** Lock memory pages.
     * addr and len must be a multiple of the system page size
     */
    bool Lock(const void* addr, size_t len);
    /** Unlock memory pages.
     * addr and len must be a multiple of the system page size
     */
    bool Unlock(const void* addr, size_t len);
};

/**
 * Singleton class to keep track of locked (ie, non-swappable) memory pages, for use in
 * std::allocator templates.
 *
 * Some implementations of the STL allocate memory in some constructors (i.e., see
 * MSVC's vector<T> implementation where it allocates 1 byte of memory in the allocator.)
 * Due to the unpredictable order of static initializers, we have to make sure the
 * LockedPageManager instance exists before any other STL-based objects that use
 * secure_allocator are created. So instead of having LockedPageManager also be
 * static-initialized, it is created on demand.
 */
class LockedPageManager : public LockedPageManagerBase<MemoryPageLocker>
{
public:
    static LockedPageManager& Instance()
    {
        boost::call_once(LockedPageManager::CreateInstance, LockedPageManager::init_flag);
        return *LockedPageManager::_instance;
    }

private:
    LockedPageManager();

    static void CreateInstance()
    {
        // Using a local static instance guarantees that the object is initialized
        // when it's first needed and also deinitialized after all objects that use
        // it are done with it.  I can think of one unlikely scenario where we may
        // have a static deinitialization order/problem, but the check in
        // LockedPageManagerBase's destructor helps us detect if that ever happens.
        static LockedPageManager instance;
        LockedPageManager::_instance = &instance;
    }

    static LockedPageManager* _instance;
    static boost::once_flag init_flag;
};

//
// Functions for directly locking/unlocking memory objects.
// Intended for non-dynamically allocated structures.
//
template <typename T>
void LockObject(const T& t)
{
    LockedPageManager::Instance().LockRange((void*)(&t), sizeof(T));
}

template <typename T>
void UnlockObject(const T& t)
{
    memory_cleanse((void*)(&t), sizeof(T));
    LockedPageManager::Instance().UnlockRange((void*)(&t), sizeof(T));
}

#endif // BITCOIN_SUPPORT_PAGELOCKER_H
