#ifndef NEW_BDD_H
#define NEW_BDD_H

#include <cstddef>
#include <vector>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <string>

namespace NewBdd {
  
  typedef unsigned short var;
  typedef int bvar;
  typedef unsigned lit;
  typedef unsigned short ref;
  typedef unsigned long long size;

  static inline var VarMax() {
    return std::numeric_limits<var>::max();
  }
  static inline bvar BvarMax() {
    return std::numeric_limits<bvar>::max();
  }
  static inline lit LitMax() {
    return std::numeric_limits<lit>::max();
  }
  static inline ref RefMax() {
    return std::numeric_limits<ref>::max();
  }
  static inline size SizeMax() {
    return std::numeric_limits<size>::max();
  }

  static inline lit Hash(lit Arg0, lit Arg1) {
    return Arg0 + 4256249 * Arg1;
  }
  static inline lit Hash(lit Arg0, lit Arg1, lit Arg2) {
    return 12582917 * Arg0 + Arg1 + 4256249 * Arg2;
  }

  class Node;

  class Man {
    friend class Node;

  private:
    int nVars;
    bvar nObjs;
    bvar nObjsAlloc;
    std::vector<var> vLevels;
    std::vector<lit> vObjs;
    std::vector<bvar> vNexts;
    std::vector<bool> vMarks;
    std::vector<ref> vRefs;

    std::vector<std::vector<bvar> > vvUnique;
    std::vector<lit> vUniqueMasks;
    std::vector<bvar> vUniqueCounts;
    std::vector<bvar> vUniqueTholds;

    std::vector<lit> vCache;
    lit CacheMask;
    size nCacheLookups;
    size nCacheHits;
    size CacheThold;
    double CacheHitRate;

    std::vector<var> Var2Level;

    size nMaxMem;
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
    bool Mark(lit x) {
      return vMarks[Lit2Bvar(x)];
    }

    void SetMark(lit x) {
      vMarks[Lit2Bvar(x)] = true;
    }
    void ResetMark(lit x) {
      vMarks[Lit2Bvar(x)] = false;
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

    void IncRef(lit a) {}
    void DecRef(lit a) {}

    void SetMark_rec(lit x);
    void ResetMark_rec(lit x);

    lit CacheLookup(lit x, lit y);
    void CacheInsert(lit x, lit y, lit z);

    lit And_rec(lit x, lit y);
    lit And(lit x, lit y);

    void Resize();
    void ResizeUnique(int i);
    void ResizeCache();
    void Refresh();

    size CountNodes_rec(lit x);

  public:
    Man(int nVars, int nVerbose = 0, int nMaxMemLog = 25, int nObjsAllocLog = 20, int nUniqueLog = 10,int nCacheLog = 15, double UniqueDensity = 4) : nVars(nVars), nVerbose(nVerbose) {
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
      nObjsAlloc = 1 << nObjsAllocLog;
      if((size)nObjsAlloc > (size)BvarMax()) {
        nObjsAlloc = BvarMax();
      }
      if(!nObjsAlloc || (size)nObjsAlloc > nMaxMem) {
        throw std::length_error("Memout (node) in init");
      }
      lit nUnique = 1 << nUniqueLog;
      if(!nUnique || (size)nUnique > nMaxMem) {
        throw std::length_error("Memout (unique) in init");
      }
      lit nCache = 1 << nCacheLog;
      if(!nCache || (size)nCache > nMaxMem) {
        throw std::length_error("Memout (cache) in init");
      }
      while(nObjsAlloc < nVars + 1) {
        if(nObjsAlloc == BvarMax()) {
          throw std::length_error("Memout (node) in init");
        }
        nObjsAlloc <<= 1;
        if((size)nObjsAlloc > (size)BvarMax()) {
          nObjsAlloc = BvarMax();
        }
        if((size)nObjsAlloc > nMaxMem) {
          throw std::length_error("Memout (node) in init");
        }
      }
      if(nVerbose) {
        std::cout << "Allocate " << nObjsAlloc << " nodes, " << nUnique << " unique, and " << nCache << " cache." << std::endl;
      }
      vLevels.resize(nObjsAlloc);
      vObjs.resize((size)nObjsAlloc * 2);
      vNexts.resize(nObjsAlloc);
      vMarks.resize(nObjsAlloc);
      vvUnique.resize(nVars);
      vUniqueMasks.resize(nVars);
      vUniqueCounts.resize(nVars);
      vUniqueTholds.resize(nVars);
      for(int i = 0; i < nVars; i++) {
        vvUnique[i].resize(nUnique);
        vUniqueMasks[i] = nUnique - 1;
        if(nUnique * UniqueDensity > (double)BvarMax()) {
          vUniqueTholds[i] = BvarMax();
        } else {
          vUniqueTholds[i] = nUnique * UniqueDensity;
        }
      }
      vCache.resize((size)nCache * 3);
      CacheMask = nCache - 1;
      nObjs = 1;
      vLevels[0] = VarMax();
      for(int i = 0; i < nVars; i++) {
        UniqueCreateInt(i, 1, 0);
      }
      Var2Level.resize(nVars);
      for(int i = 0; i < nVars; i++) {
        Var2Level[i] = i;
      }
      nCacheLookups = 0;
      nCacheHits = 0;
      CacheThold = nCache;
      CacheHitRate = 1;
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
    ~Man() {
      if(nVerbose) {
        std::cout << "Free " << nObjsAlloc << " nodes (" << nObjs << " live nodes) and " << vCache.size() / 3 << " cache." << std::endl;
        std::cout << "Free {";
        std::string delim;
        for(int i = 0; i < nVars; i++) {
          std::cout << delim << vvUnique[i].size();
          delim = ", ";
        }
        std::cout << "} unique." << std::endl;
      }
    }
    
    Node Const0();
    Node Const1();
    Node IthVar(int i);
    Node Not(Node const & x);
    Node NotCond(Node const & x, bool c);
    Node And(Node const & x, Node const & y);

    size CountNodes(std::vector<Node> const & vNodes);
    
  };
  
  class Node {
    friend class Man;

  private:
    Man * man;
    lit val;
    
  public:
    Node(Man * man, lit val) : man(man), val(val) {
      man->IncRef(val);
    }
    Node() {
      man = NULL;
    }
    Node(const Node & right) {
      man = right.man;
      val = right.val;
      man->IncRef(val);
    }
    ~Node() {
      if(man) {
        man->DecRef(val);
      }
    }
    Node & operator=(const Node & right) {
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
    bool operator==(const Node & other) const {
      return val == other.val;
    }
    bool operator!=(const Node & other) const {
      return val != other.val;
    }
  };

}

#endif
