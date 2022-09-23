#ifndef NEW_BDD_H
#define NEW_BDD_H

#include <cstddef>
#include <vector>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cassert>

namespace NewBdd {
  
  typedef unsigned short var;
  typedef int bvar;
  typedef unsigned lit;
  typedef unsigned short ref;
  typedef uint64_t size;

  static inline var VarMax() {
    return std::numeric_limits<var>::max();
  }
  static inline bvar BvarMax() {
    return std::numeric_limits<bvar>::max();
  }
  static inline bvar LitMax() {
    return std::numeric_limits<lit>::max();
  }
  static inline ref RefMax() {
    return std::numeric_limits<ref>::max();
  }

  static inline lit Hash(lit Arg0, lit Arg1) {
    return Arg0 + 4256249 * Arg1;
  }
  static inline lit Hash(lit Arg0, lit Arg1, lit Arg2) {
    return 12582917 * Arg0 + Arg1 + 4256249 * Arg2;
  }

  class BddMan {
  private:
    bvar nObjs;
    std::vector<var> vLevels;
    std::vector<lit> vObjs;
    bvar * pNexts;
    std::vector<bool> vMarks;
    std::vector<ref> vRefs;

    std::vector<bvar *> vpUnique;
    std::vector<size> vUniqueMask;

    std::vector<lit> vCache;
    size CacheMask;

    std::vector<var> Var2Level;

    int nVars;
    size nMaxMem;
    size nObjsAlloc;
    size nUnique;
    size nCache;
    double nUniqueDensity;
    int nVerbose;

    lit Bvar2Lit(bvar a) {
      return a << 1;
    }
    lit Bvar2Lit(bvar a, bool c) {
      return (a << 1) ^ (int)c;
    }
    bvar Lit2Bvar(lit x) {
      return x >> 1;
    }
    lit LitRegular(lit x) {
      return x & ~1;
    }
    lit LitIrregular(lit x) {
      return x | 1;
    }

    lit LitNot(lit x) {
      return x ^ 1;
    }
    lit LitNotCond(lit x, bool c) {
      return x ^ (int)c;
    }
    bool LitIsCompl(lit x) {
      return x & 1;
    }
    var Level(lit x) {
      return vLevels[Lit2Bvar(x)];
    }
    lit Then(lit x) {
      return LitNotCond(vObjs[LitRegular(x)], LitIsCompl(x));
    }
    lit Else(lit x) {
      return LitNotCond(vObjs[LitIrregular(x)], LitIsCompl(x));
    }

    var LevelOfBvar(bvar a) {
      return vLevels[a];
    }
    lit ThenOfBvar(bvar a) {
      return vObjs[a << 1];
    }
    lit ElseOfBvar(bvar a) {
      return vObjs[(a << 1) ^ 1];
    }
    void SetLevelOfBvar(bvar a, var i) {
      vLevels[a] = i;
    }
    void SetThenOfBvar(bvar a, lit x) {
      vObjs[a << 1] = x;
    }
    void SetElseOfBvar(bvar a, lit x) {
      vObjs[(a << 1) ^ 1] = x;
    }
 
    lit UniqueCreateInt(var i, lit x1, lit x0);
    lit UniqueCreate(var i, lit x1, lit x0);

    lit And_rec(lit x, lit y);

