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
  typedef unsigned edge;

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

  class Node;

  class Man {
  public:
    friend class Node;

    Man(int nVars, int nVerbose = 0, int nMaxMemLog = 25, int nObjsAllocLog = 20, int nUniqueLog = 10,int nCacheLog = 15, double UniqueDensity = 4);
    ~Man();

    void SetParameters(int nGbc_ = 0, int nReoLog = -1);

    int GetNumVars();
    int GetNumObjs();
    int Var(Node const & x);
    int Id(Node const & x);
    bool IsCompl(Node const & x);
    Node Then(Node const & x);
    Node Else(Node const & x);
    Node Const0();
    Node Const1();
    Node IthVar(int i);
    Node Not(Node const & x);
    Node NotCond(Node const & x, bool c);
    Node And(Node const & x, Node const & y);

    size CountNodes(std::vector<Node> const & vNodes);

  private:
    int nVars;
    bvar nObjs;
    bvar nObjsAlloc;
    bvar MinBvarRemoved;
    std::vector<var> vVars;
    std::vector<lit> vObjs;
    std::vector<bvar> vNexts;
    std::vector<bool> vMarks;
    std::vector<ref> vRefs;
    std::vector<edge> vEdges;

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
    std::vector<var> Var2Level;
    std::vector<var> Level2Var;

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
    var Var(lit x) {
      return vVars[Lit2Bvar(x)];
    }
    var Level(lit x) {
      return Var2Level[Var(x)];
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
    ref Ref(lit x) {
      return vRefs[Lit2Bvar(x)];
    }
    edge Edge(lit x) {
      return vEdges[Lit2Bvar(x)];
    }

    void SetMark(lit x) {
      vMarks[Lit2Bvar(x)] = true;
    }
    void ResetMark(lit x) {
      vMarks[Lit2Bvar(x)] = false;
    }

    void IncRef(lit x) {
      if(!vRefs.empty() && Ref(x) != RefMax()) {
        vRefs[Lit2Bvar(x)]++;
      }
    }
    void DecRef(lit x) {
      if(!vRefs.empty() && Ref(x) != RefMax()) {
        vRefs[Lit2Bvar(x)]--;
      }
    }
    void IncEdge(lit x) {
      vEdges[Lit2Bvar(x)]++;
      //std::cout << "incedge " << Lit2Bvar(x) << std::endl;
    }
    void DecEdge(lit x) {
      vEdges[Lit2Bvar(x)]--;
      //std::cout << "decedge " << Lit2Bvar(x) << std::endl;
    }

    var VarOfBvar(bvar a) {
      return vVars[a];
    }
    lit ThenOfBvar(bvar a) {
      return vObjs[a << 1];
    }
    lit ElseOfBvar(bvar a) {
      return vObjs[(a << 1) ^ 1];
    }
    bool MarkOfBvar(bvar a) {
      return vMarks[a];
    }
    ref RefOfBvar(bvar a) {
      return vRefs[a];
    }
    edge EdgeOfBvar(bvar a) {
      return vEdges[a];
    }
    void SetVarOfBvar(bvar a, var v) {
      vVars[a] = v;
    }
    void SetThenOfBvar(bvar a, lit x) {
      vObjs[a << 1] = x;
    }
    void SetElseOfBvar(bvar a, lit x) {
      vObjs[(a << 1) ^ 1] = x;
    }
 
    void SetMark_rec(lit x);
    void ResetMark_rec(lit x);

    void CountEdges_rec(lit x);
    void CountEdges();
    void UncountEdges_rec(lit x);
    void UncountEdges();

    lit UniqueCreateInt(var v, lit x1, lit x0);
    lit UniqueCreate(var v, lit x1, lit x0);

    lit CacheLookup(lit x, lit y);
    void CacheInsert(lit x, lit y, lit z);

    lit And_rec(lit x, lit y);
    lit And(lit x, lit y);

    bool Resize();
    void ResizeUnique(var v);
    void ResizeCache();

    void CacheClear();
    void RemoveBvar(bvar a);
    bool Gbc();

    bvar Swap(var i);
    void Sift();

    void CheckTholds();

    size CountNodes_rec(lit x);

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
