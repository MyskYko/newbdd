#include <iostream>

#include "AigBdd.h"

int main(int argc, char ** argv) {
  if(argc == 1) {
    std::cout << "Specify aig name" << std::endl;
    return 0;
  }
  aigman aig;
  aig.read(argv[1]);
  aig.supportfanouts();
  NewBdd::Man bdd(aig.nPis);
  bdd.SetParameters(1, 0);
  std::vector<NewBdd::Node> vNodes;
  Aig2Bdd(aig, bdd, vNodes);
  int count = bdd.CountNodes(vNodes);
  std::vector<NewBdd::var> ordering;
  bdd.GetOrdering(ordering);
  NewBdd::Man bdd2(aig.nPis);
  bdd2.SetInitialOrdering(ordering);
  std::vector<NewBdd::Node> vNodes2;
  Aig2Bdd(aig, bdd2, vNodes2);
  int count2 = bdd2.CountNodes(vNodes2);
  if(count == count2) {
    std::cout << "[PASS] ";
  } else {
    std::cout << "[FAIL] ";
  }
  std::cout << count << " " << count2 << " " << argv[1] << std::endl;
  Bdd2Aig(bdd, aig, vNodes);
  aig.write("a.aig");
  return 0;
}
