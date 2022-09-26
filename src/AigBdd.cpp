#include <iostream>

#include "AigBdd.h"

using namespace std;

void Aig2Bdd(aigman const & aig, NewBdd::Man & bdd, vector<NewBdd::Node> & vNodes, bool fVerbose) {
  if(aig.vvFanouts.empty()) {
    throw logic_error("aig fanouts have not been computed");
  }
  vector<int> vCounts(aig.nObjs);
  for(int i = aig.nPis + 1; i < aig.nObjs; i++) {
    vCounts[i] = aig.vvFanouts[i].size();
  }
  vector<NewBdd::Node> nodes(aig.nObjs);
  nodes[0] = bdd.Const0();
  for(int i = 0; i < aig.nPis; i++) {
    nodes[i + 1] = bdd.IthVar(i);
  }
  for(int i = aig.nPis + 1; i < aig.nObjs; i++) {
    if(fVerbose) {
      cout << "Aig2Bdd: gate " << i - aig.nPis << " / " << aig.nGates << endl;
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

int Bdd2Aig_rec(NewBdd::Man const & bdd, aigman & aig, NewBdd::NodeNoRef const & x, vector<int> & values) {
  if(bdd.Id(x) == 0) {
    return bdd.IsCompl(x);
  }
  if(values[bdd.Id(x)]) {
    return values[bdd.Id(x)] ^ (int)bdd.IsCompl(x);
  }
  int v = (bdd.Var(x) + 1) << 1;
  int i0 = Bdd2Aig_rec(bdd, aig, bdd.Else(x), values) ^ (int)bdd.IsCompl(x);
  int i1 = Bdd2Aig_rec(bdd, aig, bdd.Then(x), values) ^ (int)bdd.IsCompl(x);
  aig.vObjs.push_back(v);
  aig.vObjs.push_back(i1);
  aig.nGates++;
  int j0 = aig.nObjs << 1;
  aig.nObjs++;
  aig.vObjs.push_back(v ^ 1);
  aig.vObjs.push_back(i0);
  aig.nGates++;
  int j1 = aig.nObjs << 1;
  aig.nObjs++;
  aig.vObjs.push_back(j0 ^ 1);
  aig.vObjs.push_back(j1 ^ 1);
  aig.nGates++;
  int r = (aig.nObjs << 1) ^ 1;
  aig.nObjs++;
  values[bdd.Id(x)] = r;
  return r ^ (int)bdd.IsCompl(x);
}

void Bdd2Aig(NewBdd::Man const & bdd, aigman & aig, vector<NewBdd::Node> const & vNodes) {
  aig.clear();
  aig.nPis = bdd.GetNumVars();
  aig.nObjs = aig.nPis + 1;
  aig.vObjs.resize(aig.nObjs * 2);
  vector<int> values(bdd.GetNumObjs());
  for(unsigned i = 0; i < vNodes.size(); i++) {
    int j = Bdd2Aig_rec(bdd, aig, NewBdd::NodeNoRef(vNodes[i]), values);
    aig.vPos.push_back(j);
    aig.nPos++;
  }
}
