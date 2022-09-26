#ifndef NEW_BDD_TYPES_H
#define NEW_BDD_TYPES_H

#include <limits>

namespace NewBdd {
  
  typedef unsigned short var;
  typedef int bvar;
  typedef unsigned lit;
  typedef unsigned short ref;
  typedef unsigned long long size;
  typedef unsigned edge;

  static inline var VarMax() {
    return std::numeric_limits<var>::max();
  }
  static inline bvar BvarMax() {
    return std::numeric_limits<bvar>::max();
  }
  static inline lit LitMax() {
    return std::numeric_limits<lit>::max();
  }
  static inline ref RefMax() {
    return std::numeric_limits<ref>::max();
  }
  static inline size SizeMax() {
    return std::numeric_limits<size>::max();
  }

  class Man;
  class Node;

}

#endif
