#ifndef NEW_BDD_NODE_H
#define NEW_BDD_NODE_H

#include "NewBddMan.h"

namespace NewBdd {

  class Node {
  public:
    inline Node(Man * man, lit val) : man(man), val(val) {
      man->IncRef(val);
    }
    inline Node() : man(NULL), val(0) {
    }
    inline Node(Node const & right) : man(right.man), val(right.val) {
      if(man) {
        man->IncRef(val);
      }
    }
    inline ~Node() {
      if(man) {
        man->DecRef(val);
      }
    }
    inline Node & operator=(Node const & right) {
      if(this == &right) {
        return *this;
      }
      if(man) {
        man->DecRef(val);
      }
      man = right.man;
      val = right.val;
      if(man) {
        man->IncRef(val);
      }
      return *this;
    }

    inline var Var() const {
      return man->Var(val);
    }
    inline bvar Id() const {
      return man->Lit2Bvar(val);
    }
    inline bool IsCompl() const {
      return man->LitIsCompl(val);
    }
    inline Node Then() const {
      return Node(man, man->Then(val));
    }
    inline Node Else() const {
      return Node(man, man->Else(val));
    }
    inline bool IsConst0() const {
      return val == 0;
    }
    inline bool IsConst1() const {
      return val == 1;
    }

    inline double OneCount() const {
      return man->OneCount(val);
    }
    inline double ZeroCount() const {
      return man->OneCount(man->LitNot(val));
    }

    inline Node operator~() const {
      return Node(man, man->LitNot(val));
    }
    inline Node operator^(bool c) const {
      return c? ~*this: *this;
    }
    inline Node operator&(Node const & other) const {
      return Node(man, man->And(val, other.val));
    }
    inline Node operator|(Node const & other) const {
      return Node(man, man->LitNot(man->And(man->LitNot(val), man->LitNot(other.val))));
    }
    inline Node operator^(Node const & other) const {
      Node z0 = Node(man, man->And(man->LitNot(val), other.val));
      Node z1 = Node(man, man->And(val, man->LitNot(other.val)));
      return z0 | z1;
    }

    inline bool operator==(Node const & other) const {
      return man == other.man && val == other.val;
    }
    inline bool operator!=(Node const & other) const {
      return !(*this == other);
    }

    static inline Node Const0(Man * man) {
      return Node(man, man->Const0());
    }
    static inline Node Const1(Man * man) {
      return Node(man, man->Const1());
    }
    static inline Node IthVar(Man * man, var v) {
      return Node(man, man->IthVar(v));
    }
    static inline void SetRef(std::vector<Node> const & vNodes) {
      if(vNodes.empty()) {
        return;
      }
      Man * man = vNodes[0].man;
      std::vector<lit> vLits(vNodes.size());
      for(size i = 0; i < vNodes.size(); i++) {
        if(man != vNodes[i].man) {
          throw std::logic_error("Nodes do not share the same manager");
        }
        vLits[i] = vNodes[i].val;
      }
      man->SetRef(vLits);
    }
    static inline bvar CountNodes(std::vector<Node> const & vNodes) {
      if(vNodes.empty()) {
        return 0;
      }
      Man * man = vNodes[0].man;
      std::vector<lit> vLits(vNodes.size());
      for(size i = 0; i < vNodes.size(); i++) {
        if(man != vNodes[i].man) {
          throw std::logic_error("Nodes do not share the same manager");
        }
        vLits[i] = vNodes[i].val;
      }
      return man->CountNodes(vLits);
    }

  private:
    Man * man;
    lit val;
  };

}

#endif
