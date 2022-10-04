#include <iostream>

#include "NewBddMan.h"

using namespace std;

namespace NewBdd {

  Man::Man(int nVars, int nVerbose, int nMaxMemLog, int nObjsAllocLog, int nUniqueLog,int nCacheLog, double UniqueDensity) : nVars(nVars), nVerbose(nVerbose) {
    if(nVars >= (int)VarMax()) {
      throw length_error("Memout (var) in init");
    }
    if(nMaxMemLog > 0) {
      nMaxMem = 1ull << nMaxMemLog;
    } else {
      nMaxMem = (lit)BvarMax() + 1;
    }
    if(!nMaxMem) {
      throw length_error("Memout (maxmem) in init");
    }
    nObjsAlloc = 1 << nObjsAllocLog;
    if((size)nObjsAlloc > (size)BvarMax()) {
      nObjsAlloc = BvarMax();
    }
    if(!nObjsAlloc || (size)nObjsAlloc > nMaxMem) {
      throw length_error("Memout (node) in init");
    }
    lit nUnique = 1 << nUniqueLog;
    if(!nUnique || (size)nUnique > nMaxMem) {
      throw length_error("Memout (unique) in init");
    }
    lit nCache = 1 << nCacheLog;
    if(!nCache || (size)nCache > nMaxMem) {
      throw length_error("Memout (cache) in init");
    }
    while(nObjsAlloc < (bvar)nVars + 1) {
      if(nObjsAlloc == BvarMax()) {
        throw length_error("Memout (node) in init");
      }
      nObjsAlloc <<= 1;
      if((size)nObjsAlloc > (size)BvarMax()) {
        nObjsAlloc = BvarMax();
      }
      if((size)nObjsAlloc > nMaxMem) {
        throw length_error("Memout (node) in init");
      }
    }
    if(nVerbose) {
      cout << "Allocate " << nObjsAlloc << " nodes, " << nUnique << " unique, and " << nCache << " cache." << endl;
    }
    vVars.resize(nObjsAlloc);
    vObjs.resize((size)nObjsAlloc * 2);
    vNexts.resize(nObjsAlloc);
    vMarks.resize(nObjsAlloc);
    vvUnique.resize(nVars);
    vUniqueMasks.resize(nVars);
    vUniqueCounts.resize(nVars);
    vUniqueTholds.resize(nVars);
    for(var v = 0; v < nVars; v++) {
      vvUnique[v].resize(nUnique);
      vUniqueMasks[v] = nUnique - 1;
      if(nUnique * UniqueDensity > (double)BvarMax()) {
        vUniqueTholds[v] = BvarMax();
      } else {
        vUniqueTholds[v] = nUnique * UniqueDensity;
      }
    }
    vCache.resize((size)nCache * 3);
    CacheMask = nCache - 1;
    nObjs = 1;
    vVars[0] = VarMax();
    for(var v = 0; v < nVars; v++) {
      UniqueCreateInt(v, 1, 0);
    }
    Var2Level.resize(nVars);
    Level2Var.resize(nVars);
    for(var v = 0; v < nVars; v++) {
      Var2Level[v] = v;
      Level2Var[v] = v;
    }
    nCacheLookups = 0;
    nCacheHits = 0;
    CacheThold = nCache;
    CacheHitRate = 1;
    MinBvarRemoved = BvarMax();
    SetParameters();
  }
  Man::~Man() {
    if(nVerbose) {
      cout << "Free " << nObjsAlloc << " nodes (" << nObjs << " live nodes) and " << vCache.size() / 3 << " cache." << endl;
      cout << "Free {";
      string delim;
      for(var v = 0; v < nVars; v++) {
        cout << delim << vvUnique[v].size();
        delim = ", ";
      }
      cout << "} unique." << endl;
      if(!vRefs.empty()) {
        cout << "Free " << vRefs.size() << " refs" << endl;
      }
    }
  }

  void Man::SetParameters(int nGbc_, int nReoLog, double MaxGrowth_, bool fReoVerbose_) {
    nGbc = nGbc_;
    if(nReoLog >= 0) {
      nReo = 1 << nReoLog;
      if(!nReo || (size)nReo > (size)BvarMax()) {
        nReo = BvarMax();
      }
    } else {
      nReo = BvarMax();
    }
    MaxGrowth = MaxGrowth_;
    fReoVerbose = fReoVerbose_;
    if(nGbc || nReo != BvarMax()) {
      vRefs.resize(nObjsAlloc);
    } else {
      vRefs.clear();
    }
  }
  void Man::SetInitialOrdering(vector<var> const & Var2Level_) {
    Var2Level = Var2Level_;
    for(var v = 0; v < nVars; v++) {
      Level2Var[Var2Level[v]] = v;
    }
  }

  var Man::GetNumVars() const {
    return nVars;
  }
  bvar Man::GetNumObjs() const {
    return nObjs;
  }

  var Man::Var(Node const & x) const {
    return Var(x.val);
  }
  bvar Man::Id(Node const & x) const {
    return Lit2Bvar(x.val);
  }
  bool Man::IsCompl(Node const & x) const {
    return LitIsCompl(x.val);
  }
  Node Man::Then(Node const & x) {
    return Node(this, Then(x.val));
  }
  Node Man::Else(Node const & x) {
    return Node(this, Else(x.val));
  }
  Node Man::Const0() {
    return Node(this, 0);
  }
  Node Man::Const1() {
    return Node(this, 1);
  }
  Node Man::IthVar(var v) {
    return Node(this, Bvar2Lit((bvar)v + 1));
  }
  Node Man::Not(Node const & x) {
    return Node(this, LitNot(x.val));
  }
  Node Man::NotCond(Node const & x, bool c) {
    return c? Not(x): x;
  }
  Node Man::And(Node const & x, Node const & y) {
    return Node(this, And(x.val, y.val));
  }
  Node Man::Or(Node const & x, Node const & y) {
    return Node(this, LitNot(And(LitNot(x.val), LitNot(y.val))));
  }

  var Man::Var(NodeNoRef const & x) const {
    return Var(x.val);
  }
  bvar Man::Id(NodeNoRef const & x) const {
    return Lit2Bvar(x.val);
  }
  bool Man::IsCompl(NodeNoRef const & x) const {
    return LitIsCompl(x.val);
  }
  NodeNoRef Man::Then(NodeNoRef const & x) const {
    return NodeNoRef(Then(x.val));
  }
  NodeNoRef Man::Else(NodeNoRef const & x) const {
    return NodeNoRef(Else(x.val));
  }

  void Man::SetRef(vector<Node> const & vNodes) {
    vRefs.clear();
    vRefs.resize(nObjsAlloc);
    for(size i = 0; i < vNodes.size(); i++) {
      IncRef(vNodes[i].val);
    }
  }

  void Man::Reorder(bool fVerbose) {
    bool fReoVerbose_ = fReoVerbose;
    fReoVerbose |= fVerbose;
    Reo();
    fReoVerbose = fReoVerbose_;
  }
  void Man::GetOrdering(vector<var> & Var2Level_) {
    Var2Level_ = Var2Level;
  }

  bvar Man::CountNodes(vector<Node> const & vNodes) {
    bvar count = 0;
    for(size i = 0; i < vNodes.size(); i++) {
      count += CountNodes_rec(vNodes[i].val);
    }
    for(size i = 0; i < vNodes.size(); i++) {
      ResetMark_rec(vNodes[i].val);
    }
    return count + 1;
  }

}
