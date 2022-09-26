#ifndef NEW_BDD_NODE_H
#define NEW_BDD_NODE_H

#include "NewBdd.h"

namespace NewBdd {

  class Node {
  public:
    friend class Man;

    Node(Man * man, lit val);
    Node();
    Node(const Node & right);
    ~Node();

    Node & operator=(const Node & right);
    bool operator==(const Node & other) const;
    bool operator!=(const Node & other) const;

  private:
    Man * man;
    lit val;
  };

}

#endif
