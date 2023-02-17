#ifndef NEW_BDD_CACHE_H
#define NEW_BDD_CACHE_H

#include <vector>
#include <iostream>

#include "NewBddTypes.h"

namespace NewBdd {

  class Cache {
  private:
    std::vector<lit> vCache;
    lit Mask;
    size nLookups;
    size nHits;
    size nThold;
    double HitRate;
    size nMax;
    int nVerbose;

  public:
    Cache(lit nCache, size nMax, int nVerbose) : Mask(nCache - 1), nLookups(0), nHits(0), nThold(nCache), HitRate(1), nMax(nMax), nVerbose(nVerbose) {
      vCache.resize((size)nCache * 4);
    }
    ~Cache() {
      if(nVerbose) {
        std::cout << "Free " << Size() << " cache." << std::endl;
      }
    }

    inline lit Hash(lit Arg0, lit Arg1, lit Arg2) const {
      return Arg0 + 4256249 * Arg1 + 10000339 * Arg2;
    }
    inline lit Size() const {
      return vCache.size() / 4;
    }

    inline int Lookup(lit x, lit y, lit z) {
      nLookups++;
      if(nLookups > nThold) {
        double NewHitRate = (double)nHits / nLookups;
        if(nVerbose > 2) {
          std::cout << "Cache Hits: " << nHits << " Lookups: " << nLookups << " Rate: " << NewHitRate << std::endl;
        }
        if(NewHitRate > HitRate) {
          Resize();
        } else {
          nThold <<= 1;
          if(!nThold) {
            nThold = SizeMax();
          }
        }
        HitRate = NewHitRate;
      }
      size i = (size)(Hash(x, y, z) & Mask) * 4;
      if(vCache[i] == x && vCache[i + 1] == y && vCache[i + 2] == z) {
        nHits++;
        return vCache[i + 3];
      }
      return -1;
    }
    inline void Insert(lit x, lit y, lit z, int r) {
      size i = (size)(Hash(x, y, z) & Mask) * 4;
      vCache[i] = x;
      vCache[i + 1] = y;
      vCache[i + 2] = z;
      vCache[i + 3] = r;
    }
    inline void Clear() {
      std::fill(vCache.begin(), vCache.end(), 0);
    }

    inline void Resize() {
      lit nCache, nCacheOld;
      nCache = nCacheOld = Size();
      nCache <<= 1;
      if(!nCache || (size)nCache > nMax) {
        nThold = SizeMax();
        return;
      }
      if(nVerbose > 1) {
        std::cout << "Reallocate " << nCache << " cache." << std::endl;
      }
      vCache.resize((size)nCache * 4);
      Mask = nCache - 1;
      for(lit j = 0; j < nCacheOld; j++) {
        size i = (size)j * 4;
        if(vCache[i]) {
          size hash = (size)(Hash(vCache[i], vCache[i + 1], vCache[i + 2]) & Mask) * 4;
          vCache[hash] = vCache[i];
          vCache[hash + 1] = vCache[i + 1];
          vCache[hash + 2] = vCache[i + 2];
          vCache[hash + 3] = vCache[i + 3];
        }
      }
      nThold <<= 1;
      if(!nThold) {
        nThold = SizeMax();
      }
    }
  };

}

#endif
