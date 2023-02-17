#ifndef NEW_BDD_MAN_H
#define NEW_BDD_MAN_H

#include <vector>
#include <cmath>
#include <iostream>

#include "NewBddTypes.h"

namespace NewBdd {

  class Man {
  public:
    Man(int nVars, bool fCountOnes = false, int nVerbose = 0, int nMaxMemLog = 25, int nObjsAllocLog = 20, int nUniqueLog = 10,int nCacheLog = 15, double UniqueDensity = 4);
    ~Man();

    void SetParameters(int nGbc_ = 0, int nReoLog = -1, double MaxGrowth_ = 1.2, bool fReoVerbose_ = false);
    void SetInitialOrdering(std::vector<var> const & Var2Level_);

    var GetNumVars() const;
    bvar GetNumObjs() const;

    lit And(lit x, lit y);

    void SetRef(std::vector<lit> const & vLits);

    bool Gbc();

    void Reorder(bool fVerbose = false);
    void GetOrdering(std::vector<var> & Var2Level_);

    bvar CountNodes();
    bvar CountNodes(std::vector<lit> const & vLits);

    inline lit Const0() const;
    inline lit Const1() const;
    inline lit IthVar(var v) const;

    inline bvar Lit2Bvar(lit x) const;
    inline lit LitNot(lit x) const;
    inline lit LitNotCond(lit x, bool c) const;
    inline bool LitIsCompl(lit x) const;
    inline var Var(lit x) const;
    inline lit Then(lit x) const;
    inline lit Else(lit x) const;

    inline double OneCount(lit x) const;

    inline void IncRef(lit x);
    inline void DecRef(lit x);

  private:
    var nVars;
    bvar nObjs;
    bvar nObjsAlloc;
    bvar RemovedHead;
    std::vector<var> vVars;
    std::vector<lit> vObjs;
    std::vector<bvar> vNexts;
    std::vector<bool> vMarks;
    std::vector<ref> vRefs;
    std::vector<edge> vEdges;

    std::vector<double> vOneCounts;

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

    int nGbc;

    bvar nReo;
    double MaxGrowth;
    bool fReoVerbose;
    std::vector<var> Var2Level;
    std::vector<var> Level2Var;

    size nMaxMem;
    int nVerbose;

    inline lit Hash(lit Arg0, lit Arg1) const;

    inline lit Bvar2Lit(bvar a) const;
    inline lit Bvar2Lit(bvar a, bool c) const;

    inline lit LitRegular(lit x) const;
    inline lit LitIrregular(lit x) const;

    inline var Level(lit x) const;
    inline bool Mark(lit x) const;
    inline ref Ref(lit x) const;
    inline edge Edge(lit x) const;

    inline void SetMark(lit x);
    inline void ResetMark(lit x);
    inline void IncEdge(lit x);
    inline void DecEdge(lit x);

    inline var VarOfBvar(bvar a) const;
    inline lit ThenOfBvar(bvar a) const;
    inline lit ElseOfBvar(bvar a) const;
    inline bool MarkOfBvar(bvar a) const;
    inline ref RefOfBvar(bvar a) const;
    inline edge EdgeOfBvar(bvar a) const;

    inline void SetVarOfBvar(bvar a, var v);
    inline void SetThenOfBvar(bvar a, lit x);
    inline void SetElseOfBvar(bvar a, lit x);
    inline void SetMarkOfBvar(bvar a);
    inline void ResetMarkOfBvar(bvar a);
 
    void SetMark_rec(lit x);
    void ResetMark_rec(lit x);

    void CountEdges_rec(lit x);
    void CountEdges();

#ifdef REO_DEBUG
    void UncountEdges_rec(lit x);
    void UncountEdges();
#endif

    inline lit UniqueCreateInt(var v, lit x1, lit x0);
    inline lit UniqueCreate(var v, lit x1, lit x0);

    inline lit CacheLookup(lit x, lit y);
    inline void CacheInsert(lit x, lit y, lit z);
    inline void CacheClear();

    inline void RemoveBvar(bvar a);

