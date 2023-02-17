#include <iostream>

#include "NewBddMan.h"

using namespace std;

namespace NewBdd {

  void Man::SetMark_rec(lit x) {
    if(x < 2 || Mark(x)) {
      return;
    }
    SetMark(x);
    SetMark_rec(Then(x));
    SetMark_rec(Else(x));
  }
  void Man::ResetMark_rec(lit x) {
    if(x < 2 || !Mark(x)) {
      return;
    }
    ResetMark(x);
    ResetMark_rec(Then(x));
    ResetMark_rec(Else(x));
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
    CountEdges_rec(Then(x));
    CountEdges_rec(Else(x));
  }
  void Man::CountEdges() {
    vEdges.resize(nObjsAlloc);
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        CountEdges_rec(Bvar2Lit(a));
      }
    }
    for(bvar a = 1; a <= (bvar)nVars; a++) {
      vEdges[a]++;
    }
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        ResetMark_rec(Bvar2Lit(a));
      }
    }
  }

#ifdef REO_DEBUG
  void Man::UncountEdges_rec(lit x) {
    if(x < 2) {
      return;
    }
    DecEdge(x);
    if(Mark(x)) {
      return;
    }
    SetMark(x);
    UncountEdges_rec(Then(x));
    UncountEdges_rec(Else(x));
  }
  void Man::UncountEdges() {
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        UncountEdges_rec(Bvar2Lit(a));
      }
    }
    for(bvar a = 1; a <= (bvar)nVars; a++) {
      vEdges[a]--;
    }
    for(bvar a = (bvar)nVars + 1; a < nObjs; a++) {
      if(RefOfBvar(a)) {
        ResetMark_rec(Bvar2Lit(a));
      }
    }
    for(bvar a = 1; a < nObjs; a++) {
      if(EdgeOfBvar(a)) {
        cout << "Strange edge " << a << " : Edge = " << EdgeOfBvar(a) << endl;
      }
    }
  }
