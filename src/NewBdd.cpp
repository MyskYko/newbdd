#include "NewBdd.h"

namespace NewBdd {

  lit BddMan::UniqueCreateInt(var l, lit x1, lit x0) {
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
      std::cout << "Create node " << *q << " : Level = " << l << " Then = " << x1 << " Else = " << x0 << std::endl;
    }
    return Bvar2Lit(*q);
  }
  lit BddMan::UniqueCreate(var l, lit x1, lit x0) {
    if(x1 == x0) {
      return x1;
    }
    lit x;
    //while(true) {
    if(LitIsCompl(x0)) {
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

  lit BddMan::And_rec(lit x, lit y) {
    //CacheCheck();
    if(x == Const0() || y == Const1() || x == y) {
      return x;
    }
    if(x == Const1() || y == Const0()) {
      return y;
    }
    if(x > y) {
      std::swap(x, y);
    }
    // lit z = CacheLookup( x, y );
    // if ( !LitIsInvalid( z ) )
    //   return z;
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
    lit z = UniqueCreate(std::min(Level(x), Level(y)), z1, z0);
    DecRef(z1);
    DecRef(z0);
    // if ( LitIsInvalid( z ) )
    //   return z;
    // return CacheInsert( x, y, z );
    return z;
  }
  lit BddMan::And(lit x, lit y) {
    // nRefresh = 0;
    // lit z = LitInvalid();
    // while( LitIsInvalid( z ) )
    //   z = And_rec( x, y );
    // return z;
    return And_rec(x, y);
  }

  node BddMan::IthVar(int i) {
    return node(this, Bvar2Lit(Var2Level[i] + 1));
  }
  node BddMan::And(node const & x, node const & y) {
    return node(this, And(x.val, y.val));
  }

}
