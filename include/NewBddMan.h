#ifndef NEW_BDD_MAN_H
#define NEW_BDD_MAN_H

#include <vector>

#include "NewBdd.h"

namespace NewBdd {

  class Man {
  public:
    friend class Node;

    Man(int nVars, int nVerbose = 0, int nMaxMemLog = 25, int nObjsAllocLog = 20, int nUniqueLog = 10,int nCacheLog = 15, double UniqueDensity = 4);
    ~Man();

    void SetParameters(int nGbc_ = 0, int nReoLog = -1);
    void SetInitialOrdering(std::vector<int> const & Var2Level_);

    int GetNumVars() const;
    int GetNumObjs() const;

    int Var(Node const & x) const;
    int Id(Node const & x) const;
    bool IsCompl(Node const & x) const;
    Node Then(Node const & x);
    Node Else(Node const & x);
    Node Const0();
    Node Const1();
    Node IthVar(int i);
    Node Not(Node const & x);
    Node NotCond(Node const & x, bool c);
    Node And(Node const & x, Node const & y);

    int Var(NodeNoRef const & x) const;
    int Id(NodeNoRef const & x) const;
    bool IsCompl(NodeNoRef const & x) const;
    NodeNoRef Then(NodeNoRef const & x) const;
    NodeNoRef Else(NodeNoRef const & x) const;

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

    inline lit Hash(lit Arg0, lit Arg1) const;

    inline lit Bvar2Lit(bvar a) const;
    inline lit Bvar2Lit(bvar a, bool c) const;
    inline bvar Lit2Bvar(lit x) const;

    inline lit LitRegular(lit x) const;
    inline lit LitIrregular(lit x) const;
    inline lit LitNot(lit x) const;
    inline lit LitNotCond(lit x, bool c) const;

    inline bool LitIsCompl(lit x) const;
    inline var Var(lit x) const;
    inline var Level(lit x) const;
    inline lit Then(lit x) const;
    inline lit Else(lit x) const;
    inline bool Mark(lit x) const;
    inline ref Ref(lit x) const;
    inline edge Edge(lit x) const;

    inline void SetMark(lit x);
    inline void ResetMark(lit x);
    inline void IncRef(lit x);
    inline void DecRef(lit x);
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

  inline lit Man::Hash(lit Arg0, lit Arg1) const {
    return Arg0 + 4256249 * Arg1;
  }

  inline lit Man::Bvar2Lit(bvar a) const {
    return a << 1;
  }
  inline lit Man::Bvar2Lit(bvar a, bool c) const {
    return (a << 1) ^ (int)c;
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
    return x ^ (int)c;
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

}

#endif