    lit And_rec(lit x, lit y);

    bool Resize();
    void ResizeUnique(var v);
    void ResizeCache();

    bvar Swap(var i);
    void Sift();

    bvar CountNodes_rec(lit x);
  };

  inline lit Man::Const0() const {
    return 0;
  }
  inline lit Man::Const1() const {
    return 1;
  }
  inline lit Man::IthVar(var v) const {
    return Bvar2Lit((bvar)v + 1);
  }

  inline lit Man::Hash(lit Arg0, lit Arg1) const {
    return Arg0 + 4256249 * Arg1;
  }

  inline lit Man::Bvar2Lit(bvar a) const {
    return a << 1;
  }
  inline lit Man::Bvar2Lit(bvar a, bool c) const {
    return (a << 1) ^ (lit)c;
  }
  inline bvar Man::Lit2Bvar(lit x) const {
    return x >> 1;
  }

  inline lit Man::LitRegular(lit x) const {
    return x & ~1;
  }
  inline lit Man::LitIrregular(lit x) const {
    return x | 1;
  }
  inline lit Man::LitNot(lit x) const {
    return x ^ 1;
  }
  inline lit Man::LitNotCond(lit x, bool c) const {
    return x ^ (lit)c;
  }

  inline bool Man::LitIsCompl(lit x) const {
    return x & 1;
  }
  inline var Man::Var(lit x) const {
    return vVars[Lit2Bvar(x)];
  }
  inline var Man::Level(lit x) const {
    return Var2Level[Var(x)];
  }
  inline lit Man::Then(lit x) const {
    return LitNotCond(vObjs[LitRegular(x)], LitIsCompl(x));
  }
  inline lit Man::Else(lit x) const {
    return LitNotCond(vObjs[LitIrregular(x)], LitIsCompl(x));
  }
  inline bool Man::Mark(lit x) const {
    return vMarks[Lit2Bvar(x)];
  }
  inline ref Man::Ref(lit x) const {
    return vRefs[Lit2Bvar(x)];
  }
  inline edge Man::Edge(lit x) const {
    return vEdges[Lit2Bvar(x)];
  }

  inline void Man::SetMark(lit x) {
    vMarks[Lit2Bvar(x)] = true;
  }
  inline void Man::ResetMark(lit x) {
    vMarks[Lit2Bvar(x)] = false;
  }
  inline void Man::IncRef(lit x) {
    if(!vRefs.empty() && Ref(x) != RefMax()) {
      vRefs[Lit2Bvar(x)]++;
    }
  }
  inline void Man::DecRef(lit x) {
    if(!vRefs.empty() && Ref(x) != RefMax()) {
      vRefs[Lit2Bvar(x)]--;
    }
  }
  inline void Man::IncEdge(lit x) {
    vEdges[Lit2Bvar(x)]++;
  }
  inline void Man::DecEdge(lit x) {
    vEdges[Lit2Bvar(x)]--;
  }

  inline var Man::VarOfBvar(bvar a) const {
    return vVars[a];
  }
  inline lit Man::ThenOfBvar(bvar a) const {
    return vObjs[a << 1];
  }
  inline lit Man::ElseOfBvar(bvar a) const {
    return vObjs[(a << 1) ^ 1];
  }
  inline bool Man::MarkOfBvar(bvar a) const {
    return vMarks[a];
  }
  inline ref Man::RefOfBvar(bvar a) const {
    return vRefs[a];
  }
  inline edge Man::EdgeOfBvar(bvar a) const {
    return vEdges[a];
  }

  inline void Man::SetVarOfBvar(bvar a, var v) {
    vVars[a] = v;
  }
  inline void Man::SetThenOfBvar(bvar a, lit x) {
    vObjs[a << 1] = x;
  }
  inline void Man::SetElseOfBvar(bvar a, lit x) {
    vObjs[(a << 1) ^ 1] = x;
  }
  inline void Man::SetMarkOfBvar(bvar a) {
    vMarks[a] = true;
  }
  inline void Man::ResetMarkOfBvar(bvar a) {
    vMarks[a] = false;
  }

