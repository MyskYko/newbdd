#include <cstring>

#include "NewBdd.h"

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
    vEdges.clear();
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
    if(++vUniqueCounts[v] > vUniqueTholds[v]) {
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
        Refresh();
        //           if ( Refresh() )
        //             return x;
      } else {
        break;
      }
    }
    return x;
  }

  lit Man::CacheLookup(lit x, lit y) {
    if(++nCacheLookups > CacheThold) {
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
    // if(z1 == LitMax()) {
    //   return z1;
    // }
    IncRef(z1);
    lit z0 = And_rec(x0, y0);
    // if(z0 == LitMax()) {
    //   DecRef(z1);
    //   return z0;
    // }
    IncRef(z0);
    z = UniqueCreate(v, z1, z0);
    DecRef(z1);
    DecRef(z0);
    // if(z == LitMax()) {
    //   return z;
    // }
    CacheInsert(x, y, z);
    return z;
  }
  lit Man::And(lit x, lit y) {
    // nRefresh = 0;
    // lit z = LitInvalid();
    // while( LitIsInvalid( z ) )
    //   z = And_rec( x, y );
    // return z;
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
    return true;
    // if ( pEdges )
    //   {
    //     pEdges = (edge *)realloc( pEdges, sizeof(edge) * nObjsAlloc );
    //     if ( !pEdges )
    //       throw "Reallocation failed";
    //     memset ( pEdges + nObjsAllocOld, 0, sizeof(edge) * nObjsAllocOld );
    //   }
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
    CacheThold <<= 1;
    if(!CacheThold) {
      CacheThold = SizeMax();
    }
  }

  void Man::CacheClear() {
    size i = vCache.size();
    vCache.clear();
    vCache.resize(i);
  }

  void Man::RemoveBvar(bvar a) {
    var v = VarOfBvar(a);
    vector<bvar>::iterator q = vvUnique[v].begin() + (Hash(ThenOfBvar(a), ElseOfBvar(a)) & vUniqueMasks[v]);
    for(; *q; q = vNexts.begin() + *q) {
      if(*q == a) {
        break;
      }
    }
    vector<bvar>::iterator next = vNexts.begin() + *q;
    *q = *next;
    *next = 0;
    vUniqueCounts[v]--;
    SetVarOfBvar(a, VarMax());
    if(MinBvarRemoved > a) {
      MinBvarRemoved = a;
    }
  }

  void Man::Gbc() {
    if(nVerbose >= 2) {
      cout <<  "Garbage collect" << endl;
    }
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
  }

  void Man::Swap(int i) {
    CountEdges();
    var v1 = Level2Var[i];
    var v2 = Level2Var[i + 1];
    int f = 0;
    for(vector<bvar>::iterator p = vvUnique[v1].begin(); p != vvUnique[v1].end(); p++) {
      vector<bvar>::iterator q = p;
      while(*q) {
        if(EdgeOfBvar(*q) && (Var(ThenOfBvar(*q)) == v2 || Var(ElseOfBvar(*q)) == v2)) {
          DecEdge(ThenOfBvar(*q));
          DecEdge(ElseOfBvar(*q));
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
        f00 = Else(f0);
        f01 = Then(f0);
      } else {
        f00 = f01 = f0;
      }
      if(Var(f1) == v2) {
        f10 = Else(f1);
        f11 = Then(f1);
      } else {
        f10 = f11 = f1;
      }
      if(f10 == f00) {
        f0 = f10;
      } else {
        f0 = UniqueCreate(v1, f10, f00);
        if(!Edge(f0)) {
          IncEdge(f10);
          IncEdge(f00);
        }
      }
      IncEdge(f0);
      IncRef(f0);
      if(f11 == f01) {
        f1 = f11;
      } else {
        f1 = UniqueCreate(v1, f11, f01);
        if(!Edge(f1)) {
          IncEdge(f11);
          IncEdge(f01);
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
    CacheClear();
  }

  void Man::Refresh() {
    if(Resize()) {
      return;
    }
    Gbc();
  }

  size Man::CountNodes_rec(lit x) {
    if(x < 2 || Mark(x)) {
      return 0;
    }
    SetMark(x);
    return 1ull + CountNodes_rec(Else(x)) + CountNodes_rec(Then(x));
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
