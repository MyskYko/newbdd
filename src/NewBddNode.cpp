#include <stdexcept>

#include "NewBddNode.h"

using namespace std;

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

  var Node::Var() const {
    return man->Var(val);
  }
  bvar Node::Id() const {
    return man->Lit2Bvar(val);
  }
  bool Node::IsCompl() const {
    return man->LitIsCompl(val);
  }
  Node Node::Then() const {
    return Node(man, man->Then(val));
  }
  Node Node::Else() const {
    return Node(man, man->Else(val));
  }
  bool Node::IsConst0() const {
    return val == 0;
  }
  bool Node::IsConst1() const {
    return val == 1;
  }

  Node Node::operator~() const {
    return Node(man, man->LitNot(val));
  }
  Node Node::operator^(bool c) const {
    return c? ~*this: *this;
  }
  Node Node::operator&(Node const & other) const {
    return Node(man, man->And(val, other.val));
  }
  Node Node::operator|(Node const & other) const {
    return Node(man, man->LitNot(man->And(man->LitNot(val), man->LitNot(other.val))));
  }
  Node Node::operator^(Node const & other) const {
    Node z0 = Node(man, man->And(man->LitNot(val), other.val));
    Node z1 = Node(man, man->And(val, man->LitNot(other.val)));
    return z0 | z1;
  }

  bool Node::operator==(Node const & other) const {
    if(man != other.man) {
      return false;
    }
    return !man || val == other.val;
  }
  bool Node::operator!=(Node const & other) const {
    return !(*this == other);
  }

  bool Node::Valid() const {
    return man;
  }

  Node Const0(Man * man) {
    return Node(man, man->Const0());
  }
  Node Const1(Man * man) {
    return Node(man, man->Const1());
  }
  Node IthVar(Man * man, var v) {
    return Node(man, man->IthVar(v));
  }

  void SetRef(vector<Node> const & vNodes) {
    if(vNodes.empty()) {
      return;
    }
    Man * man = vNodes[0].man;
    vector<lit> vLits(vNodes.size());
    for(size i = 0; i < vNodes.size(); i++) {
      if(man != vNodes[i].man) {
        throw logic_error("nodes do not share the same manager");
      }
      vLits[i] = vNodes[i].val;
    }
    man->SetRef(vLits);
  }

  bvar CountNodes(vector<Node> const & vNodes) {
    if(vNodes.empty()) {
      return 0;
    }
    Man * man = vNodes[0].man;
    vector<lit> vLits(vNodes.size());
    for(size i = 0; i < vNodes.size(); i++) {
      if(man != vNodes[i].man) {
        throw logic_error("nodes do not share the same manager");
      }
      vLits[i] = vNodes[i].val;
    }
    return man->CountNodes(vLits);
  }

#ifdef COUNT_ONES
  double Node::OneCount() const {
    return man->OneCount(val);
  }
  double Node::ZeroCount() const {
    return man->OneCount(man->LitNot(val));
  }
#endif

}
