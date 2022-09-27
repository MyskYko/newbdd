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
  std::vector<NewBdd::Node> vNodes;
  Aig2Bdd(aig, bdd, vNodes);
  std::cout << bdd.CountNodes(vNodes) << std::endl;
  return 0;
}
