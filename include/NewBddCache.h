#ifndef NEW_BDD_CACHE_H
#define NEW_BDD_CACHE_H

#include <vector>
#include <iostream>

#include "NewBddTypes.h"

namespace NewBdd {

  class Cache {
  public:
    inline Cache(lit nCache, size nMax, int nVerbose);
    inline ~Cache();
    inline lit Hash(lit Arg0, lit Arg1) const {
      return Arg0 + 4256249 * Arg1;
    }
    inline lit Size() const;
    inline lit Lookup(lit x, lit y);
    inline void Insert(lit x, lit y, lit z);
    inline void Clear();
    inline void Resize();

  private:
    std::vector<lit> vCache;
    lit CacheMask;
    size nCacheLookups;
    size nCacheHits;
    size CacheThold;
    double CacheHitRate;
    size nMax;
    int nVerbose;
  };
  
  inline Cache::Cache(lit nCache, size nMax, int nVerbose) : nMax(nMax), nVerbose(nVerbose) {
    vCache.resize((size)nCache * 3);
    CacheMask = nCache - 1;
    nCacheLookups = 0;
    nCacheHits = 0;
    CacheThold = nCache;
    CacheHitRate = 1;
  }
  inline Cache::~Cache() {
    if(nVerbose) {
      std::cout << "Free " << Size() << " cache." << std::endl;
    }
  }

  inline lit Cache::Size() const {
    return vCache.size() / 3;
  }
  inline lit Cache::Lookup(lit x, lit y) {
    nCacheLookups++;
    if(nCacheLookups > CacheThold) {
      double NewCacheHitRate = (double)nCacheHits / nCacheLookups;
      if(nVerbose > 2) {
        std::cout << "Cache Hits: " << nCacheHits << " Lookups: " << nCacheLookups << " Rate: " << NewCacheHitRate << std::endl;
      }
      if(NewCacheHitRate > CacheHitRate) {
        Resize();
      } else {
        CacheThold <<= 1;
        if(!CacheThold) {
          CacheThold = SizeMax();
        }
      }
      CacheHitRate = NewCacheHitRate;
    }
    size i = (size)(Hash(x, y) & CacheMask) * 3;
    if(vCache[i] == x && vCache[i + 1] == y) {
      nCacheHits++;
      return vCache[i + 2];
    }
    return LitMax();
  }
  inline void Cache::Insert(lit x, lit y, lit z) {
    size i = (size)(Hash(x, y) & CacheMask) * 3;
    vCache[i] = x;
    vCache[i + 1] = y;
    vCache[i + 2] = z;
  }
  inline void Cache::Clear() {
    fill(vCache.begin(), vCache.end(), 0);
  }

  inline void Cache::Resize() {
    lit nCache, nCacheOld;
    nCache = nCacheOld = Size();
    nCache <<= 1;
    if(!nCache || (size)nCache > nMax) {
      CacheThold = SizeMax();
      return;
    }
    if(nVerbose > 1) {
      std::cout << "Reallocate " << nCache << " cache." << std::endl;
    }
    vCache.resize((size)nCache * 3);
    CacheMask = nCache - 1;
    for(lit j = 0; j < nCacheOld; j++) {
      size i = (size)j * 3;
      if(vCache[i] || vCache[i + 1]) {
        size hash = (size)(Hash(vCache[i], vCache[i + 1]) & CacheMask) * 3;
        vCache[hash] = vCache[i];
        vCache[hash + 1] = vCache[i + 1];
        vCache[hash + 2] = vCache[i + 2];
      }
    }
    CacheThold <<= 1;
    if(!CacheThold) {
      CacheThold = SizeMax();
    }
  }

}

#endif
