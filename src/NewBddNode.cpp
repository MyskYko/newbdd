#include "NewBddNode.h"

namespace NewBdd {

  Node::Node(Man * man, lit val) : man(man), val(val) {
    man->IncRef(val);
  }
  Node::Node() {
    man = NULL;
  }
  Node::Node(const Node & right) {
    man = right.man;
    val = right.val;
    man->IncRef(val);
  }
  Node::~Node() {
    if(man) {
      man->DecRef(val);
    }
  }

  Node & Node::operator=(const Node & right) {
    if(this == &right) {
      return *this;
    }
    if(man) {
      man->DecRef(val);
    }
    val = right.val;
    man = right.man;
    man->IncRef(val);
    return *this;
  }
  bool Node::operator==(const Node & other) const {
    return val == other.val;
  }
  bool Node::operator!=(const Node & other) const {
    return val != other.val;
  }

  NodeNoRef::NodeNoRef(lit val) : val(val) {}
  NodeNoRef::NodeNoRef() {}
  NodeNoRef::NodeNoRef(const NodeNoRef & right) {
    val = right.val;
  }
  NodeNoRef::NodeNoRef(const Node & right) {
    val = right.val;
  }

  NodeNoRef & NodeNoRef::operator=(const NodeNoRef & right) {
    if(this == &right) {
      return *this;
    }
    val = right.val;
    return *this;
  }
  bool NodeNoRef::operator==(const NodeNoRef & other) const {
    return val == other.val;
  }
  bool NodeNoRef::operator!=(const NodeNoRef & other) const {
    return val != other.val;
  }

}
