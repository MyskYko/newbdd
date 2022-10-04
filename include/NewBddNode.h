#ifndef NEW_BDD_NODE_H
#define NEW_BDD_NODE_H

#include "NewBdd.h"

namespace NewBdd {

  class Node {
  public:
    friend class Man;
    friend class NodeNoRef;

    Node(Man * man, lit val);
    Node();
    Node(const Node & right);
    ~Node();
    Node & operator=(const Node & right);

  private:
    Man * man;
    lit val;
  };

  class NodeNoRef {
  public:
    friend class Man;

    NodeNoRef(lit val);
    NodeNoRef();
    NodeNoRef(const NodeNoRef & right);
    NodeNoRef(const Node & right);
    NodeNoRef & operator=(const NodeNoRef & right);

  private:
    lit val;
  };

}

#endif
