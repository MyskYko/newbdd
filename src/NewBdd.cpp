#include <cstring>

#include "NewBdd.h"

using namespace std;

namespace NewBdd {

  lit Man::UniqueCreateInt(var i, lit x1, lit x0) {
    vector<bvar>::iterator p, q;
    p = q = vvUnique[i].begin() + (Hash(x1, x0) & vUniqueMask[i]);
    for(; *q; q = vNexts.begin() + *q) {
      if(LevelOfBvar(*q) == i && ThenOfBvar(*q) == x1 && ElseOfBvar(*q) == x0) {
        return Bvar2Lit(*q);
      }
    }
    bvar next = *p;
    if(nObjs == nObjsAlloc) {
      return LitMax();
    }
    //     for ( ; nMinRemoved < (lit)nObjs; nMinRemoved++ )
    //       if ( BvarIsRemoved( nMinRemoved ) )
    //         break;
    //     if ( nMinRemoved == (lit)nObjs )
    //     *p = nMinRemoved++;
    //   }
    // else
    *p = nObjs++;
    SetLevelOfBvar(*p, i);
    SetThenOfBvar(*p, x1);
    SetElseOfBvar(*p, x0);
    vNexts[*p] = next;
    if(nVerbose >= 3) {
      cout << "Create node " << *p << " : Level = " << i << " Then = " << x1 << " Else = " << x0 << endl;
    }
    return Bvar2Lit(*p);
  }
  lit Man::UniqueCreate(var i, lit x1, lit x0) {
    if(x1 == x0) {
      return x1;
    }
    lit x;
    while(true) {
      if(!LitIsCompl(x0)) {
        x = UniqueCreateInt(i, x1, x0);
      } else {
        x = LitNot(UniqueCreateInt(i, LitNot(x1), LitNot(x0)));
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

  lit Man::CacheLookup(lit x, lit y) {
    nCacheLookup++;
    size i = (size)(Hash(x, y) & CacheMask) * 3;
    if(vCache[i] == x && vCache[i + 1] == y) {
      nCacheHit++;
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
    lit x0, x1, y0, y1;
    if(Level(x) < Level(y)) {
      x0 = Else(x), x1 = Then(x), y0 = y1 = y;
    } else if(Level(x) > Level(y)) {
      x0 = x1 = x, y0 = Else(y), y1 = Then(y);
    } else {
      x0 = Else(x), x1 = Then(x), y0 = Else(y), y1 = Then(y);
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
    z = UniqueCreate(min(Level(x), Level(y)), z1, z0);
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

  void Man::Resize() {
    nObjsAlloc <<= 1;
    if((size)nObjsAlloc > (size)BvarMax()) {
      nObjsAlloc = BvarMax();
    }
    if(!nObjsAlloc || (size)nObjsAlloc > nMaxMem) {
      throw length_error("Memout (node) in resize");
    }
    if(nVerbose) {
      cout << "\tReallocate " << nObjsAlloc << " nodes." << endl;
    }
    vLevels.resize(nObjsAlloc);
    vObjs.resize((size)nObjsAlloc * 2);
    vNexts.resize(nObjsAlloc);
    vMarks.resize(nObjsAlloc);
    // if ( pRefs )
    //   {
    //     pRefs       = (ref *)realloc( pRefs, sizeof(ref) * nObjsAlloc );
    //     if ( !pRefs )
    //       throw "Reallocation failed";
    //     memset( pRefs + nObjsAllocOld, 0, sizeof(ref) * nObjsAllocOld );
    //   }
    // if ( pEdges )
    //   {
    //     pEdges = (edge *)realloc( pEdges, sizeof(edge) * nObjsAlloc );
    //     if ( !pEdges )
    //       throw "Reallocation failed";
    //     memset ( pEdges + nObjsAllocOld, 0, sizeof(edge) * nObjsAllocOld );
    //   }
    // while ( nUnique < nObjsAlloc * UniqueMinRate )
    //   if ( UniqueResize() )
    //     break;
  }

  void Man::Refresh() {
    Resize();
  }

  size Man::CountNodes_rec(lit x) {
    if(x < 2 || Mark(x)) {
      return 0;
    }
    SetMark(x);
    return 1ull + CountNodes_rec(Else(x)) + CountNodes_rec(Then(x));
  }

  Node Man::Const0() {
    return Node(this, 0);
  }
  Node Man::Const1() {
    return Node(this, 1);
  }
  Node Man::IthVar(int i) {
    return Node(this, Bvar2Lit(Var2Level[i] + 1));
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
