#include "NewBdd.h"

using namespace std;

namespace NewBdd {

  Man::Man(int nVars, int nVerbose, int nMaxMemLog, int nObjsAllocLog, int nUniqueLog,int nCacheLog, double UniqueDensity) : nVars(nVars), nVerbose(nVerbose) {
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
    vVars.resize(nObjsAlloc);
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
    vVars[0] = VarMax();
    for(int i = 0; i < nVars; i++) {
      UniqueCreateInt(i, 1, 0);
    }
    Var2Level.resize(nVars);
    Level2Var.resize(nVars);
    for(int i = 0; i < nVars; i++) {
      Var2Level[i] = i;
      Level2Var[i] = i;
    }
    nCacheLookups = 0;
    nCacheHits = 0;
    CacheThold = nCache;
    CacheHitRate = 1;
    MinBvarRemoved = BvarMax();
    nGbc = 0;
    nReo = BvarMax();
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
  Man::~Man() {
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

  void Man::SetParameters(int nGbc_, int nReoLog) {
    nGbc = nGbc_;
    if(nReoLog >= 0) {
      nReo = 1 << nReoLog;
      if(!nReo || (size)nReo > (size)BvarMax()) {
        nReo = BvarMax();
      }
    }
    if(nGbc || nReo != BvarMax()) {
      vRefs.resize(nObjsAlloc);
    }
  }

  int Man::GetNumVars() {
    return nVars;
  }
  int Man::GetNumObjs() {
    return nObjs;
  }
  int Man::Var(Node const & x) {
    return Var(x.val);
  }
  int Man::Id(Node const & x) {
    return Lit2Bvar(x.val);
  }
  bool Man::IsCompl(Node const & x) {
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
  Node Man::IthVar(int i) {
    return Node(this, Bvar2Lit(i + 1));
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

  size Man::CountNodes(vector<Node> const & vNodes) {
    size count = 0;
    for(size i = 0; i < vNodes.size(); i++) {
      count += CountNodes_rec(vNodes[i].val);
    }
    for(size i = 0; i < vNodes.size(); i++) {
      ResetMark_rec(vNodes[i].val);
    }
    return count + 1;
  }

}
