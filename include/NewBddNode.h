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
    Node(Node const & right);
    ~Node();
    Node & operator=(Node const & right);

    bool operator==(Node const & other) const;
    bool operator!=(Node const & other) const;

  private:
    Man * man;
    lit val;
  };

  class NodeNoRef {
  public:
    friend class Man;

    NodeNoRef(lit val);
    NodeNoRef();
    NodeNoRef(NodeNoRef const & right);
    NodeNoRef(Node const & right);
    NodeNoRef & operator=(NodeNoRef const & right);

  private:
    lit val;
  };

}

#endif
