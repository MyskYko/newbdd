#include <iostream>

#include "AigBdd.h"

int main(int argc, char ** argv) {
  if(argc == 1) {
    std::cout << "Specify aig name" << std::endl;
    return 1;
  }
  aigman aig;
  aig.read(argv[1]);
  aig.supportfanouts();
  NewBdd::Man bdd(aig.nPis);
  std::vector<NewBdd::Node> vNodes;
  Aig2Bdd(aig, bdd, vNodes);
  NewBdd::Node::SetRef(vNodes);
  bdd.Reorder();
  int count = NewBdd::Node::CountNodes(vNodes);
  std::vector<NewBdd::var> ordering;
  bdd.GetOrdering(ordering);
  NewBdd::Man bdd2(aig.nPis);
  bdd2.SetInitialOrdering(ordering);
  std::vector<NewBdd::Node> vNodes2;
  Aig2Bdd(aig, bdd2, vNodes2);
  int count2 = NewBdd::Node::CountNodes(vNodes2);
  Bdd2Aig(bdd, aig, vNodes);
  aig.write("a.aig");
  if(count == count2) {
    std::cout << "[PASS] ";
    std::cout << count << " " << count2 << " " << argv[1] << std::endl;
  } else {
    std::cout << "[FAIL] ";
    std::cout << count << " " << count2 << " " << argv[1] << std::endl;
    return 1;
  }
  return 0;
}
