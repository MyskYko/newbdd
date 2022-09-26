#include <iostream>

#include "AigBdd.h"

int main(int argc, char ** argv) {
  aigman aig;
  aig.read(argv[1]);
  aig.supportfanouts();
  NewBdd::Man bdd(aig.nPis);
  std::vector<NewBdd::Node> vNodes;
  Aig2Bdd(aig, bdd, vNodes);
  std::cout << bdd.CountNodes(vNodes) << std::endl;
  return 0;
}
