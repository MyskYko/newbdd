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
    bool operator==(const Node & other) const;
    bool operator!=(const Node & other) const;

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
    bool operator==(const NodeNoRef & other) const;
    bool operator!=(const NodeNoRef & other) const;

  private:
    lit val;
  };

}

#endif
