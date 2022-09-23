#include "NewBdd.h"

using namespace std;

namespace NewBdd {

  lit Man::UniqueCreateInt(var l, lit x1, lit x0) {
    bvar * head, * q;
    head = q = vpUnique[l] + (Hash(x1, x0) & vUniqueMask[l]);
    for(; *q; q = pNexts + *q) {
      if(LevelOfBvar(*q) == l && ThenOfBvar(*q) == x1 && ElseOfBvar(*q) == x0) {
        return Bvar2Lit(*q);
      }
    }
    bvar next = *head;
    //  if ( IsLimit() )
    //   {
    //     for ( ; nMinRemoved < (lit)nObjs; nMinRemoved++ )
    //       if ( BvarIsRemoved( nMinRemoved ) )
    //         break;
    //     if ( nMinRemoved == (lit)nObjs )
    //       return LitInvalid();
    //     *q = nMinRemoved++;
    //   }
    // else
    q = head;
    *q = nObjs++;
    SetLevelOfBvar(*q, l);
    SetThenOfBvar(*q, x1);
    SetElseOfBvar(*q, x0);
    pNexts[*q] = next;
    if(nVerbose >= 3) {
      cout << "Create node " << *q << " : Level = " << l << " Then = " << x1 << " Else = " << x0 << endl;
    }
    return Bvar2Lit(*q);
  }
  lit Man::UniqueCreate(var l, lit x1, lit x0) {
    if(x1 == x0) {
      return x1;
    }
    lit x;
    //while(true) {
    if(!LitIsCompl(x0)) {
      x = UniqueCreateInt(l, x1, x0);
    } else {
      x = LitNot(UniqueCreateInt(l, LitNot(x1), LitNot(x0)));
    }
  //       if ( LitIsInvalid( x ) )
  //         {
  //           if ( Refresh() )
  //             return x;
  //         }
  //       else
  //         break;
  //     }
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
    // if ( LitIsInvalid( Res ) )
    //   return Res;
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
    // if ( LitIsInvalid( z1 ) )
    //   return z1;
    IncRef(z1);
    lit z0 = And_rec(x0, y0);
    // if( LitIsInvalid( z0 ) )
    //   {
    //     DecRef( z1 );
    //     return z0;
    //   }
    IncRef(z0);
    z = UniqueCreate(min(Level(x), Level(y)), z1, z0);
    DecRef(z1);
    DecRef(z0);
    // if ( LitIsInvalid( z ) )
    //   return z;
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

  size Man::CountNodes_rec(lit x) {
    if(x < 2 || Mark(x)) {
      return 0;
    }
    SetMark(x);
    return 1ull + CountNodes_rec(Else(x)) + CountNodes_rec(Then(x));
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