  inline double Man::OneCount(lit x) const {
    if(vOneCounts.empty()) {
      throw std::logic_error("Counting ones was not turned on");
    }
    if(LitIsCompl(x)) {
      return std::pow(2.0, nVars) - vOneCounts[Lit2Bvar(x)];
    }
    return vOneCounts[Lit2Bvar(x)];
  }

  inline lit Man::UniqueCreateInt(var v, lit x1, lit x0) {
    std::vector<bvar>::iterator p, q;
    p = q = vvUnique[v].begin() + (Hash(x1, x0) & vUniqueMasks[v]);
    for(; *q; q = vNexts.begin() + *q) {
      if(VarOfBvar(*q) == v && ThenOfBvar(*q) == x1 && ElseOfBvar(*q) == x0) {
        return Bvar2Lit(*q);
      }
    }
    bvar next = *p;
    if(nObjs < nObjsAlloc) {
      *p = nObjs++;
    } else if(RemovedHead) {
      *p = RemovedHead;
      RemovedHead = vNexts[*p];
    } else {
      return LitMax();
    }
    SetVarOfBvar(*p, v);
    SetThenOfBvar(*p, x1);
    SetElseOfBvar(*p, x0);
    vNexts[*p] = next;
    if(!vOneCounts.empty()) {
      vOneCounts[*p] = OneCount(x1) / 2 + OneCount(x0) / 2;
    }
    if(nVerbose >= 3) {
      std::cout << "Create node " << *p << " : Var = " << v << " Then = " << x1 << " Else = " << x0;
      if(!vOneCounts.empty()) {
        std::cout << " Ones = " << vOneCounts[*q];
      }
      std::cout << std::endl;
    }
    vUniqueCounts[v]++;
    if(vUniqueCounts[v] > vUniqueTholds[v]) {
      bvar a = *p;
      ResizeUnique(v);
      return Bvar2Lit(a);
    }
    return Bvar2Lit(*p);
  }
  inline lit Man::UniqueCreate(var v, lit x1, lit x0) {
    if(x1 == x0) {
      return x1;
    }
    lit x;
    while(true) {
      if(!LitIsCompl(x0)) {
        x = UniqueCreateInt(v, x1, x0);
      } else {
        x = LitNot(UniqueCreateInt(v, LitNot(x1), LitNot(x0)));
      }
      if((x | 1) == LitMax()) {
        bool fRemoved = false;
        if(nGbc > 1) {
          fRemoved = Gbc();
        }
        if(!Resize() && !fRemoved && (nGbc != 1 || !Gbc())) {
          throw std::length_error("Memout (node)");
        }
      } else {
        break;
      }
    }
    return x;
  }

  inline lit Man::CacheLookup(lit x, lit y) {
    nCacheLookups++;
    if(nCacheLookups > CacheThold) {
      double NewCacheHitRate = (double)nCacheHits / nCacheLookups;
      if(NewCacheHitRate > CacheHitRate) {
        ResizeCache();
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
  inline void Man::CacheInsert(lit x, lit y, lit z) {
    size i = (size)(Hash(x, y) & CacheMask) * 3;
    vCache[i] = x;
    vCache[i + 1] = y;
    vCache[i + 2] = z;
  }
  inline void Man::CacheClear() {
    fill(vCache.begin(), vCache.end(), 0);
  }

  inline void Man::RemoveBvar(bvar a) {
    var v = VarOfBvar(a);
    SetVarOfBvar(a, VarMax());
    std::vector<bvar>::iterator q = vvUnique[v].begin() + (Hash(ThenOfBvar(a), ElseOfBvar(a)) & vUniqueMasks[v]);
    for(; *q; q = vNexts.begin() + *q) {
      if(*q == a) {
        break;
      }
    }
    bvar next = vNexts[*q];
    vNexts[*q] = RemovedHead;
    RemovedHead = *q;
    *q = next;
    vUniqueCounts[v]--;
  }

}

#endif
