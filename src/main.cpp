#include <aig.hpp>
#include "NewBdd.h"

void Aig2Bdd(aigman const & aig, NewBdd::Man & bdd, std::vector<NewBdd::Node> & vNodes, bool fVerbose = 0) {
  std::vector<int> vCounts(aig.nObjs);
  for(int i = aig.nPis + 1; i < aig.nObjs; i++) {
    vCounts[i] = aig.vvFanouts[i].size();
  }
  std::vector<NewBdd::Node> nodes(aig.nObjs);
  nodes[0] = bdd.Const0();
  for(int i = 0; i < aig.nPis; i++) {
    nodes[i + 1] = bdd.IthVar(i);
  }
  for(int i = aig.nPis + 1; i < aig.nObjs; i++) {
    if(fVerbose) {
      std::cout << "Aig2Bdd: gate " << i - aig.nPis << " / " << aig.nGates << std::endl;
    }
    int i0 = aig.vObjs[i + i] >> 1;
    int i1 = aig.vObjs[i + i + 1] >> 1;
    bool c0 = aig.vObjs[i + i] & 1;
    bool c1 = aig.vObjs[i + i + 1] & 1;
    nodes[i] = bdd.And(bdd.NotCond(nodes[i0], c0), bdd.NotCond(nodes[i1], c1));
    vCounts[i0]--;
    if(!vCounts[i0]) {
      nodes[i0] = bdd.Const0();
    }
    vCounts[i1]--;
    if(!vCounts[i1]) {
      nodes[i1] = bdd.Const0();
    }
  }
  for(int i = 0; i < aig.nPos; i++) {
    int i0 = aig.vPos[i] >> 1;
    bool c0 = aig.vPos[i] & 1;
    vNodes.emplace_back(bdd.NotCond(nodes[i0], c0));
  }
}

int main(int argc, char ** argv) {
  aigman aig;
  aig.read(argv[1]);
  aig.supportfanouts();
  NewBdd::Man bdd(aig.nPis, 1);
  std::vector<NewBdd::Node> vNodes;
  Aig2Bdd(aig, bdd, vNodes, 0);
  std::cout << bdd.CountNodes(vNodes) << std::endl;
  return 0;
}