  public:
    BddMan(int nVars, int nMaxMemLog = 25, int nObjsAllocLog = 20, int nUniqueLog = 10,int nCacheLog = 15, double nUniqueDensity = 4, int nVerbose = 3) : nVars(nVars), nUniqueDensity(nUniqueDensity), nVerbose(nVerbose) {
      if(nVars >= VarMax()) {
        throw std::length_error("Memout (var) in init");
      }
      if(nMaxMemLog > 0) {
        nMaxMem = 1ull << nMaxMemLog;
      } else {
        nMaxMem = (lit)BvarMax() + 1;
      }
      if(!nMaxMem) {
        throw std::length_error("Memout (maxmem) in init");
      }
      nObjsAlloc = 1ull << nObjsAllocLog;
      if(!nObjsAlloc || nObjsAlloc > nMaxMem) {
        throw std::length_error("Memout (node) in init");
      }
      nUnique = 1ull << nUniqueLog;
      if(!nUnique || nUnique > nMaxMem) {
        throw std::length_error("Memout (unique) in init");
      }
      nCache = 1ull << nCacheLog;
      if(!nCache || nCache > nMaxMem) {
        throw std::length_error("Memout (cache) in init");
      }
      while(nObjsAlloc < (size)nVars + 1) {
        nObjsAlloc <<= 1;
        if(!nObjsAlloc || nObjsAlloc > nMaxMem) {
          throw std::length_error("Memout (node) in init");
        }
      }
      while(nUnique < nMaxMem && nUnique * nUniqueDensity < nObjsAlloc / nVars) {
        nUnique <<= 1;
        if(!nUnique) {
          throw std::length_error("Memout (unique) in init");
        }
      }
      if(nVerbose) {
        std::cout << "Allocate " << nObjsAlloc << " nodes, " << nUnique << " unique, and " << nCache << " cache." << std::endl;
      }
      vLevels.resize(nObjsAlloc);
      vObjs.resize(nObjsAlloc * 2);
      pNexts = (bvar *)calloc(nObjsAlloc, sizeof(bvar));
      vMarks.resize(nObjsAlloc);
      vpUnique.resize(nVars);
      vUniqueMask.resize(nVars);
      for(int i = 0; i < nVars; i++) {
        vpUnique[i] = (bvar *)calloc(nUnique, sizeof(bvar));
        vUniqueMask[i] = nUnique - 1;
      }
      vCache.resize(nCache * 3);
      CacheMask = nCache - 1;
      nObjs = 1;
      vLevels[Const()] = VarMax();
      for(int i = 0; i < nVars; i++) {
        UniqueCreateInt(i, Const1(), Const0());
      }
      Var2Level.resize(nVars);
      for(int i = 0; i < nVars; i++) {
        Var2Level[i] = i;
      }
      //   {
      //     var u = std::distance( vOrdering.begin(), std::find( vOrdering.begin(), vOrdering.end(), v ) );
      //     if( u == nVars )
      //       throw "Invalid Ordering";
      //     
      //   }
      // fRef        = 0;
      // nRefresh    = 0;
      // fRealloc    = 0;
      // fGC         = 0;
      // nGC         = 0;
      // fReo        = 0;
      // nReo        = 0;
      // MaxGrowth   = 0;
      // nCacheHit   = 0;
      // nCacheFind  = 0;
      // nCall       = 0;
      // HitRateOld  = 1;
      // nMinRemoved = nObjsAlloc;
      // pRefs       = NULL;
      // pEdges      = NULL;
      // vOrdering.clear();
      // if ( pvOrdering )
      //   {
      //     for ( var v : *pvOrdering )
      //       vOrdering.push_back( v );
      //     if ( vOrdering.size() != nVars )
      //       throw "Wrong number of Variables in the ordering";
      //   }
      // else
      //   for ( var v = 0; v < nVars; v++ )
      //     vOrdering.push_back( v );
    }
    ~BddMan() {
      if(nVerbose) {
        std::cout << "Free " << nObjsAlloc << " nodes (" << nObjs << " live nodes)." << std::endl;
      }
      free(pNexts);
      for(int i = 0; i < nVars; i++) {
        free(vpUnique[i]);
      }
    }
    
    void IncRef(lit a) {}
    void DecRef(lit a) {}

    bvar Const() {
      return 0;
    }
    lit Const0() {
      return 0;
    }
    lit Const1() {
      return 1;
    }

    lit IthVar(int i) {
      return Var2Level[i] + 1;
    }

    lit And(lit x, lit y);
    
  };
  
  class node {
  private:
    BddMan * man;
    lit val;
    
  public:
    node(BddMan * man) : man(man) {}
    node(BddMan * man, int i) : man(man) {
      val = man->IthVar(i);
      man->IncRef(val);
    }
    node() {
      man = NULL;
    }
    node(const node & right) {
      man = right.man;
      val = right.val;
      man->IncRef(val);
    }
    ~node() {
      if(man) {
        man->DecRef(val);
      }
    }
    node & operator=(const node & right) {
      if(this == &right) {
        return *this;
      }
      if(man) {
        man->DecRef(val);
      }
      val = right.val;
      man = right.man;
      man->IncRef(val);
      return *this;
    }
    bool operator==(const node & other) const {
      return val == other.val;
    }
    bool operator!=(const node & other) const {
      return val != other.val;
    }
    node operator&(const node & other) const {
      node x(man);
      x.val = man->And(this->val, other.val);
      man->IncRef(x.val);
      return x;
    }
  };

}

#endif