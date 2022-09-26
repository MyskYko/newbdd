#include <iostream>

#include "NewBddMan.h"

using namespace std;

namespace NewBdd {

  void Man::SetMark_rec(lit x) {
    if(x < 2 || Mark(x)) {
      return;
    }
    SetMark(x);
    SetMark_rec(Else(x));
    SetMark_rec(Then(x));
  }
  void Man::ResetMark_rec(lit x) {
    if(x < 2 || !Mark(x)) {
      return;
    }
    ResetMark(x);
    ResetMark_rec(Else(x));
    ResetMark_rec(Then(x));
  }

  void Man::CountEdges_rec(lit x) {
    if(x < 2) {
      return;
    }
    IncEdge(x);
    if(Mark(x)) {
      return;
    }
    SetMark(x);
    CountEdges_rec(Else(x));
    CountEdges_rec(Then(x));
  }
  void Man::CountEdges() {
    vEdges.resize(nObjsAlloc);
    for(bvar a = nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        CountEdges_rec(Bvar2Lit(a));
      }
    }
    for(int i = 0; i < nVars; i++) {
      vEdges[i + 1]++;
    }
    for(bvar a = nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        ResetMark_rec(Bvar2Lit(a));
      }
    }
  }

  void Man::UncountEdges_rec(lit x) {
    if(x < 2) {
      return;
    }
    DecEdge(x);
    if(Mark(x)) {
      return;
    }
    SetMark(x);
    UncountEdges_rec(Else(x));
    UncountEdges_rec(Then(x));
  }
  void Man::UncountEdges() {
    for(bvar a = nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        UncountEdges_rec(Bvar2Lit(a));
      }
    }
    for(int i = 0; i < nVars; i++) {
      vEdges[i + 1]--;
    }
    for(bvar a = nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        ResetMark_rec(Bvar2Lit(a));
      }
    }
    for(bvar a = 1; a < nObjs; a++) {
      if(Edge(Bvar2Lit(a))) {
        cout << "strange edges " << a << " having " << Edge(Bvar2Lit(a)) << endl;
      }
    }
  }

  lit Man::UniqueCreateInt(var v, lit x1, lit x0) {
    vector<bvar>::iterator p, q;
    p = q = vvUnique[v].begin() + (Hash(x1, x0) & vUniqueMasks[v]);
    for(; *q; q = vNexts.begin() + *q) {
      if(VarOfBvar(*q) == v && ThenOfBvar(*q) == x1 && ElseOfBvar(*q) == x0) {
        return Bvar2Lit(*q);
      }
    }
    bvar next = *p;
    if(nObjs < nObjsAlloc) {
      *p = nObjs++;
    } else {
      for(; MinBvarRemoved < nObjs; MinBvarRemoved++) {
        if(VarOfBvar(MinBvarRemoved) == VarMax()) {
          break;
        }
      }
      if(MinBvarRemoved >= nObjs) {
        return LitMax();
      }
      *p = MinBvarRemoved++;
    }
    SetVarOfBvar(*p, v);
    SetThenOfBvar(*p, x1);
    SetElseOfBvar(*p, x0);
    vNexts[*p] = next;
    if(nVerbose >= 3) {
      cout << "Create node " << *p << " : Var = " << v << " Then = " << x1 << " Else = " << x0 << endl;
    }
    vUniqueCounts[v]++;
    if(vUniqueCounts[v] > vUniqueTholds[v]) {
      bvar a = *p;
      ResizeUnique(v);
      return Bvar2Lit(a);
    }
    return Bvar2Lit(*p);
  }
  lit Man::UniqueCreate(var v, lit x1, lit x0) {
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
        if(!Resize() && !fRemoved && nGbc != 1 && !Gbc()) {
          throw length_error("Memout (node)");
        }
      } else {
        break;
      }
    }
    return x;
  }

  lit Man::CacheLookup(lit x, lit y) {
    nCacheLookups++;
    size i = (size)(Hash(x, y) & CacheMask) * 3;
    if(vCache[i] == x && vCache[i + 1] == y) {
      nCacheHits++;
      return vCache[i + 2];
    }
    return LitMax();
  }
  void Man::CacheInsert(lit x, lit y, lit z) {
    size i = (size)(Hash(x, y) & CacheMask) * 3;
    vCache[i] = x;
    vCache[i + 1] = y;
    vCache[i + 2] = z;
  }

  lit Man::And_rec(lit x, lit y) {
    if(x == 0 || y == 1 || x == y) {
      return x;
    }
    if(x == 1 || y == 0) {
      return y;
    }
    if(x > y) {
      swap(x, y);
    }
    lit z = CacheLookup( x, y );
    if(z != LitMax()) {
      return z;
    }
    var v;
    lit x0, x1, y0, y1;
    if(Level(x) < Level(y)) {
      v = Var(x), x0 = Else(x), x1 = Then(x), y0 = y1 = y;
    } else if(Level(x) > Level(y)) {
      v = Var(y), x0 = x1 = x, y0 = Else(y), y1 = Then(y);
    } else {
      v = Var(x), x0 = Else(x), x1 = Then(x), y0 = Else(y), y1 = Then(y);
    }
    lit z1 = And_rec(x1, y1);
    IncRef(z1);
    lit z0 = And_rec(x0, y0);
    IncRef(z0);
    z = UniqueCreate(v, z1, z0);
    DecRef(z1);
    DecRef(z0);
    CacheInsert(x, y, z);
    return z;
  }
  lit Man::And(lit x, lit y) {
    CheckTholds();
    return And_rec(x, y);
  }

  bool Man::Resize() {
    bvar nObjsAllocOld = nObjsAlloc;
    nObjsAlloc <<= 1;
    if((size)nObjsAlloc > (size)BvarMax()) {
      nObjsAlloc = BvarMax();
    }
    if((size)nObjsAlloc > nMaxMem) {
      nObjsAlloc = nObjsAllocOld;
      return false;
    }
    if(nVerbose >= 2) {
      cout << "Reallocate " << nObjsAlloc << " nodes." << endl;
    }
    vVars.resize(nObjsAlloc);
    vObjs.resize((size)nObjsAlloc * 2);
    vNexts.resize(nObjsAlloc);
    vMarks.resize(nObjsAlloc);
    if(!vRefs.empty()) {
      vRefs.resize(nObjsAlloc);
    }
    if(!vEdges.empty()) {
      vEdges.resize(nObjsAlloc);
    }
    return true;
  }

  void Man::ResizeUnique(var v) {
    lit nUnique, nUniqueOld;
    nUnique = nUniqueOld = vvUnique[v].size();
    nUnique <<= 1;
    if(!nUnique || (size)nUnique > nMaxMem) {
      vUniqueTholds[v] = BvarMax();
      return;
    }
    if(nVerbose >= 2) {
      cout << "Reallocate " << nUnique << " unique." << endl;
    }
    vvUnique[v].resize(nUnique);
    vUniqueMasks[v] = nUnique - 1;
    for(lit i = 0; i < nUniqueOld; i++) {
      vector<bvar>::iterator q, tail, tail1, tail2;
      q = tail1 = vvUnique[v].begin() + i;
      tail2 = q + nUniqueOld;
      while(*q) {
        lit hash = Hash(ThenOfBvar(*q), ElseOfBvar(*q)) & vUniqueMasks[v];
        if(hash == i) {
          tail = tail1;
        } else {
          tail = tail2;
        }
        if(tail != q) {
          *tail = *q;
          *q = 0;
        }
        q = vNexts.begin() + *tail;
        if(tail == tail1) {
          tail1 = q;
        } else {
          tail2 = q;
        }
      }
    }
    vUniqueTholds[v] <<= 1;
    if((size)vUniqueTholds[v] > (size)BvarMax()) {
      vUniqueTholds[v] = BvarMax();
    }
  }

  void Man::ResizeCache() {
    lit nCache, nCacheOld;
    nCache = nCacheOld = vCache.size() / 3;
    nCache <<= 1;
    if(!nCache || (size)nCache > nMaxMem) {
      CacheThold = SizeMax();
      return;
    }
    if(nVerbose >= 2) {
      cout << "Reallocate " << nCache << " cache." << endl;
    }
    vCache.resize((size)nCache * 3);
    CacheMask = nCache - 1;
    for(lit j = 0; j < nCacheOld; j++) {
      size i = (size)j * 3;
      if(!vCache[i] || !vCache[i + 1]) {
        continue;
      }
      size hash = (size)(Hash(vCache[i], vCache[i + 1]) & CacheMask) * 3;
      if(i != hash) {
        vCache[hash] = vCache[i];
        vCache[hash + 1] = vCache[i + 1];
        vCache[hash + 2] = vCache[i + 2];
        vCache[i] = 0;
        vCache[i + 1] = 0;
        vCache[i + 2] = 0;
      }
    }
    while(nCacheLookups > CacheThold) {
      CacheThold <<= 1;
      if(!CacheThold) {
        CacheThold = SizeMax();
      }
    }
  }

  void Man::CacheClear() {
    size i = vCache.size();
    vCache.clear();
    vCache.resize(i);
  }

  void Man::RemoveBvar(bvar a) {
    var v = VarOfBvar(a);
    SetVarOfBvar(a, VarMax());
    if(MinBvarRemoved > a) {
      MinBvarRemoved = a;
    }
    vector<bvar>::iterator q = vvUnique[v].begin() + (Hash(ThenOfBvar(a), ElseOfBvar(a)) & vUniqueMasks[v]);
    for(; *q; q = vNexts.begin() + *q) {
      if(*q == a) {
        break;
      }
    }
    bvar next = vNexts[*q];
    vNexts[*q] = 0;
    *q = next;
    vUniqueCounts[v]--;
  }

  bool Man::Gbc() {
    if(nVerbose >= 2) {
      cout << "Garbage collect" << endl;
    }
    bvar MinBvarRemovedOld = MinBvarRemoved;
    for(bvar a = nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        SetMark_rec(Bvar2Lit(a));
      }
    }
    for(bvar a = nVars + 1; a < nObjs; a++) {
      if(!MarkOfBvar(a) && VarOfBvar(a) != VarMax()) {
        RemoveBvar(a);
      }
    }
    for(bvar a = nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        ResetMark_rec(Bvar2Lit(a));
      }
    }
    CacheClear();
    return MinBvarRemoved != MinBvarRemovedOld;
  }

  bvar Man::Swap(var i) {
    var v1 = Level2Var[i];
    var v2 = Level2Var[i + 1];
    bvar f = 0;
    bvar count = 0;
    for(vector<bvar>::iterator p = vvUnique[v1].begin(); p != vvUnique[v1].end(); p++) {
      vector<bvar>::iterator q = p;
      while(*q) {
        if(!EdgeOfBvar(*q)) {
          SetVarOfBvar(*q, VarMax());
          if(MinBvarRemoved > *q) {
            MinBvarRemoved = *q;
          }
          bvar next = vNexts[*q];
          vNexts[*q] = 0;
          *q = next;
          vUniqueCounts[v1]--;
          continue;
        }
        lit f1 = ThenOfBvar(*q);
        lit f0 = ElseOfBvar(*q);
        if(Var(f1) == v2 || Var(f0) == v2) {
          DecEdge(f1);
          if(Var(f1) == v2 && !Edge(f1)) {
            DecEdge(Then(f1)), DecEdge(Else(f1)), count--;
          }
          DecEdge(f0);
          if(Var(f0) == v2 && !Edge(f0)) {
            DecEdge(Then(f0)), DecEdge(Else(f0)), count--;
          }
          bvar next = vNexts[*q];
          vNexts[*q] = f;
          f = *q;
          *q = next;
          vUniqueCounts[v1]--;
          continue;
        }
        q = vNexts.begin() + *q;
      }
    }
    while(f) {
      lit f0 = ElseOfBvar(f);
      lit f1 = ThenOfBvar(f);
      lit f00, f01, f10, f11;
      if(Var(f0) == v2) {
        f00 = Else(f0), f01 = Then(f0);
      } else {
        f00 = f01 = f0;
      }
      if(Var(f1) == v2) {
        f10 = Else(f1), f11 = Then(f1);
      } else {
        f10 = f11 = f1;
      }
      if(f10 == f00) {
        f0 = f10;
      } else {
        f0 = UniqueCreate(v1, f10, f00);
        if(!Edge(f0)) {
          IncEdge(f10), IncEdge(f00), count++;
        }
      }
      IncEdge(f0);
      IncRef(f0);
      if(f11 == f01) {
        f1 = f11;
      } else {
        f1 = UniqueCreate(v1, f11, f01);
        if(!Edge(f1)) {
          IncEdge(f11), IncEdge(f01), count++;
        }
      }
      IncEdge(f1);
      DecRef(f0);
      // change
      SetVarOfBvar(f, v2);
      SetElseOfBvar(f, f0);
      SetThenOfBvar(f, f1);
      vector<bvar>::iterator q = vvUnique[v2].begin() + (Hash(f1, f0) & vUniqueMasks[v2]);
      lit next = vNexts[f];
      vNexts[f] = *q;
      *q = f;
      vUniqueCounts[v2]++;
      // next target
      f = next;
    }
    Var2Level[v1] = i + 1;
    Var2Level[v2] = i;
    Level2Var[i] = v2;
    Level2Var[i + 1] = v1;
    return count;
  }

  void Man::Sift() {
    Gbc();
    CountEdges();
    vector<var> sift_order(nVars);
    for(int i = 0; i < nVars; i++) {
      sift_order[i] = i;
    }
    for(int i = 0; i < nVars; i++) {
      int max_j = i;
      for(int j = i + 1; j < nVars; j++) {
        if(vUniqueCounts[sift_order[j]] > vUniqueCounts[sift_order[max_j]]) {
          max_j = j;
        }
      }
      if(max_j != i) {
        swap(sift_order[max_j], sift_order[i]);
      }
    }
    for(int i = 0; i < nVars; i++) {
      int lev = Var2Level[sift_order[i]];
      bool UpFirst = lev < (nVars / 2);
      int min_lev = lev;
      bvar min_diff = 0;
      bvar diff = 0;
      if(UpFirst) {
        lev--;
        for(; lev >= 0; lev--) {
          diff += Swap(lev);
          if(diff < min_diff) {
            min_lev = lev;
            min_diff = diff;
          }
        }
        lev++;
      }
      for(; lev < nVars - 1; lev++) {
        diff += Swap(lev);
        if(diff < min_diff) {
          min_lev = lev + 1;
          min_diff = diff;
        }
      }
      lev--;
      if(UpFirst) {
        for(; lev >= min_lev; lev--) {
          diff += Swap(lev);
        }
      } else {
        for(; lev >= 0; lev--) {
          diff += Swap(lev);
          if(diff < min_diff) {
            min_lev = lev;
            min_diff = diff;
          }
        }
        lev++;
        for(; lev < min_lev; lev++) {
          diff += Swap(lev);
        }
      }
    }
    //UncountEdges();
    vEdges.clear();
  }

  void Man::CheckTholds() {
    if(nObjs > nReo) {
      Sift();
      if(nObjs == BvarMax()) {
        nReo = BvarMax();
      } else {
        while(nReo < nObjs) {
          nReo <<= 1;
        }
      }
    }
    if(nCacheLookups > CacheThold) {
      double NewCacheHitRate = (double)nCacheHits / nCacheLookups;
      if(NewCacheHitRate > CacheHitRate) {
        ResizeCache();
      } else {
        while(nCacheLookups > CacheThold) {
          CacheThold <<= 1;
          if(!CacheThold) {
            CacheThold = SizeMax();
          }
        }
      }
      CacheHitRate = NewCacheHitRate;
    }
  }

  size Man::CountNodes_rec(lit x) {
    if(x < 2 || Mark(x)) {
      return 0;
    }
    SetMark(x);
    return 1ull + CountNodes_rec(Else(x)) + CountNodes_rec(Then(x));
  }

}
