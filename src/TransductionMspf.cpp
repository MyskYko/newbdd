#include "Transduction.h"

using namespace std;

bool Transduction::IsFoConeShared_rec(vector<int> & vVisits, int i, int visitor) const {
  if(vVisits[i] == visitor) {
    return false;
  }
  if(vVisits[i]) {
    return true;
  }
  vVisits[i] = visitor;
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    if(IsFoConeShared_rec(vVisits, vvFos[i][j], visitor)) {
      return true;
    }
  }
  return false;
}
bool Transduction::IsFoConeShared(int i) const {
  vector<int> vVisits(nObjs);
  for(unsigned j = 0; j < vvFos[i].size(); j++) {
    if(IsFoConeShared_rec(vVisits, vvFos[i][j], j + 1)) {
      return true;
    }
  }
  return false;
}

void Transduction::BuildFoConeCompl(int i, vector<NewBdd::Node> & vPoFsCompl) const {
  if(nVerbose > 2) {
    cout << "\t\tBuild with complemented " << i << endl;
  }
  vector<NewBdd::Node> vFsCompl = vFs;
  vFsCompl[i] = bdd->Not(vFs[i]);
  vector<bool> vMarks;
  MarkFoCone_rec(vMarks, i);
  for(list<int>::const_iterator it = vObjs.begin(); it != vObjs.end(); it++) {
    if(vMarks[*it]) {
      Build(*it, vFsCompl);
    }
  }
  for(unsigned j = 0; j < vPos.size(); j++) {
    int i0 = vvFis[vPos[j]][0] >> 1;
    int c0 = vvFis[vPos[j]][0] & 1;
    vPoFsCompl.push_back(bdd->NotCond(vFsCompl[i0], c0));
  }
}
