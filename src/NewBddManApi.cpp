#include <iostream>

#include "NewBddMan.h"

using namespace std;

namespace NewBdd {

  Man::Man(int nVars, bool fCountOnes, int nVerbose, int nMaxMemLog, int nObjsAllocLog, int nUniqueLog,int nCacheLog, double UniqueDensity) : nVars(nVars), nVerbose(nVerbose) {
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
    cache = new Cache(nCache, nMaxMem);
    if(fCountOnes) {
      if(nVars > 1023) {
        throw length_error("Cannot count ones for more than 1023 variables");
      }
      vOneCounts.resize(nObjsAlloc);
    }
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
    MinBvarRemoved = BvarMax();
    SetParameters();
  }
  Man::~Man() {
    if(nVerbose) {
      cout << "Free " << nObjsAlloc << " nodes (" << nObjs << " live nodes) and " << cache->Size() << " cache." << endl;
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
    delete cache;
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

  lit Man::And(lit x, lit y) {
    if(nObjs > nReo) {
      Reorder(fReoVerbose);
      while(nReo < nObjs) {
        nReo <<= 1;
        if((size)nReo > (size)BvarMax()) {
          nReo = BvarMax();
        }
      }
    }
    return And_rec(x, y);
  }

  void Man::SetRef(vector<lit> const & vLits) {
    vRefs.clear();
    vRefs.resize(nObjsAlloc);
    for(size i = 0; i < vLits.size(); i++) {
      IncRef(vLits[i]);
    }
  }

  bool Man::Gbc() {
    if(nVerbose >= 2) {
      cout << "Garbage collect" << endl;
    }
    bvar MinBvarRemovedOld = MinBvarRemoved;
    if(!vEdges.empty()) {
      for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
        if(!EdgeOfBvar(a) && VarOfBvar(a) != VarMax()) {
          RemoveBvar(a);
        }
      }
      return MinBvarRemoved != MinBvarRemovedOld;
    }
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        SetMark_rec(Bvar2Lit(a));
      }
    }
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(!MarkOfBvar(a) && VarOfBvar(a) != VarMax()) {
        RemoveBvar(a);
      }
    }
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        ResetMark_rec(Bvar2Lit(a));
      }
    }
    cache->Clear();
    return MinBvarRemoved != MinBvarRemovedOld;
  }

  void Man::Reorder(bool fVerbose) {
    bool fReoVerbose_ = fReoVerbose;
    fReoVerbose = fVerbose;
    if(nVerbose >= 2) {
      cout << "Reorder" << endl;
    }
    CountEdges();
    Sift();
#ifdef REO_DEBUG
    UncountEdges();
#endif
    vEdges.clear();
    cache->Clear();
    fReoVerbose = fReoVerbose_;
  }
  void Man::GetOrdering(vector<var> & Var2Level_) {
    Var2Level_ = Var2Level;
  }

  bvar Man::CountNodes() {
    bvar count = 0;
    if(!vEdges.empty()) {
      for(bvar a = 1; a < nObjs; a++) {
        if(EdgeOfBvar(a)) {
          count++;
        }
      }
      return count;
    }
    for(bvar a = 1; a <= (bvar)nVars; a++) {
      count++;
      SetMarkOfBvar(a);
    }
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        count += CountNodes_rec(Bvar2Lit(a));
      }
    }
    for(bvar a = 1; a <= (bvar)nVars; a++) {
      ResetMarkOfBvar(a);
    }
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        ResetMark_rec(Bvar2Lit(a));
      }
    }
    return count;
  }
  bvar Man::CountNodes(vector<lit> const & vLits) {
    bvar count = 0;
    for(size i = 0; i < vLits.size(); i++) {
      count += CountNodes_rec(vLits[i]);
    }
    for(size i = 0; i < vLits.size(); i++) {
      ResetMark_rec(vLits[i]);
    }
    return count + 1;
  }

}
