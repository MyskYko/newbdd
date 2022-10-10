#include "NewBddNode.h"

namespace NewBdd {

  Node::Node(Man * man, lit val) : man(man), val(val) {
    man->IncRef(val);
  }
  Node::Node() {
    man = NULL;
  }
  Node::Node(Node const & right) {
    man = right.man;
    val = right.val;
    if(man) {
      man->IncRef(val);
    }
  }
  Node::~Node() {
    if(man) {
      man->DecRef(val);
    }
  }
  Node & Node::operator=(Node const & right) {
    if(this == &right) {
      return *this;
    }
    if(man) {
      man->DecRef(val);
    }
    val = right.val;
    man = right.man;
    if(man) {
      man->IncRef(val);
    }
    return *this;
  }

  bool Node::operator==(Node const & other) const {
    return val == other.val;
  }

  NodeNoRef::NodeNoRef(lit val) : val(val) {}
  NodeNoRef::NodeNoRef() {}
  NodeNoRef::NodeNoRef(NodeNoRef const & right) {
    val = right.val;
  }
  NodeNoRef::NodeNoRef(Node const & right) {
    val = right.val;
  }
  NodeNoRef & NodeNoRef::operator=(NodeNoRef const & right) {
    if(this == &right) {
      return *this;
    }
    val = right.val;
    return *this;
  }

}