#endif

  lit Man::And_rec(lit x, lit y) {
    if(x == 0 || y == 1) {
      return x;
    }
    if(x == 1 || y == 0) {
      return y;
    }
    if(Lit2Bvar(x) == Lit2Bvar(y)) {
      if(x == y) {
        return x;
      }
      return 0;
    }
    if(x > y) {
      swap(x, y);
    }
    lit z = CacheLookup(x, y);
    if(z != LitMax()) {
      return z;
    }
    var v;
    lit x0, x1, y0, y1;
    if(Level(x) < Level(y)) {
      v = Var(x), x1 = Then(x), x0 = Else(x), y0 = y1 = y;
    } else if(Level(x) > Level(y)) {
      v = Var(y), x0 = x1 = x, y1 = Then(y), y0 = Else(y);
    } else {
      v = Var(x), x1 = Then(x), x0 = Else(x), y1 = Then(y), y0 = Else(y);
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

  bool Man::Resize() {
    if(nObjsAlloc == BvarMax()) {
      return false;
    }
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
    if(!vOneCounts.empty()) {
      vOneCounts.resize(nObjsAlloc);
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
      if(vCache[i]) {
        size hash = (size)(Hash(vCache[i], vCache[i + 1]) & CacheMask) * 3;
        vCache[hash] = vCache[i];
        vCache[hash + 1] = vCache[i + 1];
        vCache[hash + 2] = vCache[i + 2];
      }
    }
    CacheThold <<= 1;
    if(!CacheThold) {
      CacheThold = SizeMax();
    }
  }

  bvar Man::Swap(var i) {
    var v1 = Level2Var[i];
    var v2 = Level2Var[i + 1];
    bvar f = 0;
    bvar diff = 0;
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
            DecEdge(Then(f1)), DecEdge(Else(f1)), diff--;
          }
          DecEdge(f0);
          if(Var(f0) == v2 && !Edge(f0)) {
            DecEdge(Then(f0)), DecEdge(Else(f0)), diff--;
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
      lit f1 = ThenOfBvar(f);
      lit f0 = ElseOfBvar(f);
      lit f00, f01, f10, f11;
      if(Var(f1) == v2) {
        f11 = Then(f1), f10 = Else(f1);
      } else {
        f10 = f11 = f1;
      }
      if(Var(f0) == v2) {
        f01 = Then(f0), f00 = Else(f0);
      } else {
        f00 = f01 = f0;
      }
      if(f11 == f01) {
        f1 = f11;
      } else {
        f1 = UniqueCreate(v1, f11, f01);
        if(!Edge(f1)) {
          IncEdge(f11), IncEdge(f01), diff++;
        }
      }
      IncEdge(f1);
      IncRef(f1);
      if(f10 == f00) {
        f0 = f10;
      } else {
        f0 = UniqueCreate(v1, f10, f00);
        if(!Edge(f0)) {
          IncEdge(f10), IncEdge(f00), diff++;
        }
      }
      IncEdge(f0);
      DecRef(f1);
      SetVarOfBvar(f, v2);
      SetThenOfBvar(f, f1);
      SetElseOfBvar(f, f0);
      vector<bvar>::iterator q = vvUnique[v2].begin() + (Hash(f1, f0) & vUniqueMasks[v2]);
      lit next = vNexts[f];
      vNexts[f] = *q;
      *q = f;
      vUniqueCounts[v2]++;
      f = next;
    }
    Var2Level[v1] = i + 1;
    Var2Level[v2] = i;
    Level2Var[i] = v2;
    Level2Var[i + 1] = v1;
    return diff;
  }

  void Man::Sift() {
    bvar count = CountNodes();
    vector<var> sift_order(nVars);
    for(var v = 0; v < nVars; v++) {
      sift_order[v] = v;
    }
    for(var i = 0; i < nVars; i++) {
      var max_j = i;
      for(var j = i + 1; j < nVars; j++) {
        if(vUniqueCounts[sift_order[j]] > vUniqueCounts[sift_order[max_j]]) {
          max_j = j;
        }
      }
      if(max_j != i) {
        swap(sift_order[max_j], sift_order[i]);
      }
    }
    for(var v = 0; v < nVars; v++) {
      bvar lev = Var2Level[sift_order[v]];
      bool UpFirst = lev < (bvar)(nVars / 2);
      bvar min_lev = lev;
      bvar min_diff = 0;
      bvar diff = 0;
      bvar thold = count * (MaxGrowth - 1);
      if(fReoVerbose) {
        cout << "Sift " << sift_order[v] << " : Level = " << lev << " Count = " << count << " Thold = " << thold << endl;
      }
#ifdef REO_DEBUG
    if(count != CountNodes()) {
      throw runtime_error("Count mismatch in reorder");
    }
#endif
      if(UpFirst) {
        lev--;
        for(; lev >= 0; lev--) {
          diff += Swap(lev);
          if(fReoVerbose) {
            cout << "\tSwap " << lev << " : Diff = " << diff << " Thold = " << thold << endl;
          }
#ifdef REO_DEBUG
          if(count + diff != CountNodes()) {
            throw runtime_error("Count mismatch in reorder");
          }
#endif
          if(diff < min_diff) {
            min_lev = lev;
            min_diff = diff;
            thold = (count + diff) * (MaxGrowth - 1);
          } else if(diff > thold) {
            lev--;
            break;
          }
        }
        lev++;
      }
      for(; lev < (bvar)nVars - 1; lev++) {
        diff += Swap(lev);
        if(fReoVerbose) {
          cout << "\tSwap " << lev << " : Diff = " << diff << " Thold = " << thold << endl;
        }
#ifdef REO_DEBUG
        if(count + diff != CountNodes()) {
          throw runtime_error("Count mismatch in reorder");
        }
#endif
        if(diff <= min_diff) {
          min_lev = lev + 1;
          min_diff = diff;
          thold = (count + diff) * (MaxGrowth - 1);
        } else if(diff > thold) {
          lev++;
          break;
        }
      }
      lev--;
      if(UpFirst) {
        for(; lev >= min_lev; lev--) {
#ifdef REO_DEBUG
          diff +=
#endif
          Swap(lev);
          if(fReoVerbose) {
            cout << "\tSwap " << lev << " : Diff = " << diff << " Thold = " << thold << endl;
          }
#ifdef REO_DEBUG
          if(count + diff != CountNodes()) {
            throw runtime_error("Count mismatch in reorder");
          }
#endif
        }
      } else {
        for(; lev >= 0; lev--) {
          diff += Swap(lev);
          if(fReoVerbose) {
            cout << "\tSwap " << lev << " : Diff = " << diff << " Thold = " << thold << endl;
          }
#ifdef REO_DEBUG
          if(count + diff != CountNodes()) {
            throw runtime_error("Count mismatch in reorder");
          }
#endif
          if(diff <= min_diff) {
            min_lev = lev;
            min_diff = diff;
            thold = (count + diff) * (MaxGrowth - 1);
          } else if(diff > thold) {
            lev--;
            break;
          }
        }
        lev++;
        for(; lev < min_lev; lev++) {
#ifdef REO_DEBUG
          diff +=
#endif
          Swap(lev);
          if(fReoVerbose) {
            cout << "\tSwap " << lev << " : Diff = " << diff << " Thold = " << thold << endl;
          }
#ifdef REO_DEBUG
          if(count + diff != CountNodes()) {
            throw runtime_error("Count mismatch in reorder");
          }
#endif
        }
      }
      count += min_diff;
      if(fReoVerbose) {
        cout << "Sifted " << sift_order[v] << " : Level = " << min_lev << " Count = " << count << " Thold = " << thold << endl;
      }
    }
  }

  bvar Man::CountNodes_rec(lit x) {
    if(x < 2 || Mark(x)) {
      return 0;
    }
    SetMark(x);
    return 1 + CountNodes_rec(Then(x)) + CountNodes_rec(Else(x));
  }

}
