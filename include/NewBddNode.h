#ifndef NEW_BDD_NODE_H
#define NEW_BDD_NODE_H

#include "NewBddMan.h"

namespace NewBdd {

  class Node {
  public:
    static Node Const0(Man * man);
    static Node Const1(Man * man);
    static Node IthVar(Man * man, var v);
    static void SetRef(std::vector<Node> const & vNodes);
    static bvar CountNodes(std::vector<Node> const & vNodes);

    Node(Man * man, lit val);
    Node();
    Node(Node const & right);
    ~Node();
    Node & operator=(Node const & right);

    var Var() const;
    bvar Id() const;
    bool IsCompl() const;
    Node Then() const;
    Node Else() const;
    bool IsConst0() const;
    bool IsConst1() const;

#ifdef COUNT_ONES
    double OneCount() const;
    double ZeroCount() const;
#endif

    Node operator~() const;
    Node operator^(bool c) const;
    Node operator&(Node const & other) const;
    Node operator|(Node const & other) const;
    Node operator^(Node const & other) const;

    bool operator==(Node const & other) const;
    bool operator!=(Node const & other) const;

    bool Valid() const;

  private:
    Man * man;
    lit val;
  };

}

#endif
